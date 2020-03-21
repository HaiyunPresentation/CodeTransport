#include "Detector.h"

using namespace std;
using namespace cv;


bool Detector::GetCropCode(Mat& srcImg, vector<Mat>& dst)
{
	int mode = 1;
	vector<vector<Point>> qrPoint;
	vector<Mat> bgrChannel;
	cv::split(srcImg, bgrChannel);
	FindAnchors(srcImg, qrPoint, mode);
	cout << "qrPoint.size()==" << qrPoint.size();
	if (qrPoint.size() == 4)
	{
		cout << endl;
		dst = ResizeCode(srcImg, qrPoint, mode);
		return true;
	}
	for (int i = 0; i < 3; i++)
	{
		qrPoint.clear();
		FindAnchors(bgrChannel[i], qrPoint, mode);
		if (qrPoint.size() == 4)
		{
			cout << ", " << qrPoint.size() << endl;

			dst = ResizeCode(srcImg, qrPoint, mode);
			return true;
		}
		else
		{
			cout << ", " << qrPoint.size();
			qrPoint.clear();
		}
	}
	mode = 0;
	FindAnchors(srcImg, qrPoint, mode);
	cout << ", " << qrPoint.size();
	if (qrPoint.size() == 4)
	{
		cout << endl;
		dst = ResizeCode(srcImg, qrPoint, mode);
		return true;
	}
	for (int i = 0; i < 3; i++)
	{
		qrPoint.clear();
		FindAnchors(bgrChannel[i], qrPoint, mode);
		if (qrPoint.size() == 4)
		{
			cout << ", " << qrPoint.size() << endl;

			dst = ResizeCode(srcImg, qrPoint, mode);
			return true;
		}
		else
		{
			cout << ", " << qrPoint.size();
			qrPoint.clear();
		}
	}
	cout << endl;
	return false;

}

