#ifndef QR_LOCATE_H
#define QR_LOCATE_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


Mat GetRegularROI(Mat &img, RotatedRect &rect);

bool JudgeCornerByX(Mat &img);
bool JudgeCornerByY(Mat &img);


#endif //!QR_LOCATE_H