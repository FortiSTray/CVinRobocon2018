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
		//resize(src, src, Size(src.cols / 2.5, src.rows / 2.5));
		//videoCapture >> src;

		if (src.empty())
		{
			printf("could not load image\n");
			return -1;
		}
		Mat locateResult = src.clone();
		Mat out;

		namedWindow("input image", CV_WINDOW_AUTOSIZE);
		imshow("input image", src);

		cvtColor(src, src, COLOR_BGR2GRAY);
		medianBlur(src, src, 3);
		//threshold(src, src, 0, 255, THRESH_BINARY | THRESH_OTSU);
		adaptiveThreshold(src, src, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 55, 0);
		imshow("threshold.jpg", src);

		//detect rectangle now
		vector<vector<Point>> contours;
		vector<Point> locateContour;
		vector<Vec4i> hireachy;
		Moments monents;
		findContours(src.clone(), contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());

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
				//drawContours(locateResult, contours, static_cast<int>(t), Scalar(255, 0, 0), 2, 8);
				//imshow("result", locateResult);
				//waitKey(0);

				int k = t;
				int c = 0;


				while (hireachy[k][2] != -1)
				{
					k = hireachy[k][2];
					c = c + 1;
					if (c >= 2)
					{
						drawContours(locateResult, contours, static_cast<int>(t), Scalar(255, 0, 0), 2, 8);
						locateContour.insert(locateContour.end(), contours[t].begin(), contours[t].end());
					}
				}
			}
		}
		//resize(locateResult, locateResult, Size(700, 700));
		//resize(locateResult, locateResult, Size(525, 700));
		if (!locateContour.empty())
		{
			RotatedRect rectReal = minAreaRect(locateContour);
			Mat regularROI = GetRegularROI(src, rectReal);
			imshow("result", locateResult);
			imshow("ROI", regularROI);
		}

	//	if (waitKey(5) == 27) break;

	//}
    waitKey(0);
    return 0;
}
