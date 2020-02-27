#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#define BLOCK_ROWS 16
#define BLOCK_BITS 256

using namespace std;
using namespace cv;


void rotate_arbitrarily_angle(Mat src, Mat& dst, float angle)
{
	float radian = (float)(angle / 180.0 * CV_PI);
	int maxBorder = (int)(max(src.cols, src.rows) * sqrt(2));
	int dx = (maxBorder - src.cols) / 2;
	int dy = (maxBorder - src.rows) / 2;
	copyMakeBorder(src, dst, dy, dy, dx, dx, BORDER_CONSTANT);

	//Rotate
	Point2f center((float)(dst.cols / 2), (float)(dst.rows / 2));
	Mat affine_matrix = getRotationMatrix2D(center, angle, 1.0);
	warpAffine(dst, dst, affine_matrix, dst.size());

	//Caculate the minimum rectangle including the total image after rotating
	float sinVal = abs(sin(radian));
	float cosVal = abs(cos(radian));
	Size targetSize((int)(src.cols * cosVal + src.rows * sinVal),
		(int)(src.cols * sinVal + src.rows * cosVal));

	//Crop
	int x = (dst.cols - targetSize.width) / 2;
	int y = (dst.rows - targetSize.height) / 2;
	Rect rect(x, y, targetSize.width, targetSize.height);
	dst = Mat(dst, rect);
}


bool IsCode(Mat& img, vector<Point2f>& list)
{
	QRCodeDetector qrCodeDetector;
	qrCodeDetector.detect(img, list);
	if (list.empty())
	{
		cout << "Can't find code" << endl;
		return false;
	}
	else return true;
}

bool IsCode(Mat& img)
{
	vector<Point2f> list;
	return IsCode(img, list);
}

bool GetCropCode(Mat& srcImg, Mat& dst)
{
	Mat newImg = srcImg;

	QRCodeDetector qrDetector;
	vector<Point2f> list;
	if (!IsCode(newImg, list)) return false;

	Mat warpPerspective_mat(3, 3, CV_32FC1);
	Mat warpPerspective_dst = Mat::zeros(256, 256, newImg.type());

	vector<Point2f> dstTri;
	dstTri.push_back(Point2f(0, 0));
	dstTri.push_back(Point2f(255, 0));
	dstTri.push_back(Point2f(255, 255));
	dstTri.push_back(Point2f(0, 255));
	warpPerspective_mat = getPerspectiveTransform(list, dstTri);
	warpPerspective(newImg, warpPerspective_dst, warpPerspective_mat, warpPerspective_dst.size());
	

	cvtColor(warpPerspective_dst, dst, COLOR_BGR2GRAY);
	threshold(dst, dst, 150, 255, THRESH_BINARY | THRESH_OTSU);

	return true;
}

int Decode(Mat img,bool& end)
{
	FILE* fp = fopen("out.bin", "a+");
	Mat cropImg;
	if (!GetCropCode(img, cropImg))
	{
		//imshow("", img);
		//waitKey(0);
		return -1;
	}
	//imshow("", cropImg);
	//waitKey(0);
	
	Mat resizeImg;
	resize(cropImg, resizeImg, Size(256, 256));
	imshow("",resizeImg);
	waitKey(0);
	Mat circleImg;
	cvtColor(resizeImg, circleImg, COLOR_GRAY2BGR);
	int total_bit = 0;
	while (total_bit < 252)
	{
		unsigned char chr = '\0';
		for (int k = 0; k < 8; k++)
		{
			while (total_bit / BLOCK_ROWS < 2 && (total_bit % BLOCK_ROWS < 2 || total_bit % BLOCK_ROWS >= 14)
				|| total_bit / BLOCK_ROWS >= 14 && total_bit % BLOCK_ROWS < 2)
			{
				total_bit++;
			}

			int val = //(*(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 5) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
			    //+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 5) + resizeImg.step[1] * (16 * (total_bit % 16) + 9))
				+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 7) + resizeImg.step[1] * (16 * (total_bit % 16) + 7))
				//+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
				//+* (resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 9)))
				//5;
				;
			circle(circleImg, Point(16 * (total_bit % 16) + 7,16 * (total_bit / 16) + 7), 2, Scalar(255, 0, 0), -1);
			//imshow("circleImg", circleImg);
			//waitKey(0);
			//int val = *(code.data + code.step[0] * (total_bit / 16) + code.step[1] * (total_bit % 16));
			chr = chr << 1;
			if (val < 128 && (!(total_bit % 2) && !((total_bit / 16) % 2) || (total_bit % 2) && ((total_bit / 16) % 2))
				|| val >= 128 && !(!(total_bit % 2) && !((total_bit / 16) % 2) || (total_bit % 2) && ((total_bit / 16) % 2)))
			{
				
				chr = chr | 1;
				
			}
			
			total_bit++;
		}
		
		if (chr==BLOCK_BITS-1)
		{
			end = true;
			break;
		}
		fprintf(fp, "%c", chr);

	}
	cout << endl<< total_bit << endl;
	//imshow("circleImg", circleImg);
	//waitKey(0);
	fclose(fp);
	return 0;
}
int NaiveCodeVideoCapture()
{
	VideoCapture vc = VideoCapture("out.mp4");
	if (!vc.isOpened())
		return -1;

	Mat srcImg;
	bool end=false;
	int order = 0;
	while (true)
	{

		vc.read(srcImg);
		if (!srcImg.data) break;

		imwrite("op" + to_string(order) + ".jpg",srcImg);
		order++;
		Decode(srcImg,end);
		if (end) break;

		for (int i = 0; i < 6; i++)
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

	/*Mat img;
	int ord = 0;
	for (;ord<=16; ord++)
	{
		img= imread("test"+to_string(ord)+".jpg");
		if (img.empty())
		{
			cout << "Null";
			break;
		}
		bool end = false;
		cout << ord << " : ";

		Decode(img, end);
		cout << endl;
		
	}*/

	cout <<endl<< "Done" <<endl;

	return 0;
}