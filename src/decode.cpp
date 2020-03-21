#include "decode.h"
#include "Detector.h"

#define BLOCK_SIDE_LENGTH 64
#define BLOCK_SIZE (BLOCK_SIDE_LENGTH*BLOCK_SIDE_LENGTH)
#define PIXEL_ROWS 256
#define ANCHOR_ROWS 8
#define ANCHOR_SIZE (PIXEL_ROWS/BLOCK_SIDE_LENGTH*ANCHOR_ROWS)
#define CUMU_END_BLOCKS 4088
#define NON_BLOCK_AREA_LARGE(x) (x / BLOCK_SIDE_LENGTH < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_SIDE_LENGTH)\
|| x / BLOCK_SIDE_LENGTH >= BLOCK_SIDE_LENGTH - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_SIDE_LENGTH))\
&& (x % BLOCK_SIDE_LENGTH >= BLOCK_SIDE_LENGTH - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_SIDE_LENGTH)\
	|| x % BLOCK_SIDE_LENGTH < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_SIDE_LENGTH))

#define OUTPUT_FPS 10
#define VERSION "200320"


using namespace std;
using namespace cv;


Detector detector;
int flameStep[3][2] = { {56,56},{56,60},{60,56} };
int newOrder = -1;

int isEnd(Mat& grayImg, int order, int& eofRow, int& eofCol)
{
	//imshow("", srcImg);
	//waitKey();
	unsigned char row = 0x00, col = row;
	int flameRemainder = order % 3;
	for (int i = 0; i < 8; i++)
	{
		row <<= 1;
		int val = (*(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 2))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 2)))
			/ 4.0;
		if (val < 128)
			row = row | 1;

	}

	if (row == 255)
	{
		return -1;
	}
	if (row >= BLOCK_SIDE_LENGTH)
	{
		return -2;
	}
	for (int i = 0; i < 8; i++)
	{
		col <<= 1;
		int val = (*(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4 + 2) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4 + 2) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 2))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4 + 2) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][0] + i / 4 + 2) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[flameRemainder][1] + i % 4) + 2)))
			/ 4.0;
		if (val < 128)
			col = col | 1;

	}
	if (col == 255)
	{
		return -1;
	}
	if (col >= BLOCK_SIDE_LENGTH || (row < ANCHOR_ROWS || row >= BLOCK_SIDE_LENGTH - ANCHOR_ROWS) && (col < ANCHOR_ROWS || col >= BLOCK_SIDE_LENGTH - ANCHOR_ROWS))
	{
		return -2;
	}

	eofRow = row;
	eofCol = col;
	return 0;
}

/*
 * Judge the relative order between flames.
 * Return  0: normal
 * Return -1: backward
 * Return  1: forward(Miss a flame)
 * Return -2: All are white(Considering EOF)
 */
int judgeOrder(Mat& srcImg, int order)
{
	Mat grayImg = srcImg.clone();
	cvtColor(srcImg, grayImg, COLOR_BGR2GRAY);
	threshold(grayImg, grayImg, 150, 255, THRESH_BINARY | THRESH_OTSU);
	//imshow("", srcImg);
	//waitKey();
	int fix = 0, remainder = order % 3;
	int preVal = 0, curVal = 0, nxtVal = 0;

	for (int i = 0; i < 16; i++)
	{
		preVal += *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[(remainder + 2) % 3][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[(remainder + 2) % 3][1] + i % 4) + 2));
		//circle(img2, , 2, Scalar(255));
	}
	preVal /= 16;
	// remain footprint of the former flame
	if (preVal < 128)	return -1;

	for (int i = 0; i < 16; i++)
	{
		curVal += *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[remainder][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[remainder][1] + i % 4) + 2));
	}
	curVal /= 16;
	// normal
	if (curVal < 128)	return 0;

	for (int i = 0; i < 16; i++)
	{
		nxtVal += *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[(remainder + 1) % 3][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (flameStep[(remainder + 1) % 3][1] + i % 4) + 2));
	}
	nxtVal /= 16;
	if (nxtVal < 128)	return 1;

	// All are white(Considering EOF)
	return 2;

}

