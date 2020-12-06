#pragma once
#include<fstream>
#include<vector>
#include<iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
std::mutex mtx_sepia;
std::mutex mtx_grey;

using namespace std;
/// Using Pragma pack to remove padding
/// The following is the standard header for BMP
/// Attributes have been initialised to default values
/// Dividing the header into 3 structs (file, info and color)

#pragma pack(push,1)

struct FileHeader
{
	uint16_t type{ 0x4D42 }; //file extension converted to hex
	uint32_t size{ 0 };	//size of file
	uint16_t res1{ 0 };	//reserved
	uint16_t res2{ 0 };	//reserved
	uint32_t begin{ 0 };	//starting byte of pixel data
};

struct InfoHeader
{
	uint32_t size{ 0 };
	int32_t width{ 0 };
	int32_t height{ 0 };

	//The following fields are constants for the scope of this program beacuse we are working with uncompressed images
	uint16_t planeNum{ 1 };
	uint16_t bitcount{ 0 };
	uint32_t compressID{ 0 }; //can be changed to 3 in case of compression
	uint32_t imgSize{ 0 }; //always 0 for uncompressed image
	int32_t xppm{ 0 };
	int32_t yppm{ 0 };
	uint32_t totalColor{ 0 }; //0 is max
	uint32_t impColorCount{ 0 }; // 0 is all colors
};

struct ColorHeader //Contains bit masks for RGB and Alpha channels . When working with different color spaces (greyscale etc.) this struct will need to be redone . It is currently initialised to 32bit images
{
	uint32_t red{ 0x00ff0000 };
	uint32_t green{ 0x0000ff00 };
	uint32_t blue{ 0x000000ff };
	uint32_t alpha{ 0xff000000 };
	uint32_t colorSpace{ 0x73524742 };//sRGB
	uint32_t buffer[16]{ 0 }; //sRGB buffer
};
#pragma pack(pop)

class Image {

	FileHeader fileHeader;
	InfoHeader infoHeader;
	ColorHeader colorHeader;
	int rowPadding{ 0 };
	vector<uint8_t> imgData;
	uint32_t global_row_sepia_threads{ 0};
	uint32_t global_row_greyscale_threads{ 0};

    std::vector<std::thread> some_threads;
public:
	vector<vector<uint8_t>> imgTable;
private:

	int matchPadding(int rowPadding)
	{
		int newRow = this->rowPadding;
		while (newRow % rowPadding != 0)
		{
			newRow++;
		}
		return newRow;
	}

	void checkHeader(ColorHeader& colorHeader)
	{
		ColorHeader baseColorHeader;
		if (baseColorHeader.red != colorHeader.red || baseColorHeader.green != colorHeader.green || baseColorHeader.blue != colorHeader.blue || baseColorHeader.alpha != colorHeader.alpha)
		{
			cout << " Unmatched Header information ";
		}
		if (baseColorHeader.colorSpace != colorHeader.colorSpace)
		{
			cout << " Color Space not identified ";
		}
	}

