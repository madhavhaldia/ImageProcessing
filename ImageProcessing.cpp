#include<ImgReadWrite.h>

int main()
{
	Image Img("test.bmp");
	Img.write("test_copy.bmp");
	return 0;
}