static unsigned char const table_byte[256] = {

	0x0, 0x7, 0xe, 0x9, 0x5, 0x2, 0xb, 0xc, 0xa, 0xd, 0x4, 0x3, 0xf, 0x8, 0x1, 0x6,

	0xd, 0xa, 0x3, 0x4, 0x8, 0xf, 0x6, 0x1, 0x7, 0x0, 0x9, 0xe, 0x2, 0x5, 0xc, 0xb,

	0x3, 0x4, 0xd, 0xa, 0x6, 0x1, 0x8, 0xf, 0x9, 0xe, 0x7, 0x0, 0xc, 0xb, 0x2, 0x5,

	0xe, 0x9, 0x0, 0x7, 0xb, 0xc, 0x5, 0x2, 0x4, 0x3, 0xa, 0xd, 0x1, 0x6, 0xf, 0x8,

	0x6, 0x1, 0x8, 0xf, 0x3, 0x4, 0xd, 0xa, 0xc, 0xb, 0x2, 0x5, 0x9, 0xe, 0x7, 0x0,

	0xb, 0xc, 0x5, 0x2, 0xe, 0x9, 0x0, 0x7, 0x1, 0x6, 0xf, 0x8, 0x4, 0x3, 0xa, 0xd,

	0x5, 0x2, 0xb, 0xc, 0x0, 0x7, 0xe, 0x9, 0xf, 0x8, 0x1, 0x6, 0xa, 0xd, 0x4, 0x3,

	0x8, 0xf, 0x6, 0x1, 0xd, 0xa, 0x3, 0x4, 0x2, 0x5, 0xc, 0xb, 0x7, 0x0, 0x9, 0xe,

	0xc, 0xb, 0x2, 0x5, 0x9, 0xe, 0x7, 0x0, 0x6, 0x1, 0x8, 0xf, 0x3, 0x4, 0xd, 0xa,

	0x1, 0x6, 0xf, 0x8, 0x4, 0x3, 0xa, 0xd, 0xb, 0xc, 0x5, 0x2, 0xe, 0x9, 0x0, 0x7,

	0xf, 0x8, 0x1, 0x6, 0xa, 0xd, 0x4, 0x3, 0x5, 0x2, 0xb, 0xc, 0x0, 0x7, 0xe, 0x9,

	0x2, 0x5, 0xc, 0xb, 0x7, 0x0, 0x9, 0xe, 0x8, 0xf, 0x6, 0x1, 0xd, 0xa, 0x3, 0x4,

	0xa, 0xd, 0x4, 0x3, 0xf, 0x8, 0x1, 0x6, 0x0, 0x7, 0xe, 0x9, 0x5, 0x2, 0xb, 0xc,

	0x7, 0x0, 0x9, 0xe, 0x2, 0x5, 0xc, 0xb, 0xd, 0xa, 0x3, 0x4, 0x8, 0xf, 0x6, 0x1,

	0x9, 0xe, 0x7, 0x0, 0xc, 0xb, 0x2, 0x5, 0x3, 0x4, 0xd, 0xa, 0x6, 0x1, 0x8, 0xf,

	0x4, 0x3, 0xa, 0xd, 0x1, 0x6, 0xf, 0x8, 0xe, 0x9, 0x0, 0x7, 0xb, 0xc, 0x5, 0x2

};

/*
 * Check CRC-4/ITU result.
 * Return 0 if right or return -1 if wrong.
 */
int crc4ITUCheck(unsigned char chr, char crc)
{
	if (table_byte[chr] != crc)
		return -1;
	return 0;
}

int Decode(Mat& img, bool& end, int& order, char* outputPath, char* valPath)
{

	vector<cv::Mat> resBGR;
	if (!detector.GetCropCode(img, resBGR))
	{
		printf("Can't detect %d\n", order);
		//imwrite(to_string(order)+"err" + to_string(newOrder++) + ".jpg", img);
		return -1;
	}
	int judge = judgeOrder(resBGR[3], order);
	int eofRow = 0, eofCol = 0, eofConfindence = 0;

	if (judge == -1)
	{
		printf("Backward!\n");
		//imwrite(to_string(order)+"backward" + to_string(newOrder++) + "F.jpg", img);
		return -1;
	}
	else if (judge == 1)
	{
		printf("Miss a flame\n");
		order++;
		newOrder++;
		printf("%d", order);
	}

	if (judge == -2)
	{
		printf("judge==-2\n");
		if (isEnd(resBGR[0], order, eofRow, eofCol) != 0)
			return -2;
	}


	//imshow("", resBGR[3]);
	//waitKey();
	printf("Detect %d\n", order);

	FILE* fp = fopen(outputPath, "ab+");
	FILE* fvp = fopen(valPath, "ab+");


	for (int channel = 0; channel < 3; channel++)
	{
		if (end) break;
		int endState = isEnd(resBGR[channel], order, eofRow, eofCol);
		int totalBlocks = 0;
		while (totalBlocks < CUMU_END_BLOCKS)
		{
			unsigned char chr = '\0', crc = '\0';
			for (int bit = 0; bit < 8; bit++)
			{
				while (NON_BLOCK_AREA_LARGE(totalBlocks))
				{
					totalBlocks++;
				}
				int val = (resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1)
					+ resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2)
					+ resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1)
					+ resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2)
					)/4;

				/*int val = (*(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2)))
					/ 4.0;*/
				chr = chr << 1;
				if (val < 128 && (!(totalBlocks % 2) && !((totalBlocks / BLOCK_SIDE_LENGTH) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_SIDE_LENGTH) % 2))
					|| val >= 128 && !(!(totalBlocks % 2) && !((totalBlocks / BLOCK_SIDE_LENGTH) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_SIDE_LENGTH) % 2)))
				{
					chr = chr | 1;
				}
				totalBlocks++;
			}

			if (chr == 0)
			{

				if (endState == 0 && totalBlocks - 8 >= eofRow * BLOCK_SIDE_LENGTH + eofCol)
				{
					end = true;
					break;
				}
			}

			for (int crcBit = 0; crcBit < 4; crcBit++)
			{
				while (NON_BLOCK_AREA_LARGE(totalBlocks))
				{
					totalBlocks++;
				}
				int val = (resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1)
					+ resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2)
					+ resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1)
					+ resBGR[channel].at<uchar>(PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2, PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2)
					) / 4;
				/*int val = (*(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 1) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 1))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks / BLOCK_SIDE_LENGTH) + 2) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_SIDE_LENGTH * (totalBlocks % BLOCK_SIDE_LENGTH) + 2)))
					/ 4.0;*/
				crc = crc << 1;
				if (val < 128 && (!(totalBlocks % 2) && !((totalBlocks / BLOCK_SIDE_LENGTH) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_SIDE_LENGTH) % 2))
					|| val >= 128 && !(!(totalBlocks % 2) && !((totalBlocks / BLOCK_SIDE_LENGTH) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_SIDE_LENGTH) % 2)))
				{

					crc = crc | 1;

				}
				totalBlocks++;
			}

			if (crc4ITUCheck(chr, crc))
				fprintf(fvp, "%c", 0x00);
			else fprintf(fvp, "%c", 0xff);

			fprintf(fp, "%c", chr);
		}
	}
	fclose(fp);
	fclose(fvp);
	return 0;
}

