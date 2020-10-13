#include "ImgReadWrite.h"

int main()
{
	Image Img("new.bmp");
	vector<vector<uint8_t>> inputImageData = Img.imgTable;
	cout << Img.imgTable.size() << endl;
	InfoHeader inputInfo = Img.getInfoHeader();
	InfoHeader outputInfo = Img.getInfoHeader();
	int height = inputInfo.height;
	int width = inputInfo.width;
	int factor = inputImageData.size()/(inputInfo.height * inputInfo.width);
	cout << height << endl << width << endl << factor << endl;
	vector<vector<uint8_t>> outputImageData(inputInfo.height*factor, vector<uint8_t>(inputInfo.width*factor, 0));
	outputImageData = Img.rotate(inputImageData, outputImageData, height, width, inputInfo, outputInfo);
	cout << "................................................................." << endl;
	cout << outputInfo.height << endl << outputInfo.width << endl ;
	Img.setInfoHeader(outputInfo);
	Img.writeMatrix("rot.bmp");
	Img.imgTable = outputImageData;
	/*imgDataOutput = Img.brightnessUp(inputImageData, imgDataOutput, inputImageData.size(), 100);
	Image outputImg(Img, imgDataOutput);
	outputImg.write("bright24.bmp");*/
	return 0;
}