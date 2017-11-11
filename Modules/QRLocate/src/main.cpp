#include <iostream>
#include <math.h>
#include <time.h>
#include "opencv2/opencv.hpp"
#include "QRLocate.h"

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) 
{
	//VideoCapture videoCapture(0);

	//while (true)
	//{
		Mat src = imread("QRCode.jpg");
		//videoCapture >> src;

		if (src.empty())
		{
			printf("could not load image\n");
			return -1;
		}
		Mat locateResult = src.clone();

		namedWindow("input image", CV_WINDOW_AUTOSIZE);
		imshow("input image", src);

		cvtColor(src, src, COLOR_BGR2GRAY);
		threshold(src, src, 0, 255, THRESH_BINARY | THRESH_OTSU);

		//detect rectangle now
		vector<vector<Point>> contours;
		vector<Vec4i> hireachy;
		Moments monents;
		findContours(src.clone(), contours, hireachy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());

		for (size_t t = 0; t < contours.size(); t++)
		{
			double area = contourArea(contours[t]);
			if (area < 100) continue;
			RotatedRect rect = minAreaRect(contours[t]);

			//根据矩形特征进行几何分析
			float w = rect.size.width;
			float h = rect.size.height;
			float ratio = min(w, h) / max(w, h);

			if (ratio > 0.85 && w < src.cols / 4 && h < src.rows / 4)
			{
				printf("angle : %.2f\n", rect.angle);
				Mat regularROI = GetRegularROI(src, rect);
				if (JudgeCornerByX(regularROI) && JudgeCornerByY(regularROI)) 
				{
					drawContours(locateResult, contours, static_cast<int>(t), Scalar(255, 0, 0), 2, 8);
					//imwrite(format("D:/gloomyfish/outimage/contour_%d.jpg", static_cast<int>(t)), regularROI);

					//imshow("roiImage", regularROI);
					//waitKey(0);
				}
			}
		}
		resize(locateResult, locateResult, Size(700, 700));
		//resize(locateResult, locateResult, Size(525, 700));
		imshow("result", locateResult);

	//	if (waitKey(5) == 27) break;

	//}
    waitKey(0);
    return 0;
}
