#pragma once
#ifndef _CODE_DETECT

#include "decode.h"

#endif // !_CODE_DETECT

class Detector
{
	//cv::Mat srcImage;
	//cv::Mat cropImage;

	// An approximate range
	bool IsQRRate(float rate);

	// Judge the color ratio of X-direction
	bool IsQRColorRateX(cv::Mat& img, int flag);

	// Judge the color ratio of Y-direction
	bool IsQRColorRateY(cv::Mat& img, int flag);

	bool IsQRColorRate(cv::Mat& img, int flag);

	// Get the center vector of four anchors
	int CenterPoint(std::vector<cv::Point>& anchor, cv::Point2f& p);

	// Judge the top-left one from three large anchors
	int JudgeTopLeft(cv::Point2f* center);

public:

	bool GetCropCode(cv::Mat& srcImg, std::vector<cv::Mat>& dst);

	int FindAnchors(cv::Mat& srcImg, std::vector<std::vector<cv::Point>>& qrPoint, int mode);

	bool IsAnchor(std::vector<cv::Point>& contour, cv::Mat& img, int i);

	cv::Mat CropRect(cv::Mat& img, cv::RotatedRect& rotatedRect);

	cv::Mat CropCode(cv::Mat& img, cv::RotatedRect& rotatedRect, cv::Point2f center[4], int topLeftOrder, int buttonRightOrder);

	std::vector<cv::Mat> ResizeCode(cv::Mat& img, std::vector<std::vector<cv::Point>>& qrPoint, int mode);

	bool IsCode(cv::Mat& srcImg);



};