	void headerWrite(ofstream& headerOut)
	{
		headerOut.write((const char*)&fileHeader, sizeof(fileHeader));
		headerOut.write((const char*)&infoHeader, sizeof(infoHeader));
		if (infoHeader.bitcount == 32)
		{
			headerOut.write((const char*)&colorHeader, sizeof(colorHeader));
		}
	}
	void dataWriter(ofstream& dataOut)
	{
		headerWrite(dataOut);
		dataOut.write((const char*)imgData.data(), imgData.size());
	}

public:
	Image(Image &Img, vector<uint8_t> imageData) // Use this while printing image
	{
		setFileHeader(Img.getFileHeader());
		setInfoHeader(Img.getInfoHeader());
		setColorHeader(Img.getColorHeader());
		imgData = imageData;
		rowPadding = Img.rowPadding;
	}
	Image(const char* ImgName)
	{
		read(ImgName);
	}
	Image(int width, int height, bool bAlpha = true)
	{
		if (width <= 0 || height <= 0)
		{
			cout << "Improper dimensions";
		}
		infoHeader.width = width;
		infoHeader.height = height;
		if (bAlpha)
		{
			infoHeader.size = sizeof(InfoHeader) + sizeof(ColorHeader);
			fileHeader.begin = sizeof(FileHeader) + sizeof(InfoHeader) + sizeof(ColorHeader);

			infoHeader.bitcount = 32;
			infoHeader.compressID = 3;
			rowPadding = width * 4;
			imgData.resize(rowPadding * height);
			fileHeader.size = fileHeader.begin + imgData.size();
		}
		else
		{
			infoHeader.size = sizeof(InfoHeader);
			fileHeader.begin = sizeof(FileHeader) + sizeof(InfoHeader);

			infoHeader.bitcount = 24;
			infoHeader.compressID = 0;
			rowPadding = width * 3;
			imgData.resize(rowPadding * height);

			int newRow = matchPadding(4);
			fileHeader.size = fileHeader.begin + imgData.size() + infoHeader.height * (newRow - rowPadding);
		}
	}
	void read(const char* imgname)
	{
		ifstream imgIn{ imgname, ios::binary };
		if (imgIn)
		{
			imgIn.read((char*)&fileHeader, sizeof(fileHeader));
			if (fileHeader.type != 0x4D42)
			{
				cout << "Unrecognised Throw Error here";
			}

			imgIn.read((char*)&infoHeader, sizeof(infoHeader));

			if (infoHeader.bitcount == 32)
			{
				if (infoHeader.size >= (sizeof(InfoHeader) + sizeof(ColorHeader)))
				{
					imgIn.read((char*)&colorHeader, sizeof(colorHeader));
					checkHeader(colorHeader);
				}
				else
				{
					cout << "Bit mask information not found";
				}
			}
			imgIn.seekg(fileHeader.begin, imgIn.beg);

			if (infoHeader.bitcount == 32)
			{
				infoHeader.size = sizeof(InfoHeader) + sizeof(ColorHeader);
				fileHeader.begin = sizeof(FileHeader) + sizeof(InfoHeader) + sizeof(ColorHeader);
			}
			else
			{
				infoHeader.size = sizeof(InfoHeader);
				fileHeader.begin = sizeof(InfoHeader) + sizeof(FileHeader);
			}

			fileHeader.size = fileHeader.begin;
			if (infoHeader.height < 0) // this is a check if the image centre is bottom left or top left.
			{
				cout << "Throw Error Origin not compatible";
			}

			imgData.resize(infoHeader.width * infoHeader.height * infoHeader.bitcount / 8); // Resize for different image dimensions

			if (infoHeader.width % 4 == 0)
			{
				imgIn.read((char*)imgData.data(), imgData.size());
				imgIn.read((char*)imgTable.data(), imgTable.size());
				fileHeader.size += static_cast<uint32_t>(imgData.size());
			}
			else
			{
				rowPadding = infoHeader.width * infoHeader.bitcount / 8;
				uint32_t newRow = matchPadding(4);
				vector<uint8_t> paddingRow(newRow - rowPadding);
				for (auto i = 0; i < infoHeader.height; i++)
				{
					imgIn.read((char*)(imgData.data() + rowPadding * i), rowPadding);
					imgIn.read((char*)paddingRow.data(), paddingRow.size());
				}
				fileHeader.size += static_cast<uint32_t>(imgData.size()) + infoHeader.height * static_cast<uint32_t>(paddingRow.size());
			}

		}
		else
		{
			//Unable to Open file
		}
	}

	void write(const char* imgName)
	{
		ofstream imgOut{ imgName, ios::binary };
		if (imgOut)
		{
			if (infoHeader.bitcount == 32)
			{
				dataWriter(imgOut);
			}
			else if (infoHeader.bitcount == 24)
			{
				if (infoHeader.width % 4 == 0)
				{
					dataWriter(imgOut);
				}
				else
				{
					int newRow = matchPadding(4);
					vector<int> paddingRow(newRow - rowPadding);
					headerWrite(imgOut);
					for (auto i = 0; i < infoHeader.height; ++i)
					{
						imgOut.write((const char*)(imgData.data() + this->rowPadding * i), this->rowPadding);
						imgOut.write((const char*)paddingRow.data(), paddingRow.size());
					}
				}
			}
			else
			{
				cout << "Exception .. improper bits per pixel";
			}
		}
		else {
			// Cant open output image
		}

	}
	void grayscale() {
        uint32_t num_threads = infoHeader.height;
        global_row_greyscale_threads = 0;

        time_t start, end;
        time(&start);
        for (uint32_t i = 0; i < num_threads; i++) {
            some_threads.push_back(std::thread(&Image::transform_row_to_greyscale, this));
        }
        for (auto& t : some_threads) {
            t.join();
        }
        some_threads.clear();
        time(&end);
        std::cout << difftime(end, start) << " seconds (rgb to grescale - HEIGHT: " << infoHeader.height << " threads, 1 per row, WIDTH: " << infoHeader.width << ")" << std::endl;
    }


