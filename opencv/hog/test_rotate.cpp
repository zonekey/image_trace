#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

int main()
{
	cv::namedWindow("main");
	cv::namedWindow("rotated");

	cv::Mat src = cv::imread("1.jpg");
	cv::Mat dst = cv::Mat::zeros(src.cols, src.rows, src.type());
	cv::imshow("main", src);

	cv::Point center(src.cols / 2, src.rows / 2);
	double angle = -15;
	
	double n = src.cols * fabs(cos(angle * CV_PI / 180.0)) + src.rows * fabs(sin(angle * CV_PI / 180.0));
	double scale = src.cols / n;

	cv::Mat rotate_mat = cv::getRotationMatrix2D(center, angle, scale);
	
	cv::warpAffine(src, dst, rotate_mat, src.size());
	cv::imshow("rotated", dst);

	cv::waitKey(0);

	return 0;
}

