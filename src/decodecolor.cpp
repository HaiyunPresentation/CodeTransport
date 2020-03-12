#include "decode.h"
#include "Detector.h"

#define BLOCK_ROWS 64
#define BLOCK_SIZE (BLOCK_ROWS*BLOCK_ROWS)
#define PIXEL_ROWS 256
#define ANCHOR_ROWS 8
#define ANCHOR_SIZE (PIXEL_ROWS/BLOCK_ROWS*ANCHOR_ROWS)
#define CUMU_END_BLOCKS 4088

#define OUTPUT_FPS 10
#define VERSION "200312"


using namespace std;
using namespace cv;


Detector detector;
int flameStep[3][2] = { {56,56},{56,60},{60,56} };

int isEnd(Mat& grayImg, int order, int& eofRow, int& eofCol)
{
	//imshow("", grayImg);
	//waitKey();
	unsigned char row = 0x00, col = row;
	int flameRemainder = order % 3;
	for (int i = 0; i < 8; i++)
	{
		row <<= 1;
		int val = (*(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 2))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 2)))
			/ 4.0;
		if (val < 128)
			row = row | 1;

	}

	if (row == 255)
	{
		return -1;
	}
	if (row == 0 || row >= BLOCK_ROWS)
	{
		return -2;
	}
	for (int i = 0; i < 8; i++)
	{
		col <<= 1;
		int val = (*(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4 + 2) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4 + 2) + 1) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 2))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4 + 2) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 1))
			+ *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][0] + i / 4 + 2) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[flameRemainder][1] + i % 4) + 2)))
			/ 4.0;
		if (val < 128)
			col = col | 1;

	}
	if (col == 255)
	{
		return -1;
	}
	if (col == 0 || col >= BLOCK_ROWS || (row < ANCHOR_ROWS || row >= BLOCK_ROWS - ANCHOR_ROWS) && (col < ANCHOR_ROWS || col >= BLOCK_ROWS - ANCHOR_ROWS))
	{
		return -2;
	}

	eofRow = row;
	eofCol = col;
	return 0;
}

int judgeOrder(Mat& srcImg, int order)
{
	Mat grayImg;
	cvtColor(srcImg, grayImg, COLOR_BGR2GRAY);
	threshold(grayImg, grayImg, 100, 255, THRESH_BINARY | THRESH_OTSU);
	//imshow("", grayImg);
	//waitKey();
	int fix = 0,remainder=order%3;
	int val = 0;
	//Mat img2 = grayImg.clone();
	for (int i = 0; i < 16; i++)
	{
		val += *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[(remainder + 2) % 3][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[(remainder + 2) % 3][1] + i % 4) + 2));
		//circle(img2, , 2, Scalar(255));
	}

	val /= 16;
	if (val < 128)	return -1;
	for (int i = 0; i < 16; i++)
	{
		val += *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[remainder][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[remainder][1] + i % 4) + 2));
	}
	val/= 16;
	if (val < 128)	return 0;
	val = 0;
	for (int i = 0; i < 16; i++)
	{
		val += *(grayImg.data + grayImg.step[0] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[(remainder+1)%3][0] + i / 4) + 2) + grayImg.step[1] * (PIXEL_ROWS / BLOCK_ROWS * (flameStep[(remainder + 1) % 3][1] + i % 4) + 2));
	}
	val /= 16;
	if (val < 128)	return 1;

}

