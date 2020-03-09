#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <windows.h>
#define PIXEL_ROWS 512
#define BLOCK_ROWS 64
#define BLOCK_BITS (BLOCK_ROWS*BLOCK_ROWS)
#define OUTPUT_ROWS 800
#define OUTPUT_COLS 800
#define ANCHOR_SIZE 64
#define OUTPUT_FPS 6

using namespace std;
using namespace cv;


//Set the value of a pixel is a gray image
//Black is 0, and white is 255
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

void PrintAnchor(Mat& img, int row, int col, int size)
{
	for (int i = 0; i < size * 7 / 8; i++)
	{
		for (int j = 0; j < size * 7 / 8; j++)
		{
			DrawColor(img, i + row, j + col, 255);
		}
	}
	for (int i = 0; i < size / 8 * 7; i++)
	{
		if (i < size / 8 || (i >= size / 8 * 6 && i < size / 8 * 7))
		{
			for (int j = 0; j < size / 8 * 7; j++)
			{
				DrawColor(img, i + row, j + col, 0);
			}
		}
		else
		{
			for (int j = 0; j < size / 8; j++)
			{
				DrawColor(img, i + row, j + col, 0);
				DrawColor(img, i + row, j + col + size / 8 * 6, 0);
			}

		}
		if (i >= size / 8 * 2 && i < size / 8 * 5)
		{
			for (int j = size / 8 * 2; j < size / 8 * 5; j++)
			{
				DrawColor(img, i + row, j + col, 0);
			}
		}
	}
	return;
}

void PrintBenchmark(Mat& img)
{
	PrintAnchor(img, 0, 0, ANCHOR_SIZE);
	PrintAnchor(img, PIXEL_ROWS / BLOCK_ROWS * (BLOCK_ROWS) * 7 / 8 + ANCHOR_SIZE / 8, 0, ANCHOR_SIZE);
	PrintAnchor(img, 0, PIXEL_ROWS / BLOCK_ROWS * (BLOCK_ROWS) * 7 / 8 + ANCHOR_SIZE / 8, ANCHOR_SIZE);
	for (int total_bit = 0; total_bit < BLOCK_BITS; total_bit++)
	{
		if (total_bit / BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
			&& (total_bit % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
				|| total_bit % BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS))
			|| total_bit / BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
			&& total_bit % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS))
			//if((total_bit/32<4|| total_bit / 32 >=28)&& (total_bit % 32 < 4 || total_bit % 32 >= 28))
			continue;
		if (!(total_bit % 2) && !((total_bit / BLOCK_ROWS) % 2) || (total_bit % 2) && ((total_bit / BLOCK_ROWS) % 2))
			DrawBlock(img, total_bit / BLOCK_ROWS, total_bit % BLOCK_ROWS, 0);
	}
	//DrawBlock(img, BLOCK_ROWS - 2, BLOCK_ROWS - 2, 255);
	//DrawBlock(img, BLOCK_ROWS - 1, BLOCK_ROWS - 2, 255);
	//DrawBlock(img, BLOCK_ROWS - 2, BLOCK_ROWS - 1, 255);
	//DrawBlock(img, BLOCK_ROWS-1,BLOCK_ROWS-1, 255);
	PrintAnchor(img, PIXEL_ROWS * 15 / 16 + ANCHOR_SIZE / 16, PIXEL_ROWS * 15 / 16 + ANCHOR_SIZE / 16, ANCHOR_SIZE / 2);
	//imshow("Benchmark", img);
	//waitKey();
}

int main()
{
	FILE* fp = fopen("in.bin", "r");
	VideoWriter writer;

	Mat outputImg(OUTPUT_ROWS, OUTPUT_COLS, CV_8UC1);
	writer.open("in.mp4", VideoWriter::fourcc('A', 'V', 'C', '1'), OUTPUT_FPS, outputImg.size(), false);
	int flames = 0;
	bool exit_sign = false;
	while (!exit_sign)
	{
		cout << "OK" << endl;
		Mat srcImg(PIXEL_ROWS, PIXEL_ROWS, CV_8UC1, Scalar(255));

		PrintBenchmark(srcImg);

		int total_bit = 0;
		for (int i = 0; i < (BLOCK_BITS - 4 * (PIXEL_ROWS / BLOCK_ROWS) * (PIXEL_ROWS / BLOCK_ROWS)) / 8; i++)
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
				//while (total_bit / BLOCK_ROWS < 2 && (total_bit % BLOCK_ROWS < 2 || total_bit % BLOCK_ROWS >= 28)
				//	|| total_bit / BLOCK_ROWS >= 28 && total_bit % BLOCK_ROWS < 2)
				while ((total_bit / BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
					|| total_bit / BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS))
					&& (total_bit % BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
						|| total_bit % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)))
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
				*(outputImg.data + outputImg.step[0] * (i + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (j + (OUTPUT_COLS - PIXEL_ROWS) / 2))
					= *(srcImg.data + srcImg.step[0] * i + srcImg.step[1] * j);
			}
		}
		imshow("outputImg", outputImg);
		waitKey(0);

		imwrite("test" + to_string(flames) + ".jpg", outputImg);
		writer << outputImg;
		flames++;
	}

	fclose(fp);

	return 0;
}
