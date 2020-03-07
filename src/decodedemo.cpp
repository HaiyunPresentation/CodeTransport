#include "decode.h"
#include "CodeDetect.h"

#define BLOCK_ROWS 32
#define BLOCK_BITS 1024
#define OUTPUT_FPS 5

using namespace std;
using namespace cv;


Detector detector;


int Decode(Mat img, bool& end, int order)
{
	FILE* fp = fopen("out.bin", "a+");
	Mat resizeImg;
	if (!detector.GetCropCode(img, resizeImg))
	{
		cout << "Can't detect " << order << endl;
		imwrite("v0302" + to_string(order) + ".jpg", img);
		return -1;
	}
	cout << "Detect " << order << endl;
	//imshow("",resizeImg);
	//waitKey(0);
	imwrite("v0302OK" + to_string(order) + ".jpg", resizeImg);
	Mat circleImg;
	cvtColor(resizeImg, circleImg, COLOR_GRAY2BGR);
	int total_bit = 0;
	while (total_bit < 1020)
	{
		unsigned char chr = '\0';
		for (int k = 0; k < 8; k++)
		{
			while ((total_bit / 32 < 4 || total_bit / 32 >= 28) && (total_bit % 32 < 4 || total_bit % 32 >= 28))
			{
				total_bit++;
			}

			int val = //(*(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 5) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
				//+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 5) + resizeImg.step[1] * (16 * (total_bit % 16) + 9))
				+*(resizeImg.data + resizeImg.step[0] * (8 * (total_bit / BLOCK_ROWS) + 4) + resizeImg.step[1] * (8 * (total_bit % BLOCK_ROWS) + 4))
				//+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
				//+* (resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 9)))
				//5;
				;
			circle(circleImg, Point(8 * (total_bit % BLOCK_ROWS) + 4, 8 * (total_bit / BLOCK_ROWS) + 4), 2, Scalar(255, 0, 0), -1);
			
			//int val = *(code.data + code.step[0] * (total_bit / 16) + code.step[1] * (total_bit % 16));
			chr = chr << 1;
			if (val < 128 && (!(total_bit % 2) && !((total_bit / 32) % 2) || (total_bit % 2) && ((total_bit / 32) % 2))
				|| val >= 128 && !(!(total_bit % 2) && !((total_bit / 32) % 2) || (total_bit % 2) && ((total_bit / 32) % 2)))
			{

				chr = chr | 1;

			}

			total_bit++;
		}

		if (chr == 255)
		{
			end = true;
			break;
		}
		fprintf(fp, "%c", chr);

	}
	imshow("circleImg", circleImg);
	waitKey(0);
	cout << total_bit << endl;

	fclose(fp);

	return 0;
}

int NaiveCodeVideoCapture()
{
	VideoCapture vc = VideoCapture("out.mp4");
	if (!vc.isOpened())
	{
		cout << "Can't find the video!" << endl;
		return -1;
	}

	Mat srcImg;
	bool end = false;
	int order = 0;
	do
	{
		vc.read(srcImg);
		if (!srcImg.data) break;
	} while (!detector.IsCode(srcImg));
	while (true)
	{

		vc.read(srcImg);
		if (!srcImg.data) break;

		//imwrite("op" + to_string(order) + ".jpg",srcImg);

		
		Decode(srcImg, end, order);
		order++;
		if (end) break;

		for (int i = 1; i < 30/OUTPUT_FPS; i++)
		{
			vc.read(srcImg);
			if (!srcImg.data) break;
		}
	}
	return 0;
}


int main()
{

	NaiveCodeVideoCapture();
	//Mat img = imread("op12D.jpg");
	//Mat img = imread("op22.jpg");
	//if (img.empty())
	//{
	//	cout << "Null";
	//	return -1;
	//}
	/*Mat dst;
	//cout << QRCodeDetect::GetCropCode(img, dst);
	//cout << CodeDetect::GetCropCode(img,dst);
	Mat img;
	int ord = 1;
	for (;ord<=4; ord++)
	{
		//img= imread(".\\test"+to_string(ord)+".jpg");
		img = imread(".\\T" + to_string(ord) +".jpg");
		if (img.empty())
		{
			cout << "Null";
			//break;
		}
		else
		{
			bool end = false;
			cout << ord << " : " << CodeDetect::GetCropCode(img,dst)<<" ";

		//Decode(img, end,ord);

		}
		cout << endl;

	}
	//*/
	cout << endl << "Done" << endl;

	return 0;
}