int NaiveCodeVideoCapture(char* inputPath, char* outputPath, char* valPath)
{
	VideoCapture vc = VideoCapture(inputPath);
	if (!vc.isOpened())
	{
		printf("Can't find the video!\n");
		return -1;
	}
	Detector detector;
	Mat srcImg;
	bool end = false;
	int order = 0;

	do
	{
		newOrder++;
		vc.read(srcImg);
		if (!srcImg.data) break;
	} while (!detector.IsCode(srcImg));

	/*for (int i = 1; i <= (30 / OUTPUT_FPS - 1) / 2; i++)
	{
		vc.read(srcImg);
		if (!srcImg.data) break;
	}*/

	while (true)
	{
		//imwrite("op" + to_string(order) + ".jpg",srcImg);
		int decodeState = 0;
		while ((decodeState = Decode(srcImg, end, order, outputPath, valPath)) == -1 || decodeState == -2)
		{
			vc.read(srcImg);
			if (!srcImg.data) break;
		}

		order++;
		printf("%d\n", order);
		if (end) break;
		vc.read(srcImg);
		if (!srcImg.data) break;
		/*for (int i = 1; i < 30 / OUTPUT_FPS; i++)
		{
			vc.read(srcImg);
			if (!srcImg.data) break;
		}*/
	}
	return 0;
}

void usage()
{
#define USAGE_FORMAT "  %-12s  %s\n"
	printf("%s\n", "NaiveCode Decoder");
	printf("%s\n", "Copyright (C) by HaiyunPresentation");
	printf("\n");
	printf("%s\n", "Usage:");
	printf("  %s\n", "decode.exe <input_path> <output_path> <val_path>");
	printf(USAGE_FORMAT, "input_path", "Path of your captured video");
	printf(USAGE_FORMAT, "output_path", "Path of output file");
	printf(USAGE_FORMAT, "val_path", "Path of val file");
	printf("\n");

}


int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		usage();
		return -1;
	}
	char* inputPath = argv[1];
	char* outputPath = argv[2];
	char* valPath = argv[3];
	/*char inputPath[] = "out.mp4";
	char outputPath[] = "out.bin";
	char valPath[] = "out.val";*/

	FILE* fp = fopen(outputPath, "w");
	if (fp == NULL)
	{
		printf("Can't open the output file!\n");
		return -1;
	}
	fclose(fp);
	FILE* fvp = fopen(valPath, "w");
	if (fvp == NULL)
	{
		printf("Can't open the val file!\n");
		return -1;
	}
	fclose(fvp);

	NaiveCodeVideoCapture(inputPath, outputPath, valPath);

	/*vector<cv::Mat> dstBGR;
	Mat srcImg;
	int ord = 0;
	for (; ord < 14; ord++)
	{
		srcImg = imread(".\\Frame" + to_string(ord+1) + ".jpg");
		if (srcImg.empty())
		{
			cout << "Image Not Found";
			//break;
		}
		else
		{
			bool end = false;
			cout << ord << " : " << detector.GetCropCode(srcImg, dstBGR) << " ";
			Decode(srcImg, end, ord, outputPath, valPath);
		}
		cout << endl;

	}
	*/
	printf("Done.\n");

	return 0;
}