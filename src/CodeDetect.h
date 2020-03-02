#pragma once
#ifndef _CODE_DETECT

#include "decode.h"

#endif // !_CODE_DETECT


namespace CodeDetect
{
	//An approximate range
	bool IsQRRate(float rate);

	//Judge the color ratio of X-direction
	bool IsQRColorRateX(cv::Mat& image, int flag);

	//Judge the color ratio of Y-direction
	bool IsQRColorRateY(cv::Mat& image, int flag);

	bool IsQRColorRate(cv::Mat& image, int flag);

	cv::Mat CropImage(cv::Mat& img, cv::RotatedRect& rotatedRect);

	int JudgeTopLeft(cv::Point2f* center);

	cv::Mat Crop4AnchorCode(cv::Mat& img, cv::RotatedRect& rotatedRect, cv::Point2f center[4], int topLeftOrder, int buttonRightOrder);

	cv::Mat Crop3AnchorCode(cv::Mat& img, cv::RotatedRect& rotatedRect, cv::Point2f topLeft);

	bool IsAnchor(std::vector<cv::Point>& contour, cv::Mat& img, int i);

	int FindAnchors(cv::Mat& srcImg, std::vector<std::vector<cv::Point>>& qrPoint);

	int CenterPoint(std::vector<cv::Point>& anchor, cv::Point2f& p);

	cv::Mat resize4AnchorCode(cv::Mat& img, std::vector<std::vector<cv::Point>>& qrPoint);

	bool IsCode(cv::Mat& srcImg);

	bool GetCropCode(cv::Mat& srcImg, cv::Mat& dst);


};