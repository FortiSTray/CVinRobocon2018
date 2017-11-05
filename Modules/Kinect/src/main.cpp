#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>

#include "opencv2\opencv.hpp"
#include "actKinect.h"

using namespace std;
using namespace cv;

int main()
{
	freopen("ballTrackData.txt", "w", stdout);

	ActKinect actKinect;

	actKinect.initDepthSensor();
	actKinect.initColorSensor();

	while (true)
	{
		actKinect.updateDepth();
		actKinect.updateColor();

		actKinect.coordinateMapping();

		actKinect.detectBall();
		actKinect.ballTrack();

		if (waitKey(5) == VK_ESCAPE)
			break;
	}

	return 0;
}