	void sepia() {




        uint32_t num_threads = infoHeader.height;
        global_row_sepia_threads = 0;

        time_t start, end;
        time(&start);
        for (uint32_t i = 0; i < num_threads; i++) {
            some_threads.push_back(std::thread(&Image::transform_row_to_sepia, this));
            }
        for (auto& t : some_threads) {
            t.join();
        }
        some_threads.clear();
        time(&end);
        std::cout << difftime(end, start) << " seconds (rgb to sepia - HEIGHT: " << infoHeader.height << " threads, 1 per row, WIDTH: " << infoHeader.width << ")" << std::endl;

    }
	void transform_row_to_greyscale() {
        uint32_t channels = infoHeader.bitcount / 8;
        uint32_t A{0}, R{0}, G{0}, B{0}, grey{0};
        mtx_grey.lock();
        for (uint32_t column = 0; column < infoHeader.width; column++) {
            R = imgData[channels * (global_row_greyscale_threads * infoHeader.width + column) + 0];
            G = imgData[channels * (global_row_greyscale_threads * infoHeader.width + column) + 1];
            B = imgData[channels * (global_row_greyscale_threads * infoHeader.width + column) + 2];

            grey = ((0.3 * R) + (0.59 * G) + (0.11 * B));

            imgData[channels * (global_row_greyscale_threads * infoHeader.width + column) + 0] = grey;
            imgData[channels * (global_row_greyscale_threads * infoHeader.width + column) + 1] = grey;
            imgData[channels * (global_row_greyscale_threads * infoHeader.width + column) + 2] = grey;
        }
        global_row_greyscale_threads += 1;
        mtx_grey.unlock();
    }

    transform_row_to_sepia() {

        uint32_t channels = infoHeader.bitcount / 8;
        uint32_t A{0}, R{0}, G{0}, B{0}, grey{0};
        uint32_t tr{0}, tg{0}, tb{0};
        mtx_sepia.lock();
        for (uint32_t column = 0; column < infoHeader.width; column++) {
            R = imgData[channels * (global_row_sepia_threads * infoHeader.width + column) + 0];
            G = imgData[channels * (global_row_sepia_threads * infoHeader.width + column) + 1];
            B = imgData[channels * (global_row_sepia_threads * infoHeader.width + column) + 2];

            tr = 0.393 * R + 0.769 * G + 0.189 * B;
            tg = 0.349 * R + 0.686 * G + 0.168 * B;
            tb = 0.272 * R + 0.534 * G + 0.131 * B;

            if (tr > 255) R = 255;
            else R = tr;
            if (tg > 255) G = 255;
            else G = tg;
            if (tb > 255) B = 255;
            else B = tb;

            imgData[channels * (global_row_sepia_threads * infoHeader.width + column) + 0] = R;
            imgData[channels * (global_row_sepia_threads * infoHeader.width + column) + 1] = G;
            imgData[channels * (global_row_sepia_threads * infoHeader.width + column) + 2] = B;
        }
        global_row_sepia_threads += 1;
        mtx_sepia.unlock();

    }

	vector<uint8_t> brightnessUp(vector<uint8_t> _inputImgData, vector<uint8_t> _outputImgData, int imgSize, int brightness) {
		for (int i = 0; i < imgSize; i++) {

			int temp = _inputImgData[i] + brightness;
			_outputImgData[i] = (temp > 255) ? 255 : temp;


		}
		return _outputImgData;
	}
	vector<uint8_t> brightnessDown(vector<uint8_t> _inputImgData, vector<uint8_t> _outputImgData, int imgSize, int brightness) {
		for (int i = 0; i < imgSize; i++) {

			int temp = _inputImgData[i] - brightness;
			_outputImgData[i] = (temp < 0) ? 0 : temp;


		}
		return _outputImgData;
	}

	FileHeader getFileHeader()
	{
		return fileHeader;
	}
	InfoHeader getInfoHeader()
	{
		return infoHeader;
	}
	ColorHeader getColorHeader()
	{
		return colorHeader;
	}
	vector<uint8_t> getImgData()
	{
		return imgData;
	}
	void setFileHeader(FileHeader data)
	{
		fileHeader = data;
	}
	void setInfoHeader(InfoHeader data)
	{
		infoHeader = data;
	}
	void setColorHeader(ColorHeader data)
	{
		colorHeader = data;
	}
};
