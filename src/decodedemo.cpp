#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <cmath>
#define BLOCK_ROWS 32
#define BLOCK_BITS 1024
#define OUTPUT_FPS 5

using namespace std;
using namespace cv;


namespace QRCodeDetect
{
	void RotateArbitrarilyAngle(Mat src, Mat& dst, float angle)
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

		imshow("dst", dst);
		waitKey();

		return true;
	}
};

namespace CodeDetect
{
	//An approximate range
	bool IsQRRate(float rate)
	{
		return rate > 0.3 && rate < 1.9;
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

	int JudgeTopLeft(Point2f *center)
	{
		double distance01 = (center[0].x - center[1].x) * (center[0].x - center[1].x) 
			+ (center[0].y - center[1].y) * (center[0].y - center[1].y);
		double distance02 = (center[0].x - center[2].x) * (center[0].x - center[2].x)
			+ (center[0].y - center[2].y) * (center[0].y - center[2].y);
		double distance12 = (center[1].x - center[2].x) * (center[1].x - center[2].x)
			+ (center[1].y - center[2].y) * (center[1].y - center[2].y);
		if (distance01>=distance02&& distance01 >= distance12)
		{
			return 2;
		}
		else if (distance02>=distance01&&distance02>=distance12)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	Mat Crop4AnchorCode(Mat& img, RotatedRect& rotatedRect, Point2f center[4], int topLeftOrder, int buttonRightOrder)
	{
		Mat warpPerspective_mat(3, 3, CV_32FC1);
		Mat warp256mat(3, 3, CV_32FC1);
		Mat warpRectMat(3, 3, CV_32FC1);

		Mat warpPerspective_dst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
		Mat warpRectdst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
		Mat warp256= Mat::zeros(256, 256, img.type());
		Point2f srcPoint[4], topLeftCenter= center[topLeftOrder],buttonRightCenter = center[buttonRightOrder];
		rotatedRect.points(srcPoint);
		//Mat img2 = img.clone();
		//circle(img2, center[0], 10, Scalar(255, 0, 0));
		//circle(img2, center[1], 10, Scalar(0, 255, 0));
		//circle(img2, center[2], 10, Scalar(0, 0, 255));
		//circle(img2, center[3], 10, Scalar(255, 0, 255));
		//resize(img2, img2, Size(img2.cols / 2, img2.rows / 2));
		//imshow("circle", img2);
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

		Point2f topRightRect=srcPoint[(topLeftRectOrder+1)%4];
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
		cout<< buttonLeftOrder << buttonRightOrder<<topLeftOrder << topRightOrder;
		vector<Point2f> srcTri;
		srcTri.push_back(center[topLeftOrder]);
		srcTri.push_back(center[topRightOrder]);
		srcTri.push_back(center[buttonRightOrder]);
		srcTri.push_back(center[buttonLeftOrder]);

		vector<Point2f> dstTri;
		dstTri.push_back(Point2f(rotatedRect.size.width * 13.5 / 256, rotatedRect.size.height * 13.5 / 256));
		dstTri.push_back(Point2f(rotatedRect.size.width * 241.5 / 256, rotatedRect.size.height * 13.5 / 256));
		dstTri.push_back(Point2f(rotatedRect.size.width * 248.5/256, rotatedRect.size.height*248.5/256));
		dstTri.push_back(Point2f(rotatedRect.size.width * 13.5 / 256, rotatedRect.size.height* 241.5 / 256));

		vector<Point2f> dst256Tri;
		dst256Tri.push_back(Point2f(13.5, 13.5));
		dst256Tri.push_back(Point2f(241.5, 13.5));
		dst256Tri.push_back(Point2f(248.5, 248.5));
		dst256Tri.push_back(Point2f(13.5, 241.5));

		vector<Point2f> srcTriRect;
		srcTriRect.push_back(srcPoint[topLeftRectOrder]);
		srcTriRect.push_back(srcPoint[(topLeftRectOrder + 1) % 4]);
		srcTriRect.push_back(srcPoint[(topLeftRectOrder + 2) % 4]);
		srcTriRect.push_back(srcPoint[(topLeftRectOrder + 3) % 4]);

		vector<Point2f> dstTriRect;
		dstTriRect.push_back(Point2f(0, 0));
		dstTriRect.push_back(Point2f(rotatedRect.size.width - 1, 0));
		dstTriRect.push_back(Point2f(rotatedRect.size.width - 1, rotatedRect.size.height - 1));
		dstTriRect.push_back(Point2f(0, rotatedRect.size.height - 1));


		warpPerspective_mat = getPerspectiveTransform(srcTri, dstTri);
		warpPerspective(img, warpPerspective_dst, warpPerspective_mat, warpPerspective_dst.size());
		resize(warpPerspective_dst, warpPerspective_dst, Size(256, 256));

		warpRectMat = getPerspectiveTransform(srcTriRect, dstTriRect);
		warpPerspective(img, warpRectdst, warpRectMat, warpRectdst.size());
		resize(warpRectdst, warpRectdst, Size(256, 256));

		warp256mat = getPerspectiveTransform(srcTri, dst256Tri);
		warpPerspective(img, warp256, warp256mat, warp256.size());
		//imshow("warp", warpPerspective_dst);
		//imshow("rect", warpRectdst);
		//imshow("warp256", warp256);
		//waitKey(0);

		return warp256;
	}

	//This function suites 3-anchor code.
	Mat Crop3AnchorCode(Mat& img, RotatedRect& rotatedRect,Point2f topLeft)
	{
		Mat warpPerspective_mat(3, 3, CV_32FC1);

		Mat warpPerspective_dst = Mat::zeros(rotatedRect.size.width, rotatedRect.size.height, img.type());
		Point2f srcPoint[4];
		rotatedRect.points(srcPoint);
		Mat img2 = img.clone();
		circle(img2, srcPoint[0], 10, Scalar(255,0,0));
		circle(img2, srcPoint[1], 10, Scalar(0,255,0));
		circle(img2, srcPoint[2], 10, Scalar(0,0,255));
		circle(img2, srcPoint[3], 10, Scalar(255,0,255));
		circle(img2, topLeft, 20, Scalar(255));
		resize(img2, img2, Size(img2.cols /2, img2.rows / 2));
		imshow("circle", img2);
		waitKey(0);
		int topLeftOrder = 0, buttonRightOrder = 0; 
		double minDistance = ((double)srcPoint[0].x - topLeft.x) * ((double)srcPoint[0].x - topLeft.x)
			+ ((double)srcPoint[0].y - topLeft.y) * ((double)srcPoint[0].y - topLeft.y),
			maxDistance=minDistance;
		for (int i =1; i < 4; i++)
		{
			double distance = ((double)srcPoint[i].x - topLeft.x) * ((double)srcPoint[i].x - topLeft.x)
				+ ((double)srcPoint[i].y - topLeft.y) * ((double)srcPoint[i].y - topLeft.y);
			if (distance > maxDistance)
			{
				maxDistance = distance;
				buttonRightOrder = i;
			}
			else if (distance < minDistance)
			{
				minDistance = distance;
				topLeftOrder = i;
			}
		}
		if (topLeftOrder - buttonRightOrder != 2 && topLeftOrder - buttonRightOrder != -2)
		{
			cout << "Order Judgement Error in CropCode()!" << endl;
			return img2;
		}
		
		vector<Point2f> srcTri;
		srcTri.push_back(srcPoint[topLeftOrder]);
		srcTri.push_back(srcPoint[(topLeftOrder + 1) % 4]);
		srcTri.push_back(srcPoint[(topLeftOrder + 2) % 4]);
		srcTri.push_back(srcPoint[(topLeftOrder + 3) % 4]);

		vector<Point2f> dstTri;
		dstTri.push_back(Point2f(0, 0));
		dstTri.push_back(Point2f(rotatedRect.size.width - 1, 0));
		dstTri.push_back(Point2f(rotatedRect.size.width - 1, rotatedRect.size.height - 1));
		dstTri.push_back(Point2f(0, rotatedRect.size.height - 1));

		warpPerspective_mat = getPerspectiveTransform(srcTri, dstTri);
		warpPerspective(img, warpPerspective_dst, warpPerspective_mat, warpPerspective_dst.size());
		//imshow("", warpPerspective_dst);
		//waitKey(0);
		
		return warpPerspective_dst;
	}
	
	bool IsAnchor(vector<Point>& contour, Mat& img, int i)
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

	int FindAnchors(Mat& srcImg, vector<vector<Point>>& qrPoint)
	{

		Mat src_gray;
		cvtColor(srcImg, src_gray, COLOR_BGR2GRAY);
		//namedWindow("src_gray");
		//imshow("src_gray", src_gray);

		Mat canny_output;
		Canny(src_gray, canny_output, 0, 255);
		Mat canny_output_copy = canny_output.clone();
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
				bool isQR = IsAnchor(contours[parentIndex], threshold_output_copy, parentIndex);

				//Save three anchors
				if (isQR)
					qrPoint.push_back(contours[parentIndex]);

				ic = 0;
				parentIndex = -1;
			}
		}
		
		return 0;
	}
	
