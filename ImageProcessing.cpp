#include "ImgReadWrite.h"

int main()
{
	Image Img("new.bmp");
	vector<uint8_t> inputImageData = Img.getImgData();
	vector<uint8_t> imgDataOutput = Img.getImgData();
	InfoHeader InputHeader = Img.getInfoHeader();
	int size = InputHeader.height * InputHeader.width;
	std::cout << size << endl;
	std::cout << inputImageData.size();
	/*imgDataOutput = Img.brightnessUp(inputImageData, imgDataOutput, inputImageData.size(), 100);
	Image outputImg(Img, imgDataOutput);
	outputImg.write("bright24.bmp");*/
	return 0;
}