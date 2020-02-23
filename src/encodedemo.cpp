#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <windows.h>
#define PIXEL_ROWS 512
#define BLOCK_ROWS 16
#define BLOCK_BITS 256
#define OUTPUT_ROWS 800
#define OUTPUT_COLS 800
#define BENCHMARK_SIZE 64
using namespace std;
using namespace cv;


//把灰度图任一像素改成指定灰度
//黑为0，白为255
void DrawColor(Mat& img, int i, int j, int value)
{
	if (value < 0 || value>255)
	{
		cout << "Pixel value error" << endl;
		return;
	}
	*(img.data + img.step[0] * i + img.step[1] * j) = value;
}


void DrawBlock(Mat& img, int i, int j, int value)
{
	if (value < 0 || value>255)
	{
		cout << "Block pixel value error" << endl;
		return;
	}
	for (int row = PIXEL_ROWS / BLOCK_ROWS * i; row < PIXEL_ROWS / BLOCK_ROWS * (i + 1); row++)
	{
		for (int col = PIXEL_ROWS / BLOCK_ROWS * j; col < PIXEL_ROWS / BLOCK_ROWS * (j + 1); col++)
		{
			DrawColor(img, row, col, value);
		}
	}
}

//打印一个定位点
void PrintBenchmark(Mat& img, int row, int col,int size)
{
	for (int i = 0; i < size / 8*7; i++)
	{
		if (i < size/8 || (i >= size / 8*6 && i < size / 8*7))
		{
			for (int j = 0; j < size / 8*7; j++)
			{
				DrawColor(img, i + row, j + col, 0);
			}
		}
		else
		{
			for (int j = 0; j < size/8; j++)
			{
				DrawColor(img, i + row, j + col, 0);
				DrawColor(img, i + row, j + col + size / 8*6, 0);
			}

		}
		if (i >= size / 8*2 && i < size / 8*5)
		{
			for (int j = size / 8*2; j < size / 8*5; j++)
			{
				DrawColor(img, i + row, j + col, 0);
			}
		}
	}
	return;
}
void AddBenchmark(Mat& img)
{
	PrintBenchmark(img, 0, 0,BENCHMARK_SIZE);
	PrintBenchmark(img, PIXEL_ROWS/BLOCK_ROWS*(BLOCK_ROWS-2) + BENCHMARK_SIZE/8, 0,BENCHMARK_SIZE);
	PrintBenchmark(img, 0, PIXEL_ROWS/BLOCK_ROWS* (BLOCK_ROWS - 2) + BENCHMARK_SIZE/8,BENCHMARK_SIZE);
	for (int total_bit = 0; total_bit < BLOCK_BITS; total_bit++)
	{
		if (total_bit / BLOCK_ROWS < 2 && (total_bit % BLOCK_ROWS < 2 || total_bit % BLOCK_ROWS >= 14)
			|| total_bit / BLOCK_ROWS >= 14 && total_bit % BLOCK_ROWS < 2)
			continue;
		if (!(total_bit % 2) && !((total_bit / 16) % 2) || (total_bit % 2) && ((total_bit / 16) % 2))
			DrawBlock(img, total_bit / BLOCK_ROWS, total_bit % BLOCK_ROWS, 0);
	}

}

int main()
{
	//unsigned char str[] = "Hh十分惭愧，我们暂时还不能明白黄指导项目的高深奥义。所以说，还是too young啦！你们呐，Naive！！！";
	FILE* fp = fopen("in.bin", "r");
	VideoWriter writer;
	int fps = 5;
	Mat outputImg(OUTPUT_ROWS, OUTPUT_COLS, CV_8UC1);
	writer.open("in.mp4", VideoWriter::fourcc('A', 'V', 'C', '1'), fps, outputImg.size(), false);
	int flames = 0;
	bool exit_sign = false;
	while (!exit_sign)
	{
		cout << "OK" << endl;
		Mat srcImg(PIXEL_ROWS, PIXEL_ROWS, CV_8UC1, Scalar(255));

		AddBenchmark(srcImg);

		int total_bit = 0;
		for (int i = 0; i < BLOCK_BITS / 8 - 2; i++)
		{
			unsigned char t;

			if (fscanf(fp, "%c", &t) == EOF)
			{
				exit_sign = true;
				break;
			}
			//cout << (int)t << endl;

			for (int k = 0; k < 8; k++)
			{
				while (total_bit / BLOCK_ROWS < 2 && (total_bit % BLOCK_ROWS < 2 || total_bit % BLOCK_ROWS >= 14)
					|| total_bit / BLOCK_ROWS >= 14 && total_bit % BLOCK_ROWS < 2)
				{
					total_bit++;
				}

				if (t & 128 && (!(total_bit % 2) && !((total_bit / BLOCK_ROWS) % 2) || (total_bit % 2) && ((total_bit / BLOCK_ROWS) % 2))
					|| !(t & 128) && (!(total_bit % 2) && ((total_bit / BLOCK_ROWS) % 2) || (total_bit % 2) && !((total_bit / BLOCK_ROWS) % 2)))
				{

					DrawBlock(srcImg, total_bit / BLOCK_ROWS, total_bit % BLOCK_ROWS, 0);
				}
				else
				{
					DrawBlock(srcImg, total_bit / BLOCK_ROWS, total_bit % BLOCK_ROWS, 255);
				}
				t = t << 1;

				total_bit++;

			}

			if (total_bit > BLOCK_BITS) break;
		}
		//imshow("srcImg", srcImg);
		//waitKey(0);

		Mat outputImg(OUTPUT_ROWS, OUTPUT_COLS, CV_8UC1, Scalar(240));
		for (int i = 0; i < PIXEL_ROWS; i++)
		{
			for (int j = 0; j < PIXEL_ROWS; j++)
			{
				*(outputImg.data + outputImg.step[0] * (i + (OUTPUT_ROWS - PIXEL_ROWS)/2) + outputImg.step[1] * (j + (OUTPUT_COLS - PIXEL_ROWS)/2))
					= *(srcImg.data + srcImg.step[0] * i + srcImg.step[1] * j);
			}
		}
		//imshow("outputImg", outputImg);
		//waitKey(0);

		imwrite("test" + to_string(flames) + ".jpg", outputImg);
		writer << outputImg;
		flames++;
	}

	fclose(fp);

	return 0;
}
