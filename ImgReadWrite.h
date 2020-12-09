<<<<<< < Updated upstream
#pragma once
#include<fstream>
#include<vector>
#include<iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
	std::mutex mtx_sepia;
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
	uint32_t global_row_blur_threads{ 0 };
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
	Image(Image& Img, vector<uint8_t> imageData) // Use this while printing image
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

	 void blur() {												//filterblur
		uint32_t num_threads = infoHeader.height;
		

		time_t start, end;
		time(&start);
		
			some_threads.push_back(std::thread(&Image::transform_to_blur, this));			//call blur function
			
		for (auto& t : some_threads) {
			t.join();
		}
		some_threads.clear();
		time(&end);
		std::cout << difftime(end, start) << " seconds (rgb to sepia - HEIGHT: " << infoHeader.height << " threads, 1 per row, WIDTH: " << infoHeader.width << ")" << std::endl;

	}

	void transform_to_blur() {

		double blur_filter[3][3] =
		{
		   0.11, 0.11, 0.11,
		   0.11, 0.11, 0.11,
		   0.11, 0.11, 0.11
		};

		double blur_factor = 1.0;
		double blur_bias = 0.0;


		uint32_t channels = infoHeader.bitcount / 8;
		uint32_t A{ 0 }, R{ 0 }, G{ 0 }, B{ 0 }, grey{ 0 };
		uint32_t tr{ 0 }, tg{ 0 }, tb{ 0 };
		uint32_t w = infoHeader.width, h = infoHeader.height;
		mtx_sepia.lock();

		global_row_blur_threads = 0;

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{

				R = imgData[channels * (global_row_blur_threads * infoHeader.width + x) + 0];
				G = imgData[channels * (global_row_blur_threads * infoHeader.width + x) + 1];
				B = imgData[channels * (global_row_blur_threads * infoHeader.width + x) + 2];

				for (int filterY = 0; filterY < 3; filterY++)
					for (int filterX = 0; filterX < 3; filterX++)
					{
						int imageX = (x - 3 / 2 + filterX + w) % w;
						int imageY = (y - 3 / 2 + filterY + h) % h;


						R += imgData[imageY * w + imageX] * blur_filter[filterY][filterX];
						G += imgData[imageY * w + imageX] * blur_filter[filterY][filterX];
						B += imgData[imageY * w + imageX] * blur_filter[filterY][filterX];
					}

				imgData[channels * (global_row_blur_threads * infoHeader.width + x) + 0] = R;
				imgData[channels * (global_row_blur_threads * infoHeader.width + x) + 1] = G;
				imgData[channels * (global_row_blur_threads * infoHeader.width + x) + 2] = B;
			}
			global_row_blur_threads += 1;
		}
		
		mtx_sepia.unlock();

	}





	void sharpen() {											//filter sharpening
		uint32_t num_threads = infoHeader.height;


		time_t start, end;
		time(&start);

		some_threads.push_back(std::thread(&Image::transform_to_sharpen, this));

		for (auto& t : some_threads) {
			t.join();
		}
		some_threads.clear();
		time(&end);
		std::cout << difftime(end, start) << " seconds (rgb to sepia - HEIGHT: " << infoHeader.height << " threads, 1 per row, WIDTH: " << infoHeader.width << ")" << std::endl;

	}

	void transform_to_sharpen() {

		double sharpening_filter[3][3] =
		{
<<<<<<< Updated upstream
		   0  , -.5 ,    0 ,
		  -.5 ,   3  , -.5,
		  0  , -.5 ,    0
		};

=======
		   0.11, 0.01, 0.11,
		   0.11, 0.11, 0.11,
		   0.11, 0.11, 0.11
		};

		
>>>>>>> Stashed changes


		uint32_t A{ 0 }, R{ 0 }, G{ 0 }, B{ 0 }, grey{ 0 };
<<<<<<< Updated upstream

=======
		
>>>>>>> Stashed changes
		uint32_t w = infoHeader.width, h = infoHeader.height;

		mtx_sepia.lock();



		for (int x = 0; x < w; x++)
		{
			R = imgData[(infoHeader.width + x) + 0];
			G = imgData[(infoHeader.width + x) + 1];
			B = imgData[(infoHeader.width + x) + 2];

			for (int y = 0; y < h; y++)
			{
				for (int filterY = 0; filterY < 3; filterY++)
					for (int filterX = 0; filterX < 3; filterX++)
					{
						int imageX = (x - 3 / 2 + filterX + w) % w;
						int imageY = (y - 3 / 2 + filterY + h) % h;


						R += imgData[imageY * w + imageX] * sharpening_filter[filterY][filterX];
						G += imgData[imageY * w + imageX] * sharpening_filter[filterY][filterX];
						B += imgData[imageY * w + imageX] * sharpening_filter[filterY][filterX];
					}

				imgData[(infoHeader.width + x) + 0] = R;
				imgData[(infoHeader.width + x) + 1] = G;
				imgData[(infoHeader.width + x) + 2] = B;
			}

		}

		mtx_sepia.unlock();

	}
	


	void sharpen() {
		uint32_t num_threads = infoHeader.height;


		time_t start, end;
		time(&start);

		some_threads.push_back(std::thread(&Image::transform_to_sharpen, this));

		for (auto& t : some_threads) {
			t.join();
		}
		some_threads.clear();
		time(&end);
		std::cout << difftime(end, start) << " seconds (rgb to sepia - HEIGHT: " << infoHeader.height << " threads, 1 per row, WIDTH: " << infoHeader.width << ")" << std::endl;

	}

	void transform_to_sharpen() {

		double sharpening_filter[3][3] =
		{
		   0  , -.5 ,    0 ,
		  -.5 ,   3  , -.5,
		  0  , -.5 ,    0
		};

		


		uint32_t channels = infoHeader.bitcount / 8;
		uint32_t A{ 0 }, R{ 0 }, G{ 0 }, B{ 0 }, grey{ 0 };
		
		uint32_t w = infoHeader.width, h = infoHeader.height;

		mtx_sepia.lock();

		

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{

				R = imgData[infoHeader.width + x) + 0];
				G = imgData[infoHeader.width + x) + 1];
				B = imgData[infoHeader.width + x) + 2];

				for (int filterY = 0; filterY < 3; filterY++)
					for (int filterX = 0; filterX < 3; filterX++)
					{
						int imageX = (x - 3 / 2 + filterX + w) % w;
						int imageY = (y - 3 / 2 + filterY + h) % h;


						R += imgData[imageY * w + imageX] * sharpening_filter[filterY][filterX];
						G += imgData[imageY * w + imageX] * sharpening_filter[filterY][filterX];
						B += imgData[imageY * w + imageX] * sharpening_filter[filterY][filterX];
					}

				imgData[(infoHeader.width + x) + 0] = R;
				imgData[(infoHeader.width + x) + 1] = G;
				imgData[(infoHeader.width + x) + 2] = B;
			}
			
		}

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


