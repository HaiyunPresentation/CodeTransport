#include <iostream>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#define PIXEL_ROWS 1024
#define BLOCK_ROWS 64
#define ANCHOR_SIZE 128
#define NON_BLOCK_AREA(x) ((x / BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS) \
	|| x / BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)) \
&& (x % BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS) \
	|| x % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)))

#define PIX_ONE_VALUE 0
#define PIX_ZRO_VALUE 255

#define BLOCK_SIZE (BLOCK_ROWS*BLOCK_ROWS)
#define CUMU_END_BLOCKS 4088

#define OUTPUT_ROWS 1200
#define OUTPUT_COLS 1200
#define OUTPUT_FPS 10

using namespace std;
using namespace cv;

int flameStep[3][2] = { {56,56},{56,60},{60,56} };

//Set the value of a pixel is a gray image
//Black is 0, and white is 255
void DrawColor(Mat& img, int i, int j, int value)
{
	if (value < 0 || value > 255)
	{
		cout << "Pixel value error" << endl;
		return;
	}
	*(img.data + img.step[0] * i + img.step[1] * j) = value;
}

void DrawBlock(Mat& img, int i, int j, int value)
{
	if (value < 0 || value > 255)
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
	PrintAnchor(img, PIXEL_ROWS * 15 / 16 + ANCHOR_SIZE / 16, PIXEL_ROWS * 15 / 16 + ANCHOR_SIZE / 16, ANCHOR_SIZE / 2);
	//imshow("Benchmark", img);
	//waitKey();
}

void AddEOFLocation(Mat& img, int flames, int totalBlocks)
{
	int flameRemainder = flames % 3;
	char rowOrder = totalBlocks / BLOCK_ROWS, colOrder = totalBlocks % BLOCK_ROWS;
	if (rowOrder >= BLOCK_ROWS || colOrder >= BLOCK_ROWS)
	{
		cout << "EOF location error!" << endl;
	}
	unsigned char judge = 0x80;
	for (int i = 0; i < 8; i++)
	{
		if (rowOrder & judge)
		{
			DrawBlock(img, flameStep[flameRemainder][0] +i/4, flameStep[flameRemainder][1] + i%4, PIX_ONE_VALUE);
		}
		if (colOrder & judge)
		{
			DrawBlock(img, flameStep[flameRemainder][0] +2+i/4, flameStep[flameRemainder][1] + i%4, PIX_ONE_VALUE);
		}
		judge >>= 1;
	}
}

int main()
{
	FILE* fp = fopen("in.bin", "r");
	VideoWriter writer;
	writer.open("in.mp4", VideoWriter::fourcc('A', 'V', 'C', '1'), OUTPUT_FPS, Size(1200, 1200), true);
	int flames = 0;
	bool exitSign = false;
	while (!exitSign)
	{
		Mat bgrImg[3];
		for (int channel = 0; channel < 3; channel++)
		{
			bgrImg[channel] = Mat(PIXEL_ROWS, PIXEL_ROWS, CV_8UC1, Scalar(255));
			PrintBenchmark(bgrImg[channel]);
			int totalBlocks = 0;
			//while (totalBlocks <= BLOCK_SIZE)
			while (totalBlocks < CUMU_END_BLOCKS)
			{
				unsigned char chr;
				if (fscanf(fp, "%c", &chr) == EOF)
				{
					while (NON_BLOCK_AREA(totalBlocks))
					{
						totalBlocks++;
					}
					
					if(!exitSign) AddEOFLocation(bgrImg[channel], flames, totalBlocks);
					exitSign = true;
					chr = 255;
				}
				for (int k = 0; k < 8; k++)
				{
					while (NON_BLOCK_AREA(totalBlocks))
					{
						totalBlocks++;
					}

					if (chr & 128 && (!(totalBlocks % 2) && !((totalBlocks / BLOCK_ROWS) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_ROWS) % 2))
						|| !(chr & 128) && (!(totalBlocks % 2) && ((totalBlocks / BLOCK_ROWS) % 2) || (totalBlocks % 2) && !((totalBlocks / BLOCK_ROWS) % 2)))
					{

						DrawBlock(bgrImg[channel], totalBlocks / BLOCK_ROWS, totalBlocks % BLOCK_ROWS, PIX_ONE_VALUE);
					}
					else
					{
						DrawBlock(bgrImg[channel], totalBlocks / BLOCK_ROWS, totalBlocks % BLOCK_ROWS, PIX_ZRO_VALUE);
					}
					chr = chr << 1;
					totalBlocks++;
				}

			}
			if (!exitSign)
			{
				int flameRemainder = flames % 3;
				for (int row = flameStep[flameRemainder][0]; row < flameStep[flameRemainder][0] + 4; row++)
					for (int col = flameStep[flameRemainder][1]; col < flameStep[flameRemainder][1] + 4; col++)
						DrawBlock(bgrImg[channel], row, col, 0);
			}
			//imshow("bgrImg", bgrImg[channel]);
			//waitKey(0);

		}
		Mat mergeImg(PIXEL_ROWS, PIXEL_ROWS, CV_8UC3, Scalar(255, 255, 255));
		merge(bgrImg, 3, mergeImg);
		//imshow("merge", mergeImg);
		//waitKey(0);

		Mat outputImg(OUTPUT_ROWS, OUTPUT_COLS, CV_8UC3, Scalar(240, 240, 240));
		for (int row = 0; row < PIXEL_ROWS; row++)
		{
			for (int col = 0; col < PIXEL_ROWS; col++)
			{
				*(outputImg.data + outputImg.step[0] * (row + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (col + (OUTPUT_COLS - PIXEL_ROWS) / 2))
					= *(mergeImg.data + mergeImg.step[0] * row + mergeImg.step[1] * col);
				*(outputImg.data + outputImg.step[0] * (row + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (col + (OUTPUT_COLS - PIXEL_ROWS) / 2) + 1)
					= *(mergeImg.data + mergeImg.step[0] * row + mergeImg.step[1] * col + 1);
				*(outputImg.data + outputImg.step[0] * (row + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (col + (OUTPUT_COLS - PIXEL_ROWS) / 2) + 2)
					= *(mergeImg.data + mergeImg.step[0] * row + mergeImg.step[1] * col + 2);
			}
		}
		//resize(outputImg, outputImg, Size(900, 900), 0, 0, INTER_LINEAR);
		//imshow("outputImg", outputImg);
		//waitKey(0);

		imwrite("test" + to_string(flames) + ".jpg", outputImg);
		writer << outputImg;
		flames++;
	}

	fclose(fp);

	return 0;
}
