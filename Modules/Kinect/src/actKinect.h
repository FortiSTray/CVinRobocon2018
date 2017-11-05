#pragma once
#ifndef __ACT_KINECT
#define __ACT_KINECT

#include "kinect.h"
#include "opencv2/opencv.hpp"
#include "opencv2/video/background_segm.hpp"
#include <iostream>
#include <time.h>
#include <math.h>
#include <thread>
#include <ppl.h>

using namespace std;
using namespace cv;

//Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface* &pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

class ActKinect
{
public:
	ActKinect();
	~ActKinect();
	
	void initDepthSensor();
	void initColorSensor();

	void updateDepth();
	void updateColor();

	void coordinateMapping();


	void detectBall();

	void getForeground();

	void ballTrack();


private:
	IKinectSensor*     pKinectSensor;
	IDepthFrameReader* pDepthReader;
	IColorFrameReader* pColorReader;
	IDepthFrame*       pDepthFrame;
	IColorFrame*       pColorFrame;

	ICoordinateMapper* pCoordinateMapper;

	cv::Mat depthTemp;
	cv::Mat colorTemp;
	cv::Mat depthImage;
	cv::Mat colorImage;

	cv::Mat depthToColor;

	cv::Ptr<cv::BackgroundSubtractor> bgModel;
	cv::Mat fgImage;
	cv::Mat fgMask;
	int record = 1;

	std::vector<cv::Vec3f> circles;

	std::vector<cv::Point> stkBall;
	int ballCoorX;
	int ballCoorY;
	int ballCoorZ;
	std::vector<cv::Vec3i> ballCoor;
	cv::Vec3i currentBall;
	cv::Vec3i orbitTail;

	int depthHeight = 0, depthWidth = 0;
	int colorHeight = 0, colorWidth = 0;
};

#endif //__ACT_KINECT_V2
