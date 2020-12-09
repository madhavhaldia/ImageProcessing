#include<iostream>
#include<stdio.h>
#include<process.h>
#include<stdlib.h>
#include "ImgReadWrite.h"

using namespace std;
int main(){

    Image Img("qwert.bmp");
	vector<uint8_t> inputImageData = Img.getImgData();
	vector<uint8_t> imgDataOutput = Img.getImgData();
	InfoHeader InputHeader = Img.getInfoHeader();
	int size = InputHeader.height * InputHeader.width;

	Img.sepia();

	Img.write("lena2.bmp");

	Img.grayscale();

	Img.write("lena1.bmp");


	return 0;
}