int Detector::FindAnchors(Mat& srcImg, vector<vector<Point>>& qrPoint, int mode)
{
	Mat srcGray;
	if (srcImg.type() != CV_8UC1)
	{
		cvtColor(srcImg, srcGray, COLOR_BGR2GRAY);
	}
	else
		srcGray = srcImg.clone();

	// Binarization
	Mat thresholdOutput;
	if (mode == 0)
		threshold(srcGray, thresholdOutput, 160, 255, THRESH_BINARY);
	else if (mode == 1)
		threshold(srcGray, thresholdOutput, 160, 255, THRESH_OTSU);
	else
		return -1;
	/*namedWindow("Threshold_output");
	imshow("Threshold_output", thresholdOutput);
	waitKey(0);*/

	// Search the contour
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(thresholdOutput, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	// Parent contour of an anchor has two child contours
	for (int index = 0; index < contours.size(); index++)
	{
		int child = hierarchy[index][2];
		if (child == -1) continue;
		int grandchild = hierarchy[child][2];
		if (grandchild == -1) continue;
		bool isQR = IsAnchor(contours[index], thresholdOutput, index);
		if (isQR)
			qrPoint.push_back(contours[index]);
	}

	return 0;
}

bool Detector::IsQRColorRate(Mat& img, int flag)
{
	bool x = IsQRColorRateX(img, flag);
	if (!x)
		return false;
	bool y = IsQRColorRateY(img, flag);
	return y;
}

//An approximate range
bool Detector::IsQRRate(float rate)
{
	return rate > 0.3 && rate < 1.9;
}

//Judge the color ratio of X-direction
bool Detector::IsQRColorRateX(Mat& img, int flag)
{
	int nr = img.rows / 2;
	int nc = img.cols;

	vector<int> vValueCount;
	vector<uchar> vColor;
	int count = 0;
	uchar lastColor = 0;

	uchar* data = img.ptr<uchar>(nr);
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
bool Detector::IsQRColorRateY(Mat& img, int flag)
{
	int nc = img.cols / 2;
	int nr = img.rows;

	vector<int> vValueCount;
	int count = 0;
	uchar lastColor = 0;

	for (int i = 0; i < nr; i++)
	{
		uchar* data = img.ptr<uchar>(i, nc);
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

Mat Detector::CropRect(Mat& img, RotatedRect& rotatedRect)
{
	Mat warpPerspectiveMat(3, 3, CV_32FC1);

	Mat warpPerspectiveDst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
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

	warpPerspectiveMat = getPerspectiveTransform(srcTri, dstTri);
	warpPerspective(img, warpPerspectiveDst, warpPerspectiveMat, warpPerspectiveDst.size());
	//imshow("", warpPerspectiveDst);
	//waitKey(0);

	return warpPerspectiveDst;
}

int Detector::JudgeTopLeft(Point2f* center)
{
	double distance01 = (center[0].x - center[1].x) * (center[0].x - center[1].x)
		+ (center[0].y - center[1].y) * (center[0].y - center[1].y);
	double distance02 = (center[0].x - center[2].x) * (center[0].x - center[2].x)
		+ (center[0].y - center[2].y) * (center[0].y - center[2].y);
	double distance12 = (center[1].x - center[2].x) * (center[1].x - center[2].x)
		+ (center[1].y - center[2].y) * (center[1].y - center[2].y);
	if (distance01 >= distance02 && distance01 >= distance12)
	{
		return 2;
	}
	else if (distance02 >= distance01 && distance02 >= distance12)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

Mat Detector::CropCode(Mat& img, RotatedRect& rotatedRect, Point2f center[4], int topLeftOrder, int buttonRightOrder)
{
	Mat warpMat(3, 3, CV_32FC1);

	Mat warpResult = Mat::zeros(256, 256, img.type());
	Point2f srcPoint[4], topLeftCenter = center[topLeftOrder], buttonRightCenter = center[buttonRightOrder];
	rotatedRect.points(srcPoint);

	int topLeftRectOrder = 0;
	double minDistance = ((double)srcPoint[0].x - topLeftCenter.x) * ((double)srcPoint[0].x - topLeftCenter.x)
		+ ((double)srcPoint[0].y - topLeftCenter.y) * ((double)srcPoint[0].y - topLeftCenter.y);
	for (int i = 1; i < 4; i++)
	{
		double distance = ((double)srcPoint[i].x - topLeftCenter.x) * ((double)srcPoint[i].x - topLeftCenter.x)
			+ ((double)srcPoint[i].y - topLeftCenter.y) * ((double)srcPoint[i].y - topLeftCenter.y);

		if (distance < minDistance)
		{
			minDistance = distance;
			topLeftRectOrder = i;
		}
	}

	Point2f topRightRect = srcPoint[(topLeftRectOrder + 1) % 4];
	int topRightOrder = 0;
	minDistance = ((double)topRightRect.x - center[0].x) * ((double)topRightRect.x - center[0].x)
		+ ((double)topRightRect.y - center[0].y) * ((double)topRightRect.y - center[0].y);
	for (int i = 1; i < 4; i++)
	{
		double distance = ((double)topRightRect.x - center[i].x) * ((double)topRightRect.x - center[i].x)
			+ ((double)topRightRect.y - center[i].y) * ((double)topRightRect.y - center[i].y);

		if (distance < minDistance)
		{
			minDistance = distance;
			topRightOrder = i;
		}
	}
	int buttonLeftOrder = 6 - buttonRightOrder - topLeftOrder - topRightOrder;
	//cout << buttonLeftOrder << buttonRightOrder << topLeftOrder << topRightOrder;

	vector<Point2f> srcTri;
	srcTri.push_back(center[topLeftOrder]);
	srcTri.push_back(center[topRightOrder]);
	srcTri.push_back(center[buttonRightOrder]);
	srcTri.push_back(center[buttonLeftOrder]);

	vector<Point2f> dst256Tri;
	dst256Tri.push_back(Point2f(13.5, 13.5));
	dst256Tri.push_back(Point2f(241.5, 13.5));
	dst256Tri.push_back(Point2f(248.5, 248.5));
	dst256Tri.push_back(Point2f(13.5, 241.5));

	warpMat = getPerspectiveTransform(srcTri, dst256Tri);
	warpPerspective(img, warpResult, warpMat, warpResult.size());
	//imshow("warpResult", warpResult);
	//waitKey(0);

	return warpResult;
}

bool Detector::IsAnchor(vector<Point>& contour, Mat& img, int i)
{
	//Limit the minimum size
	RotatedRect rotatedRect = minAreaRect(contour);
	if (rotatedRect.size.height < 10 || rotatedRect.size.width < 10)
		return false;

	//Crop the anchor
	Mat cropImg = CropRect(img, rotatedRect);
	int flag = i++;

	//Judge whether the color ratio is 1:1:3:1:1
	bool result = IsQRColorRate(cropImg, flag);
	return result;
}

int Detector::CenterPoint(vector<Point>& anchor, Point2f& p)
{
	double x = 0, y = 0;
	for (int i = 0; i < anchor.size(); i++)
	{
		x += (double)anchor[i].x;
		y += (double)anchor[i].y;
	}
	p.x = x / (double)anchor.size();
	p.y = y / (double)anchor.size();
	return 0;
}

vector<Mat> Detector::ResizeCode(Mat& srcImg, vector<vector<Point>>& qrPoint, int mode)
{
	vector<Point2f> totalPoints;
	Point2f center[4], center3[3];
	int buttonRightOrder = 0;
	int buttonRightSize = qrPoint[0].size();

	for (int i = 0; i < qrPoint.size(); i++)
	{
		if (qrPoint[i].size() < buttonRightSize)
		{
			buttonRightOrder = i;
			buttonRightSize = qrPoint[i].size();
		}
		CenterPoint(qrPoint[i], center[i]);
		//circle(img2, center[channel], 1, Scalar(0,0,255));
		for (int j = 0; j < qrPoint[i].size(); j++)
		{
			//circle(img2, qrPoint[channel][j], 1, Scalar(255));
			totalPoints.push_back(qrPoint[i][j]);
		}
	}
	/*Mat img2 = srcImg.clone();
	circle(img2, center[0], 10, Scalar(255, 0, 0), 10);
	circle(img2, center[1], 10, Scalar(0, 255, 0), 10);
	circle(img2, center[2], 10, Scalar(0, 0, 255), 10);
	circle(img2, center[3], 10, Scalar(255, 0, 255), 10);
	resize(img2, img2, Size(img2.cols / 2, img2.rows / 2));
	cv::imshow("circle", img2);
	waitKey(0);*/

	for (int i = 0, j = 0; i < 4; i++)
	{
		if (i == buttonRightOrder) continue;
		center3[j++] = center[i];
	}
	//int topLeftRectOrder = JudgeTopLeft(center3);
	int topLeftOrder = JudgeTopLeft(center3);
	if (topLeftOrder >= buttonRightOrder) topLeftOrder++;

	RotatedRect rect = minAreaRect(totalPoints);
	Mat cropImg = CropCode(srcImg, rect, center, topLeftOrder, buttonRightOrder);
	//Decode(srcImg,end);

	//imshow("crop", cropImg);
	//waitKey();

	Mat bgrOutput[3];
	split(cropImg, bgrOutput);

	//imshow("b", bgrOutput[0]);	
	//imshow("g", bgrOutput[1]);	
	//imshow("r", bgrOutput[2]);
	//waitKey();


	vector<Mat> resBGR;
	for (int channel = 0; channel < 3; channel++)
	{
		//Mat thresholdOutput;
		//cvtColor(bgrOutput[channel], thresholdOutput, COLOR_BGR2GRAY);
		if (mode == 0)
			threshold(bgrOutput[channel], bgrOutput[channel], 160, 255, THRESH_BINARY);
		else if (mode == 1)
			threshold(bgrOutput[channel], bgrOutput[channel], 160, 255, THRESH_OTSU);

		resize(bgrOutput[channel], bgrOutput[channel], Size2i(256, 256));
		//imshow("thres"+to_string(channel), bgrOutput[channel]);
		//waitKey();
		resBGR.push_back(bgrOutput[channel]);
	}
	resBGR.push_back(cropImg);

	return resBGR;
	//return srcImg;
}

bool Detector::IsCode(Mat& srcImg)
{
	vector<vector<Point>> qrPoint;
	int mode = 0;
	FindAnchors(srcImg, qrPoint, mode);
	cout << "qrPoint.size()==" << qrPoint.size();
	if (qrPoint.size() == 4)
	{
		cout << endl;
		return true;
	}
	vector<Mat> bgrChannel;
	cv::split(srcImg, bgrChannel);
	if (qrPoint.size() != 4 && qrPoint.size() > 0)
	{
		/*Mat srcGray;
		if (srcImg.type() != CV_8UC1)
		{
			cvtColor(srcImg, srcGray, COLOR_BGR2GRAY);
		}
		else
			srcGray = srcImg.clone();
		Mat thresholdOutput;
		threshold(srcGray, thresholdOutput, 160, 255, THRESH_BINARY);
		namedWindow("Threshold_output");
		imshow("Threshold_output", thresholdOutput);
		waitKey(0);*/

		for (int i = 0; i < 3; i++)
		{
			qrPoint.clear();
			FindAnchors(bgrChannel[i], qrPoint, mode);
			if (qrPoint.size() == 4)
			{
				cout << ", " << qrPoint.size() << endl;
				return true;
			}
			else
			{
				cout << ", " << qrPoint.size();
				qrPoint.clear();
			}
		}
		mode = 1;
		FindAnchors(srcImg, qrPoint, mode);
		cout << ", " << qrPoint.size();
		if (qrPoint.size() == 4)
		{
			cout << endl;
			return true;
		}
		for (int i = 0; i < 3; i++)
		{
			qrPoint.clear();
			FindAnchors(bgrChannel[i], qrPoint, mode);
			if (qrPoint.size() == 4)
			{
				cout << ", " << qrPoint.size() << endl;
				return true;
			}
			else
			{
				cout << ", " << qrPoint.size();
				qrPoint.clear();
			}
		}
		cout << endl;
	}
	cout << endl;
	return (qrPoint.size() == 4);
}
