#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#define BLOCK_ROWS 16
#define BLOCK_BITS 256

using namespace std;
using namespace cv;

//An approximate range
bool IsQRRate(float rate)
{
	return rate > 0.4 && rate < 1.9;
}

//Judge the color ratio of X-direction
bool IsQRColorRateX(Mat& image, int flag)
{
	int nr = image.rows / 2;
	int nc = image.cols;

	vector<int> vValueCount;
	vector<uchar> vColor;
	int count = 0;
	uchar lastColor = 0;

	uchar* data = image.ptr<uchar>(nr);
	for (int i = 0; i < nc; i++)
	{
		vColor.push_back(data[i]);
		uchar color = data[i];
		if (color > 0)
			color = 255;

		if (i == 0)
		{
			lastColor = color;
			count++;
		}
		else
		{
			if (lastColor != color)
			{
				vValueCount.push_back(count);
				count = 0;
			}
			count++;
			lastColor = color;
		}
	}

	if (count != 0)
		vValueCount.push_back(count);

	if (vValueCount.size() < 5 || vValueCount.size() > 7)
		return false;

	//Color ratio is 1:1:3:1:1
	int index = -1;
	int maxCount = -1;
	for (int i = 0; i < vValueCount.size(); i++)
	{
		if (i == 0)
		{
			index = i;
			maxCount = vValueCount[i];
		}
		else
		{
			if (vValueCount[i] > maxCount)
			{
				index = i;
				maxCount = vValueCount[i];
			}
		}
	}

	//Left & right
	if (index < 2)
		return false;
	if ((vValueCount.size() - index) < 3)
		return false;

	//Color ratio is 1:1:3:1:1
	float rate = ((float)maxCount) / 3.0;

	if (!IsQRRate(vValueCount[index - 2] / rate))
		return false;
	if (!IsQRRate(vValueCount[index - 1] / rate))
		return false;
	if (!IsQRRate(vValueCount[index + 1] / rate))
		return false;
	if (!IsQRRate(vValueCount[index + 2] / rate))
		return false;

	return true;
}

//Judge the color ratio of Y-direction
bool IsQRColorRateY(Mat& image, int flag)
{
	int nc = image.cols / 2;
	int nr = image.rows;

	vector<int> vValueCount;
	int count = 0;
	uchar lastColor = 0;

	for (int i = 0; i < nr; i++)
	{
		uchar* data = image.ptr<uchar>(i, nc);
		uchar color;
		if (data[0] > 0 || data[1] > 0 || data[2] > 0)
			color = 255;
		else
			color = 0;

		if (i == 0)
		{
			lastColor = color;
			count++;
		}
		else
		{
			if (lastColor != color)
			{
				vValueCount.push_back(count);
				count = 0;
			}
			count++;
			lastColor = color;
		}
	}

	if (count != 0)
		vValueCount.push_back(count);

	if (vValueCount.size() < 5 || vValueCount.size() > 7)
		return false;

	//Color ratio is 1:1:3:1:1
	int index = -1;
	int maxCount = -1;
	for (int i = 0; i < vValueCount.size(); i++)
	{
		if (i == 0)
		{
			index = i;
			maxCount = vValueCount[i];
		}
		else
		{
			if (vValueCount[i] > maxCount)
			{
				index = i;
				maxCount = vValueCount[i];
			}
		}
	}

	//Left & right
	if (index < 2)
		return false;
	if ((vValueCount.size() - index) < 3)
		return false;

	//Color ratio is 1:1:3:1:1
	float rate = ((float)maxCount) / 3.0;

	if (!IsQRRate(vValueCount[index - 2] / rate))
		return false;
	if (!IsQRRate(vValueCount[index - 1] / rate))
		return false;
	if (!IsQRRate(vValueCount[index + 1] / rate))
		return false;
	if (!IsQRRate(vValueCount[index + 2] / rate))
		return false;

	return true;
}

bool IsQRColorRate(cv::Mat& image, int flag)
{
	bool x = IsQRColorRateX(image, flag);
	if (!x)
		return false;
	bool y = IsQRColorRateY(image, flag);
	return y;
}

Mat CropImage(Mat& img, RotatedRect& rotatedRect)
{
	Mat warpPerspective_mat(3, 3, CV_32FC1);

	Mat warpPerspective_dst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
	Point2f srcPoint[4];
	rotatedRect.points(srcPoint);
	vector<Point2f> srcTri;
	srcTri.push_back(srcPoint[0]);
	srcTri.push_back(srcPoint[1]);
	srcTri.push_back(srcPoint[2]);
	srcTri.push_back(srcPoint[3]);
	vector<Point2f> dstTri;
	dstTri.push_back(Point2f(rotatedRect.size.width - 1, rotatedRect.size.height - 1));
	dstTri.push_back(Point2f(0, rotatedRect.size.height - 1));
	dstTri.push_back(Point2f(0, 0));
	dstTri.push_back(Point2f(rotatedRect.size.width - 1, 0));

	warpPerspective_mat = getPerspectiveTransform(srcTri, dstTri);
	warpPerspective(img, warpPerspective_dst, warpPerspective_mat, warpPerspective_dst.size());
	//imshow("", warpPerspective_dst);
	//waitKey(0);
	//rotatedRect.points[]
	//Mat resImg=Mat(rotatedRect.size.width,,CV_8UC1);
	//for(int i=0;i<resImg)
	return warpPerspective_dst;
}

