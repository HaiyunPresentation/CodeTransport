#include "Detector.h"

using namespace std;
using namespace cv;



//An approximate range
bool Detector::IsQRRate(float rate)
{
	return rate > 0.3 && rate < 1.9;
}

//Judge the color ratio of X-direction
bool Detector::IsQRColorRateX(Mat& image, int flag)
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
bool Detector::IsQRColorRateY(Mat& image, int flag)
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

bool Detector::IsQRColorRate(Mat& image, int flag)
{
	bool x = IsQRColorRateX(image, flag);
	if (!x)
		return false;
	bool y = IsQRColorRateY(image, flag);
	return y;
}

Mat Detector::CropImage(Mat& img, RotatedRect& rotatedRect)
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
	//cv::imshow("", warpPerspective_dst);
	//waitKey(0);
	//rotatedRect.points[]
	//Mat resImg=Mat(rotatedRect.size.width,,CV_8UC1);
	//for(int i=0;i<resImg)
	return warpPerspective_dst;
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

Mat Detector::Crop4AnchorCode(Mat& img, RotatedRect& rotatedRect, Point2f center[4], int topLeftOrder, int buttonRightOrder)
{
	///Mat warpPerspective_mat(3, 3, CV_32FC1);
	Mat warp256mat(3, 3, CV_32FC1);
	//Mat warpRectMat(3, 3, CV_32FC1);

	//Mat warpPerspective_dst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
	//Mat warpRectdst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
	Mat warp256 = Mat::zeros(256, 256, img.type());
	Point2f srcPoint[4], topLeftCenter = center[topLeftOrder], buttonRightCenter = center[buttonRightOrder];
	rotatedRect.points(srcPoint);
	//Mat img2 = img.clone();
	//circle(img2, center[0], 10, Scalar(255, 0, 0));
	//circle(img2, center[1], 10, Scalar(0, 255, 0));
	//circle(img2, center[2], 10, Scalar(0, 0, 255));
	//circle(img2, center[3], 10, Scalar(255, 0, 255));
	//resize(img2, img2, Size(img2.cols / 2, img2.rows / 2));
	//cv::imshow("circle", img2);
	//waitKey(0);


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

	/*vector<Point2f> dstTri;
	dstTri.push_back(Point2f(rotatedRect.size.width * 13.5 / 256, rotatedRect.size.height * 13.5 / 256));
	dstTri.push_back(Point2f(rotatedRect.size.width * 241.5 / 256, rotatedRect.size.height * 13.5 / 256));
	dstTri.push_back(Point2f(rotatedRect.size.width * 248.5 / 256, rotatedRect.size.height * 248.5 / 256));
	dstTri.push_back(Point2f(rotatedRect.size.width * 13.5 / 256, rotatedRect.size.height * 241.5 / 256));*/

	vector<Point2f> dst256Tri;
	dst256Tri.push_back(Point2f(13.5, 13.5));
	dst256Tri.push_back(Point2f(241.5, 13.5));
	dst256Tri.push_back(Point2f(248.5, 248.5));
	dst256Tri.push_back(Point2f(13.5, 241.5));

	/*vector<Point2f> srcTriRect;
	srcTriRect.push_back(srcPoint[topLeftRectOrder]);
	srcTriRect.push_back(srcPoint[(topLeftRectOrder + 1) % 4]);
	srcTriRect.push_back(srcPoint[(topLeftRectOrder + 2) % 4]);
	srcTriRect.push_back(srcPoint[(topLeftRectOrder + 3) % 4]);

	vector<Point2f> dstTriRect;
	dstTriRect.push_back(Point2f(0, 0));
	dstTriRect.push_back(Point2f(rotatedRect.size.width - 1, 0));
	dstTriRect.push_back(Point2f(rotatedRect.size.width - 1, rotatedRect.size.height - 1));
	dstTriRect.push_back(Point2f(0, rotatedRect.size.height - 1));*/


	//warpPerspective_mat = getPerspectiveTransform(srcTri, dstTri);
	//warpPerspective(img, warpPerspective_dst, warpPerspective_mat, warpPerspective_dst.size());
	//resize(warpPerspective_dst, warpPerspective_dst, Size(256, 256));

	//warpRectMat = getPerspectiveTransform(srcTriRect, dstTriRect);
	//warpPerspective(img, warpRectdst, warpRectMat, warpRectdst.size());
	//resize(warpRectdst, warpRectdst, Size(256, 256));

	warp256mat = getPerspectiveTransform(srcTri, dst256Tri);
	warpPerspective(img, warp256, warp256mat, warp256.size());
	//cv::imshow("warp", warpPerspective_dst);
	//cv::imshow("rect", warpRectdst);
	//cv::imshow("warp256", warp256);
	waitKey(0);

	return warp256;
}

bool Detector::IsAnchor(vector<Point>& contour, Mat& img, int i)
{
	//Limit the minimum size
	RotatedRect rotatedRect = minAreaRect(contour);
	if (rotatedRect.size.height < 10 || rotatedRect.size.width < 10)
		return false;

	//Crop the anchor
	Mat cropImg = CropImage(img, rotatedRect);
	int flag = i++;

	//Judge whether the color ratio is 1:1:3:1:1
	bool result = IsQRColorRate(cropImg, flag);
	return result;
}

