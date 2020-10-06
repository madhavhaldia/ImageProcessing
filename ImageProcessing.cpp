#include "ImgReadWrite.h"

int main()
{
	Image Img("test.bmp");
	vector<uint8_t> inputImageData = Img.getImgData();
	vector<uint8_t> imgDataOutput;
	InfoHeader InputHeader = Img.getInfoHeader();
	Img.brightnessUp(inputImageData, imgDataOutput, InputHeader.size, 70);
	Image outputImg(Img, imgDataOutput);
	outputImg.write("bright.bmp");
	return 0;
}