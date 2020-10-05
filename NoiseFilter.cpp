#include<fstream>
#include<vector>
#include<iostream>
#include<ImgReadWrite.h>
using namespace std;



class filter : public Image
{
public:
	Image img1();

	void _medianfilter()
	{
		
		string imgIn = imgIn;
		//   Move window through all elements of the imgIn
		for (int m = 1; m < infoHeader.height - 1; ++m)
			for (int n = 1; n < infoHeader.width - 1; ++n)
			{
				//   Pick up window elements
				int k = 0;
				long int  window[5];
				for (int j = m - 1; j < m + 2; ++j)
					for (int i = n - 1; i < n + 2; ++i)
						window[k++] = imgIn[j * infoHeader.width + i];

				//   Order elements (only half of them)
				for (int j = 0; j < 5; ++j)
				{
					//   Find position of minimum element
					int min = j;
					for (int l = j + 1; l < 9; ++l)
						if (window[l] < window[min])
							min = l;
					//   Put found minimum element in its place
					const long int temp = window[j];
					window[j] = window[min];
					window[min] = temp;
				}
				//   Get imgData - the middle element
				imgData[(m - 1) * (infoHeader.width - 2) + n - 1] = window[4];
			}

		//write(imgData);
	}
};