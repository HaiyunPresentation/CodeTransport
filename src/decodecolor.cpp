#include "decode.h"
#include "Detector.h"

#define BLOCK_ROWS 64
#define BLOCK_SIZE (BLOCK_ROWS*BLOCK_ROWS)
#define PIXEL_ROWS 256
#define ANCHOR_SIZE 32

#define CUMU_END_BLOCKS 4088

#define OUTPUT_FPS 6
#define VERSION "200310"

using namespace std;
using namespace cv;


Detector detector;


int Decode(Mat img, bool& end, int order)
{
	FILE* fp = fopen("out.bin", "a+");
	vector<cv::Mat> resBGR;
	if (!detector.GetCropCode(img, resBGR))
	{
		cout << "Can't detect " << order << endl;
		imwrite(VERSION + to_string(order) + "F.jpg", img);
		return -1;
	}
	cout << "Detect " << order << endl;
	//for (int i = 0; i < 4; i++)
	imwrite(to_string(order) + "BGR" + to_string(3) + ".jpg", resBGR[3]);
	for (int i = 0; i < 3; i++)
	{
		if (end) break;
		int total_blocks = 0;
		while (total_blocks < CUMU_END_BLOCKS)
		{
			unsigned char chr = '\0';
			for (int k = 0; k < 8; k++)
			{
				while ((total_blocks / BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
					|| total_blocks / BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS))
					&& (total_blocks % BLOCK_ROWS >= BLOCK_ROWS - ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)
						|| total_blocks % BLOCK_ROWS < ANCHOR_SIZE / (PIXEL_ROWS / BLOCK_ROWS)))
				{
					total_blocks++;
				}
				int val = (*(resBGR[i].data + resBGR[i].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks / BLOCK_ROWS) + 1) + resBGR[i].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks % BLOCK_ROWS) + 1)) +
					+*(resBGR[i].data + resBGR[i].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks / BLOCK_ROWS) + 1) + resBGR[i].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks % BLOCK_ROWS) + 2))
					+ *(resBGR[i].data + resBGR[i].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks / BLOCK_ROWS) + 2) + resBGR[i].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks % BLOCK_ROWS) + 1))
					+ *(resBGR[i].data + resBGR[i].step[0] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks / BLOCK_ROWS) + 2) + resBGR[i].step[1] * (PIXEL_ROWS / BLOCK_ROWS * (total_blocks % BLOCK_ROWS) + 2)))
					/ 4.0;
				chr = chr << 1;
				if (val < 128 && (!(total_blocks % 2) && !((total_blocks / BLOCK_ROWS) % 2) || (total_blocks % 2) && ((total_blocks / BLOCK_ROWS) % 2))
					|| val >= 128 && !(!(total_blocks % 2) && !((total_blocks / BLOCK_ROWS) % 2) || (total_blocks % 2) && ((total_blocks / BLOCK_ROWS) % 2)))
				{

					chr = chr | 1;

				}
				total_blocks++;
			}

			cout << (int)chr << endl;
			if (chr == 255)
			{
				//end = true;
				//break;
			}
			fprintf(fp, "%c", chr);
			
		}
	}

	//cout << total_blocks << endl;

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
	} while (!detector.IsCode(srcImg,newOrder++));
	while (true)
	{

		vc.read(srcImg);
		vc.read(srcImg);
		if (!srcImg.data) break;

		//imwrite("op" + to_string(order) + ".jpg",srcImg);


		if (Decode(srcImg, end, order) == -1)
		{
			vc.read(srcImg);
			order++;
			if (!srcImg.data) break;
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
	FILE* fp = fopen("out.bin", "w");
	fclose(fp);
	Detector detector;
	NaiveCodeVideoCapture();
	/*Mat img = imread("x.jpg");
	if (img.empty())
	{
		cout << "Image Not Found";
		return 0;
	}
	cout<<detector.IsCode(img);*/
	/*vector<cv::Mat> dstBGR;
	Mat img;
	int ord = 0;
	for (; ord <= 5; ord++)
	{
		img = imread(".\\test" + to_string(ord) + ".jpg");
		if (img.empty())
		{
			cout << "Image Not Found";
			//break;
		}
		else
		{
			bool end = false;
			cout << ord << " : " << detector.GetCropCode(img, dstBGR) << " ";

			Decode(img, end,ord);

		}
		cout << endl;

	}*/

	cout << endl << "Done" << endl;

	return 0;
}