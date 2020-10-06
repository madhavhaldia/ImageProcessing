#include "ImgReadWrite.h"

int main()
{
	Image Img("test.bmp");
	vector<uint8_t> inputImageData = Img.getImgData();
	vector<uint8_t> imgDataOutput = Img.getImgData();
	InfoHeader InputHeader = Img.getInfoHeader();
	int size = InputHeader.height * InputHeader.width;
	Img.brightnessUp(inputImageData, imgDataOutput, inputImageData.size(), 70);
	Image outputImg(Img, Img.getImgData());
	outputImg.write("bright2.bmp");
	return 0;
}