int Decode(Mat& img, bool& end, int& order)
{

	FILE* fp = fopen("out.bin", "a+");
	vector<cv::Mat> resBGR;
	if (!detector.GetCropCode(img, resBGR))
	{
		cout << "Can't detect " << order << endl;
		imwrite(VERSION + to_string(order) + "F.jpg", img);
		return -1;
	}
	int judge = judgeOrder(resBGR[3], order);
	if (judge== -1)
	{
		imwrite(VERSION + to_string(order) + "F.jpg", img);
		return -1;
	}
	else if (judge == 1)
	{
		cout << "Miss a flame" << endl;
		order++;
	}
	cout << "Detect " << order << endl;
	//for (int channel = 0; channel < 4; channel++)
	imwrite(to_string(order) + "BGR" + to_string(3) + ".jpg", resBGR[3]);
	int eofRow = 0, eofCol = 0, eofConfindence = 0;
	for (int channel = 0; channel < 3; channel++)
	{
		if (end) break;
		int totalBlocks = 0;
		while (totalBlocks < CUMU_END_BLOCKS)
		{
			unsigned char chr = '\0';
			for (int k = 0; k < 8; k++)
			{
				while ((totalBlocks / BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
					|| totalBlocks / BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS))
					&& (totalBlocks % BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
						|| totalBlocks % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)))
				{
					totalBlocks++;
				}
				int val = (*(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks / BLOCK_ROWS) + 1) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks % BLOCK_ROWS) + 1))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks / BLOCK_ROWS) + 1) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks % BLOCK_ROWS) + 2))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks / BLOCK_ROWS) + 2) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks % BLOCK_ROWS) + 1))
					+ *(resBGR[channel].data + resBGR[channel].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks / BLOCK_ROWS) + 2) + resBGR[channel].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (totalBlocks % BLOCK_ROWS) + 2)))
					/ 4.0;
				chr = chr << 1;
				if (val < 128 && (!(totalBlocks % 2) && !((totalBlocks / BLOCK_ROWS) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_ROWS) % 2))
					|| val >= 128 && !(!(totalBlocks % 2) && !((totalBlocks / BLOCK_ROWS) % 2) || (totalBlocks % 2) && ((totalBlocks / BLOCK_ROWS) % 2)))
				{

					chr = chr | 1;

				}
				totalBlocks++;
			}

			cout << (int)chr << endl;
			if (chr == 255 && eofConfindence != -1)
			{
				if (eofConfindence == 0)
				{
					int state = isEnd(resBGR[channel], order, eofRow, eofCol);
					if (state == -1)
					{
						eofConfindence = -1;
						continue;
					}
					if (state == 0 && eofRow == (totalBlocks - 8) / BLOCK_ROWS && eofCol == (totalBlocks - 8) % BLOCK_ROWS)
					{
						end = true;
						break;
					}
					else
					{
						eofConfindence++;
					}
				}
				else
				{

					if (eofConfindence == 3)
					{
						end = true;
						break;
					}
					eofConfindence++;
					continue;
				}
			}

			fprintf(fp, "%c", chr);

		}
	}

	//cout << totalBlocks << endl;

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
	int newOrder = 0;
	do
	{
		vc.read(srcImg);
		if (!srcImg.data) break;
		//if (srcImg.rows > 1000)
		//	resize(srcImg, srcImg, Size(srcImg.cols / 5, srcImg.rows / 5));
		//imshow("src", srcImg);
	} while (!detector.IsCode(srcImg, newOrder++));
	while (true)
	{

		for (int i = 1; i < (30 / OUTPUT_FPS - 1) / 2; i++)
		{
			vc.read(srcImg);
			if (!srcImg.data) break;
		}

		if (!srcImg.data) break;

		//imwrite("op" + to_string(remainder) + ".jpg",srcImg);


		if (Decode(srcImg, end, order) == -1)
		{
			vc.read(srcImg);
			if (!srcImg.data) break;
			Decode(srcImg, end, order);
		}
		order++;
		if (end) break;

		for (int i = 1; i < 30 / OUTPUT_FPS; i++)
		{
			vc.read(srcImg);
			if (!srcImg.data) break;
		}
	}
	return 0;
}


int main()
{
	//FILE* fp = fopen("out.bin", "w");
	//fclose(fp);
	Detector detector;
	NaiveCodeVideoCapture();
	/*Mat img = imread("x.jpg");
	if (img.empty())
	{
		cout << "Image Not Found";
		return 0;
	}
	cout << detector.IsCode(img);*/
	/*vector<cv::Mat> dstBGR;
	Mat grayImg;
	int ord = 0;
	for (; ord <= 5; ord++)
	{
		grayImg = imread(".\\test" + to_string(ord) + ".jpg");
		if (grayImg.empty())
		{
			cout << "Image Not Found";
			//break;
		}
		else
		{
			bool end = false;
			cout << ord << " : " << detector.GetCropCode(grayImg, dstBGR) << " ";

			Decode(grayImg, end,ord);

		}
		cout << endl;

	}*/

	cout << endl << "Done" << endl;

	return 0;
}