	int CenterPoint(vector<Point>& anchor, Point2f& p)
	{
		double x=0, y=0;
		for (int i = 0; i < anchor.size(); i++)
		{
			x += (double)anchor[i].x;
			y += (double)anchor[i].y;
		}
		p.x = x /(double) anchor.size();
		p.y = y /(double) anchor.size();
		return 0;
	}

	//This function suites 3-anchor code.
	Mat resize3AnchorCode(Mat& img, vector<vector<Point>>& qrPoint)
	{
		vector<Point2f> totalPoints;
		Point2f Center[3];
		Mat img2 = img.clone();
		for (int i = 0; i < qrPoint.size(); i++)
		{
			CenterPoint(qrPoint[i], Center[i]);
			circle(img2, Center[i], 1, Scalar(0, 0, 255));
			for (int j = 0; j < qrPoint[i].size(); j++)
			{
				circle(img2, qrPoint[i][j], 1, Scalar(255));
				totalPoints.push_back(qrPoint[i][j]);
			}
		}

		//imshow("img2", img2);
		//waitKey();
		int topLeftOrder = JudgeTopLeft(Center);
		RotatedRect rect = minAreaRect(totalPoints);
		Mat cropImg = Crop3AnchorCode(img, rect, Center[topLeftOrder]);
		//Decode(img,end);
		Mat threshold_output;
		cvtColor(cropImg, threshold_output, COLOR_BGR2GRAY);
		//imshow("crop", cropImg);
		//waitKey();
		threshold(threshold_output, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
		resize(threshold_output, threshold_output, Size2i(256, 256));
		//imshow("thres", threshold_output);
		//waitKey();
		return threshold_output;
	}

	Mat resizeCode(Mat& img, vector<vector<Point>>& qrPoint)
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
	
		//imshow("img2",img2);
		//waitKey();
		for (int i = 0,j = 0; i < 4; i++)
		{
			if (i == buttonRightOrder) continue;
			center3[j++] = center[i];
		}
  		//int topLeftOrder = JudgeTopLeft(center3);
		int topLeftOrder = JudgeTopLeft(center3);
		if (topLeftOrder >= buttonRightOrder) topLeftOrder++;

		Mat img2 = img.clone();
		circle(img2, center[topLeftOrder], 10, Scalar(255, 0, 0));
		circle(img2, center[buttonRightOrder], 10, Scalar(0, 255, 0));
		//circle(img2, center[2], 10, Scalar(0, 0, 255));
		//circle(img2, center[3], 10, Scalar(255, 0, 255));
		resize(img2, img2, Size(img2.cols / 2, img2.rows / 2));
		//imshow("circle", img2);
		//waitKey(0);


		RotatedRect rect = minAreaRect(totalPoints);
		Mat cropImg = Crop4AnchorCode(img, rect, center,topLeftOrder,buttonRightOrder);
		//Decode(img,end);
		Mat threshold_output;
		cvtColor(cropImg, threshold_output, COLOR_BGR2GRAY);
		//imshow("crop", cropImg);
		//waitKey();
		threshold(threshold_output, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
		resize(threshold_output, threshold_output, Size2i(256, 256));
		//imshow("thres", threshold_output);
		//waitKey();
		return threshold_output;
		//return img;
	}
	bool IsCode(Mat& srcImg)
	{
		vector<vector<Point>> qrPoint;
		FindAnchors(srcImg, qrPoint);
		return (qrPoint.size() == 4);
	}

	bool GetCropCode(Mat& srcImg, Mat& dst)
	{
		vector<vector<Point>> qrPoint;
		FindAnchors(srcImg, qrPoint);
		//for (int i = 0; i < qrPoint.size(); i++)
		//{
		//	for (int j = 0; j < qrPoint[i].size(); j++)
		//	{
		//		circle(srcImg, qrPoint[i][j], 1, Scalar(255));
		//	}
		//}
		//resize(srcImg, srcImg, Size(srcImg.cols / 2, srcImg.rows / 2));
		//imshow("src", srcImg);
		//waitKey();
		if (qrPoint.size() == 4)
		{
			dst = resizeCode(srcImg, qrPoint);
			return true;
		}
		else 
			return false;
	}
};

int Decode(Mat img, bool& end, int order)
{
	FILE* fp = fopen("out.bin", "a+");
	Mat resizeImg;
	if (!CodeDetect::GetCropCode(img, resizeImg))
	{
		cout << "Can't detect " << order << endl;
		imwrite("v0302" + to_string(order) + ".jpg", img);
		return -1;
	}
	cout << "Detect " << order << endl;
	//imshow("",resizeImg);
	//waitKey(0);
	imwrite("v0302oK" + to_string(order) + ".jpg", resizeImg);
	Mat circleImg;
	cvtColor(resizeImg, circleImg, COLOR_GRAY2BGR);
	int total_bit = 0;
	while (total_bit < 1020)
	{
		unsigned char chr = '\0';
		for (int k = 0; k < 8; k++)
		{
			while ((total_bit / 32 < 4 || total_bit / 32 >= 28) && (total_bit % 32 < 4 || total_bit % 32 >= 28))
			{
				total_bit++;
			}

			int val = //(*(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 5) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
				//+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 5) + resizeImg.step[1] * (16 * (total_bit % 16) + 9))
				+*(resizeImg.data + resizeImg.step[0] * (8 * (total_bit / BLOCK_ROWS) + 4) + resizeImg.step[1] * (8 * (total_bit % BLOCK_ROWS) + 4))
				//+ *(resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 5))
				//+* (resizeImg.data + resizeImg.step[0] * (16 * (total_bit / 16) + 9) + resizeImg.step[1] * (16 * (total_bit % 16) + 9)))
				//5;
				;
			circle(circleImg, Point(8 * (total_bit % BLOCK_ROWS) + 4, 8 * (total_bit / BLOCK_ROWS) + 4), 2, Scalar(255, 0, 0), -1);
			
			//int val = *(code.data + code.step[0] * (total_bit / 16) + code.step[1] * (total_bit % 16));
			chr = chr << 1;
			if (val < 128 && (!(total_bit % 2) && !((total_bit / 32) % 2) || (total_bit % 2) && ((total_bit / 32) % 2))
				|| val >= 128 && !(!(total_bit % 2) && !((total_bit / 32) % 2) || (total_bit % 2) && ((total_bit / 32) % 2)))
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
	imshow("circleImg", circleImg);
	waitKey(0);
	cout << total_bit << endl;

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
	do
	{
		vc.read(srcImg);
		if (!srcImg.data) break;
	} while (!CodeDetect::IsCode(srcImg));
	while (true)
	{

		vc.read(srcImg);
		if (!srcImg.data) break;

		//imwrite("op" + to_string(order) + ".jpg",srcImg);

		
		Decode(srcImg, end, order);
		order++;
		if (end) break;

		for (int i = 1; i < 30/OUTPUT_FPS; i++)
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
	//Mat img = imread("op12D.jpg");
	//Mat img = imread("op22.jpg");
	//if (img.empty())
	//{
	//	cout << "Null";
	//	return -1;
	//}
	/*Mat dst;
	//cout << QRCodeDetect::GetCropCode(img, dst);
	//cout << CodeDetect::GetCropCode(img,dst);
	Mat img;
	int ord = 0;
	for (;ord<=4; ord++)
	{
		img= imread(".\\test"+to_string(ord)+".jpg");
		if (img.empty())
		{
			cout << "Null";
			break;
		}
		bool end = false;
		cout << ord << " : " << CodeDetect::GetCropCode(img,dst);

		Decode(img, end,ord);
		cout << endl;

	}
	*/
	cout << endl << "Done" << endl;

	return 0;
}