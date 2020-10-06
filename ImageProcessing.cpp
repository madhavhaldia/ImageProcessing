#include "ImgReadWrite.h"

int main()
{
	Image Img("cameraman.bmp");
	vector<uint8_t> inputImageData = Img.getImgData();
	vector<uint8_t> imgDataOutput = Img.getImgData();
	InfoHeader InputHeader = Img.getInfoHeader();
	int size = InputHeader.height * InputHeader.width;
	Img.brightnessUp(inputImageData, imgDataOutput, inputImageData.size(), 70);
	Image outputImg(Img, imgDataOutput);
	outputImg.write("bright2.bmp");
	return 0;
}