bool IsQRPoint(vector<Point>& contour, Mat& img, int i)
{
	//Limit the minimum size
	RotatedRect rotatedRect = minAreaRect(contour);
	if (rotatedRect.size.height < 10 || rotatedRect.size.width < 10)
		return false;

	//Crop the 2-dimensional code
	cv::Mat cropImg = CropImage(img, rotatedRect);
	int flag = i++;

	//Judge whether the color ratio is 1:1:3:1:1
	bool result = IsQRColorRate(cropImg, flag);
	return result;
}

int FindQRPoint(Mat& srcImg, vector<vector<Point>>& qrPoint)
{

	Mat src_gray;
	cvtColor(srcImg, src_gray, COLOR_BGR2GRAY);
	//namedWindow("src_gray");
	//imshow("src_gray", src_gray);

	//Mat canny_output;
	//Canny(src_gray,canny_output,0,255);
	//imshow("Canny_output", canny_output);
	//waitKey(0);

	//Binarization
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();
	//namedWindow("Threshold_output");
	//imshow("Threshold_output", threshold_output);

	//Search the contour
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//Parent contour of an anchor has two child contours
	int parentIndex = -1;
	int ic = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		//cout << hierarchy[i][2] << " ";
		if (hierarchy[i][2] != -1)
		{
			if (ic == 0) parentIndex = i;
			ic++;
		}
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIndex = -1;
		}
		if (parentIndex != -1)
		{
			bool isQR = IsQRPoint(contours[parentIndex], threshold_output_copy, parentIndex);

			//Save three anchors
			if (isQR)
				qrPoint.push_back(contours[parentIndex]);

			ic = 0;
			parentIndex = -1;
		}
	}

	return 0;
}

Mat resizeQRCode(Mat& img, vector<vector<Point>>& qrPoint)
{
	vector<Point2f> totalPoints;
	Mat img2 = img;
	for (int i = 0; i < qrPoint.size(); i++)
	{
		for (int j = 0; j < qrPoint[i].size(); j++)
		{
			//circle(img, qrPoint[i][j], 1, Scalar(255));
			totalPoints.push_back(qrPoint[i][j]);
		}
	}
	RotatedRect rect = minAreaRect(totalPoints);
	Mat cropImg = CropImage(img, rect);
	//Decode(img,end);
	Mat threshold_output;
	cvtColor(cropImg, threshold_output, COLOR_BGR2GRAY);
	threshold(threshold_output, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	resize(threshold_output, threshold_output, Size2i(256, 256));
	//imshow("", threshold_output);
	//waitKey();
	return threshold_output;
}

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
	threshold(dst, dst, 0, 255, THRESH_BINARY | THRESH_OTSU);

	return true;
}

bool GetCropCode2(Mat& srcImg, Mat& dst)
{
	vector<vector<Point>> qrPoint;
	FindQRPoint(srcImg, qrPoint);
	if (qrPoint.size() == 3)
	{
		dst = resizeQRCode(srcImg, qrPoint);
		return true;
	}
	else return false;
}

int Decode(Mat img, bool& end, int order)
{
	FILE* fp = fopen("v0229out.bin", "a+");
	Mat resizeImg;
	if (!GetCropCode2(img, resizeImg))
	{
		cout << "Can't detect " << order << endl;
		return -1;
	}
	cout << "Detect " << order << endl;
	//imshow("",resizeImg);
	//waitKey(0);
	//imwrite("v0229o" + to_string(order) + ".jpg", resizeImg);
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
				+*(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 7) + resizeImg.step[1] * (16 * (total_bit % 16) + 7))
				//+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
				//+* (resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 9)))
				//5;
				;
			circle(circleImg, Point(16 * (total_bit % 16) + 7, 16 * (total_bit / 16) + 7), 2, Scalar(255, 0, 0), -1);
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

		if (chr == 255)
		{
			end = true;
			break;
		}
		fprintf(fp, "%c", chr);

	}
	cout << endl << total_bit << endl;

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
	while (true)
	{

		vc.read(srcImg);
		if (!srcImg.data) break;

		//imwrite("op" + to_string(order) + ".jpg",srcImg);

		Decode(srcImg, end, order);
		order++;
		if (end) break;

		for (int i = 1; i < 6; i++)
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

	cout << endl << "Done" << endl;

	return 0;
}