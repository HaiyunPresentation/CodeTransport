#include <iostream>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#define PIXEL_ROWS 1024
#define BLOCK_ROWS 64

#define BLOCK_SIZE (BLOCK_ROWS*BLOCK_ROWS)
#define OUTPUT_ROWS 1200
#define OUTPUT_COLS 1200
#define ANCHOR_SIZE 128

#define CUMU_END_BLOCKS 4088

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
	PrintAnchor(img, PIXEL_ROWS * 15 / 16 + ANCHOR_SIZE / 16, PIXEL_ROWS * 15 / 16 + ANCHOR_SIZE / 16, ANCHOR_SIZE / 2);
	//imshow("Benchmark", img);
	//waitKey();
}
void AddEOFLocation(int total_bit)
{

}

int main()
{
	FILE* fp = fopen("in.bin", "r");
	VideoWriter writer;

	writer.open("in.mp4", VideoWriter::fourcc('A', 'V', 'C', '1'), OUTPUT_FPS, Size(1200, 1200), true);
	int flames = 0;
	bool exit_sign = false;
	while (!exit_sign)
	{
		Mat bgrImg[3];
		for (int i = 0; i < 3; i++)
		{
			bgrImg[i] = Mat(PIXEL_ROWS, PIXEL_ROWS, CV_8UC1, Scalar(255));
			PrintBenchmark(bgrImg[i]);
			//if (!exit_sign)
			//{
				int total_blocks = 0;
				//while (total_blocks <= BLOCK_SIZE)
				while (total_blocks < CUMU_END_BLOCKS)
				{
					unsigned char chr;

					if (fscanf(fp, "%c", &chr) == EOF)
					{
						exit_sign = true;
						AddEOFLocation(total_blocks);
						chr = 255;
						//break;
					}
					for (int k = 0; k < 8; k++)
					{
						while ((total_blocks / BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
							|| total_blocks / BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS))
							&& (total_blocks % BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
								|| total_blocks % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)))
						{
							total_blocks++;
						}

						if (chr & 128 && (!(total_blocks % 2) && !((total_blocks / BLOCK_ROWS) % 2) || (total_blocks % 2) && ((total_blocks / BLOCK_ROWS) % 2))
							|| !(chr & 128) && (!(total_blocks % 2) && ((total_blocks / BLOCK_ROWS) % 2) || (total_blocks % 2) && !((total_blocks / BLOCK_ROWS) % 2)))
						{

							DrawBlock(bgrImg[i], total_blocks / BLOCK_ROWS, total_blocks % BLOCK_ROWS, 0);
						}
						else
						{
							DrawBlock(bgrImg[i], total_blocks / BLOCK_ROWS, total_blocks % BLOCK_ROWS, 255);
						}
						chr = chr << 1;
						total_blocks++;
					}
				
				}
			//}
			//imshow("bgrImg", bgrImg[i]);
			//waitKey(0);
			
		}
		Mat mergeImg(PIXEL_ROWS, PIXEL_ROWS, CV_8UC3, Scalar(255, 255, 255));
		merge(bgrImg,3, mergeImg);
		//imshow("merge", mergeImg);
		//waitKey(0);

		Mat outputImg(OUTPUT_ROWS, OUTPUT_COLS, CV_8UC3, Scalar(240, 240, 240));
		for (int i = 0; i < PIXEL_ROWS; i++)
		{
			for (int j = 0; j < PIXEL_ROWS; j++)
			{
				*(outputImg.data + outputImg.step[0] * (i + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (j + (OUTPUT_COLS - PIXEL_ROWS) / 2))
					= *(mergeImg.data + mergeImg.step[0] * i + mergeImg.step[1] * j);
				*(outputImg.data + outputImg.step[0] * (i + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (j + (OUTPUT_COLS - PIXEL_ROWS) / 2) + 1)
					= *(mergeImg.data + mergeImg.step[0] * i + mergeImg.step[1] * j + 1);
				*(outputImg.data + outputImg.step[0] * (i + (OUTPUT_ROWS - PIXEL_ROWS) / 2) + outputImg.step[1] * (j + (OUTPUT_COLS - PIXEL_ROWS) / 2) + 2)
					= *(mergeImg.data + mergeImg.step[0] * i + mergeImg.step[1] * j + 2);
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