int Detector::FindAnchors(Mat& srcImg, vector<vector<Point>>& qrPoint)
{

	Mat src_gray;
	cvtColor(srcImg, src_gray, COLOR_BGR2GRAY);
	//namedWindow("src_gray");
	//cv::imshow("src_gray", src_gray);

	/*Mat canny_output;
	Canny(src_gray, canny_output, 0, 255);
	Mat canny_output_copy = canny_output.clone();
	cv::imshow("Canny_output", canny_output);
	//waitKey(0);*/

	//Binarization
	Mat threshold_output;
	threshold(src_gray, threshold_output, 100, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();
	//namedWindow("Threshold_output");
	//cv::imshow("Threshold_output", threshold_output);
	//waitKey(0);

	//Search the contour
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
	//for (int i = 0; i < contours.size(); i++)
	//{
	//	IsAnchor(contours[i], canny_output_copy, i);
	//}


	//Parent contour of an anchor has two child contours
	int parentIndex = -1;
	int ic = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		//cout << hierarchy[i][2] << " ";
		int child = hierarchy[i][2];
		if (child == -1) continue;
		int grandchild = hierarchy[child][2];
		if (grandchild == -1) continue;
		bool isQR = IsAnchor(contours[i], threshold_output_copy, i);
		if (isQR)
			qrPoint.push_back(contours[i]);
	}

	return 0;
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

vector<cv::Mat> Detector::resize4AnchorCode(Mat& img, vector<vector<Point>>& qrPoint)
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
		//circle(img2, center[i], 1, Scalar(0,0,255));
		for (int j = 0; j < qrPoint[i].size(); j++)
		{
			//circle(img2, qrPoint[i][j], 1, Scalar(255));
			totalPoints.push_back(qrPoint[i][j]);
		}
	}


	for (int i = 0, j = 0; i < 4; i++)
	{
		if (i == buttonRightOrder) continue;
		center3[j++] = center[i];
	}
	//int topLeftRectOrder = JudgeTopLeft(center3);
	int topLeftOrder = JudgeTopLeft(center3);
	if (topLeftOrder >= buttonRightOrder) topLeftOrder++;

	RotatedRect rect = minAreaRect(totalPoints);
	Mat cropImg = Crop4AnchorCode(img, rect, center, topLeftOrder, buttonRightOrder);
	//Decode(img,end);
	
	//cv::imshow("crop", cropImg);
	//waitKey();

	Mat bgrOutput[3];
	split(cropImg, bgrOutput);

	//cv::imshow("b", bgrOutput[0]);	
	//cv::imshow("g", bgrOutput[1]);	
	//cv::imshow("r", bgrOutput[2]);
	//waitKey();


	std::vector<cv::Mat> resBGR;
	for (int i = 0; i < 3; i++)
	{
		//Mat threshold_output;
		//cvtColor(bgrOutput[i], threshold_output, COLOR_BGR2GRAY);


		threshold(bgrOutput[i], bgrOutput[i], 0, 255, THRESH_BINARY | THRESH_OTSU);
		resize(bgrOutput[i], bgrOutput[i], Size2i(256, 256));
		//cv::imshow("thres"+std::to_string(i), bgrOutput[i]);
		//waitKey();
		resBGR.push_back(bgrOutput[i]);
	}
	resBGR.push_back(cropImg);

	return resBGR;
	//return img;
}
bool Detector::IsCode(Mat& srcImg)
{
	vector<vector<Point>> qrPoint;
	FindAnchors(srcImg, qrPoint);
	std::cout << "qrPoint.size()==" << qrPoint.size() << std::endl;

	return (qrPoint.size() == 4);
}

bool Detector::IsCode(Mat& srcImg,int newOrder)
{
	vector<vector<Point>> qrPoint;
	FindAnchors(srcImg, qrPoint);
	std::cout << "qrPoint.size()==" << qrPoint.size() << std::endl;
	if (qrPoint.size() <4 && qrPoint.size() >0)
	{
		Mat src_gray;
		cvtColor(srcImg, src_gray, COLOR_BGR2GRAY);
		Mat threshold_output;
		threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
		imwrite("u" + to_string(newOrder) + "th.jpg", threshold_output);
		Point2f center[4];

		for (int i = 0; i < qrPoint.size(); i++)
		{

			CenterPoint(qrPoint[i], center[i]);
			circle(srcImg, center[i], 10, Scalar(0,0,255));
		}
		imshow("",srcImg);
		imwrite("undectcted" + to_string(newOrder) + ".jpg", srcImg);
		waitKey(0);

	}
	//else imwrite("uu" + to_string(newOrder) + ".jpg", srcImg);
	return (qrPoint.size() == 4);
}

bool Detector::GetCropCode(Mat& srcImg, std::vector<cv::Mat>& dst)
{
	vector<vector<Point>> qrPoint;
	FindAnchors(srcImg, qrPoint);

	if (qrPoint.size() == 4)
	{
		dst = resize4AnchorCode(srcImg, qrPoint);
		return true;
	}

	else
	{
/*		Mat src_gray;
		cvtColor(srcImg, src_gray, COLOR_BGR2GRAY);
		Mat threshold_output;
		threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
		//imwrite("u" + to_string(newOrder) + "th.jpg", threshold_output);
		Point2f center[4];

		for (int i = 0; i < qrPoint.size(); i++)
		{

			CenterPoint(qrPoint[i], center[i]);
			circle(srcImg, center[i], 10, Scalar(0, 0, 255));
		}
		imshow("", srcImg);
		imwrite("undectcted" + to_string(newOrder) + ".jpg", srcImg);
		//waitKey(0);*/

		std::cout << "qrPoint.size()==" << qrPoint.size()<<std::endl;
		return false;
	}

}
