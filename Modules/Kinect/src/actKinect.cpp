#include "actKinect.h"

using namespace std;
using namespace cv;

ActKinect::ActKinect() : pKinectSensor(NULL), pDepthReader(NULL), pColorReader(NULL), 
						 pDepthFrame(NULL), pColorFrame(NULL), pCoordinateMapper(NULL)
{
	//open kinect sensor
	GetDefaultKinectSensor(&pKinectSensor);
	pKinectSensor->Open();

	//get coordinate mapper
	pKinectSensor->get_CoordinateMapper(&pCoordinateMapper);

	bgModel = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();

	orbitTail = { 0, 0, 0 };
}
ActKinect::~ActKinect()
{
	//release coordinate mapper
	SafeRelease(pCoordinateMapper);

	//release reader
	SafeRelease(pColorReader);
	SafeRelease(pDepthReader);

	//close kinect sensor
	if (pKinectSensor)
	{
		pKinectSensor->Close();
	}
	SafeRelease(pKinectSensor);
}

void ActKinect::initDepthSensor()
{
	//get frame source
	IDepthFrameSource* pDepthSource = nullptr;
	pKinectSensor->get_DepthFrameSource(&pDepthSource);

	//get height and width
	IFrameDescription *depthDescription = nullptr;
	pDepthSource->get_FrameDescription(&depthDescription);
	depthDescription->get_Height(&depthHeight);
	depthDescription->get_Width(&depthWidth);
	depthDescription->Release();

	//open reader
	pDepthSource->OpenReader(&pDepthReader);

	//initialize depth image
	depthTemp  = cv::Mat::zeros(depthHeight, depthWidth, CV_16UC1);
	depthImage = cv::Mat::zeros(depthHeight, depthWidth, CV_8UC1 );
}

void ActKinect::initColorSensor()
{
	//get frame source
	IColorFrameSource* pColorSource = nullptr;
	pKinectSensor->get_ColorFrameSource(&pColorSource);

	//get height and width
	IFrameDescription *colorDescription = nullptr;
	pColorSource->get_FrameDescription(&colorDescription);
	colorDescription->get_Height(&colorHeight);
	colorDescription->get_Width(&colorWidth);
	colorDescription->Release();

	//open reader
	pColorSource->OpenReader(&pColorReader);

	//initialize color image
	colorTemp = cv::Mat::zeros(colorHeight, colorWidth, CV_8UC4);
	colorImage = cv::Mat::zeros(colorHeight, colorWidth, CV_8UC3);
}

void ActKinect::updateDepth()
{
	if (pDepthReader->AcquireLatestFrame(&pDepthFrame) == S_OK)
	{
		pDepthFrame->CopyFrameDataToArray(depthHeight * depthWidth, (UINT16*)depthTemp.data);
		depthTemp.convertTo(depthImage, CV_8UC1, 255.0 / 4500);
		cv::imshow("Depth", depthImage);
		pDepthFrame->Release();
	}
}

void ActKinect::updateColor()
{
	if (pColorReader->AcquireLatestFrame(&pColorFrame) == S_OK)
	{
		pColorFrame->CopyConvertedFrameDataToArray(colorHeight*colorWidth * 4, colorTemp.data, ColorImageFormat_Bgra);
		cv::cvtColor(colorTemp, colorImage, CV_BGRA2BGR);
		//cv::imshow("Color", colorImage);
		pColorFrame->Release();
	}
}

void ActKinect::coordinateMapping()
{
	//map coordinate from depth image to color image
	ColorSpacePoint* pColorCoordinates = new ColorSpacePoint[depthHeight * depthWidth];
	pCoordinateMapper->MapDepthFrameToColorSpace(depthHeight * depthWidth, (UINT16*)depthTemp.data,
		depthHeight * depthWidth, pColorCoordinates);

	//draw depthToColor image
	std::vector<BYTE> depthBuffer(depthWidth * depthHeight * 4);
	Concurrency::parallel_for(0, depthHeight, [&](const int depthY)
	{
		const unsigned int depthOffset = depthY * depthWidth;
		for (int depthX = 0; depthX < depthWidth; depthX++)
		{
			unsigned int depthIndex = depthOffset + depthX;

			const int colorX = static_cast<int>(pColorCoordinates[depthIndex].X + 0.5f);
			const int colorY = static_cast<int>(pColorCoordinates[depthIndex].Y + 0.5f);

			if ((colorX >= 0) && (colorX < colorWidth) && (colorY >= 0) && (colorY < colorHeight))
			{
				const unsigned int colorIndex = (colorY * colorWidth + colorX) * 4;
				depthIndex = depthIndex * 4;

				depthBuffer[depthIndex + 0] = colorTemp.data[colorIndex + 0];
				depthBuffer[depthIndex + 1] = colorTemp.data[colorIndex + 1];
				depthBuffer[depthIndex + 2] = colorTemp.data[colorIndex + 2];
				depthBuffer[depthIndex + 3] = colorTemp.data[colorIndex + 3];
			}
		}

	});

	depthToColor = cv::Mat(depthHeight, depthWidth, CV_8UC4, &depthBuffer[0]).clone();
	cv::imshow("DtoC", depthToColor);

	delete[] pColorCoordinates;
}

void ActKinect::detectBall()
{
	getForeground();

	cvtColor(fgImage, fgImage, COLOR_BGR2GRAY);

	cv::Mat equaImage(fgImage.rows, fgImage.cols, CV_8UC1);
	equalizeHist(fgImage, equaImage);
	//imshow("equa", equaImage);

	GaussianBlur(fgImage, fgImage, Size(5, 5), 2, 2);
	medianBlur(equaImage, equaImage, 5);
	//imshow("bulr", equaImage);

	cv::Mat cannyImage(fgImage.rows, fgImage.cols, CV_8UC1);
	Canny(equaImage, cannyImage, 125, 259);
	//imshow("canny", cannyImage);


	cv::HoughCircles(fgImage, circles, CV_HOUGH_GRADIENT, 2.5, 3000, 250, 40, 5, 40);

	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		circle(depthImage, center, radius, 200, 3, 8, 0);
	}
	imshow("ballPosition", depthImage);
}

void ActKinect::getForeground()
{
	cvtColor(depthToColor, depthToColor, COLOR_BGRA2BGR);
	Mat realImage = depthToColor.clone();

	if (fgImage.empty())
	{
		fgImage.create(realImage.size(), realImage.type());
	}

	//update the model
	cvtColor(realImage, realImage, CV_BGRA2BGR);

	bgModel->apply(realImage, fgMask, record ? -1 : 0);

	fgImage = Scalar::all(0);

	realImage.copyTo(fgImage, fgMask);

	Mat bgImage;
	bgModel->getBackgroundImage(bgImage);


	//imshow("image", realImage);
	//imshow("foreground mask", fgMask);
	//imshow("foreground image", fgImage);
	//if (!bgImage.empty())
	//{
	//	imshow("mean background image", bgImage);
	//}
}

void ActKinect::ballTrack()
{
	if (circles.size() != 0)
	{
		int depthCounter = 0;
		int pointNumCounter = 0;
		int loseCounter = 0;
		float depthValue = 0.0f;

		int xBasic = cvRound(circles[0][0]);
		int yBasic = cvRound(circles[0][1]);

		if (depthImage.ptr(yBasic)[xBasic] != 0.0f)
		{
			depthValue += depthImage.ptr(yBasic)[xBasic];
			depthCounter++;
		}
		pointNumCounter++;
		depthImage.ptr(yBasic)[xBasic] = 200;
		stkBall.push_back({ xBasic, yBasic});

		while (!stkBall.empty())
		{
			auto pix = stkBall.back();
			stkBall.pop_back();

			auto row0 = pix.y - 1, row1 = pix.y, row2 = pix.y + 1;
			auto col0 = pix.x - 1, col1 = pix.x, col2 = pix.x + 1;

#define _PASS_(x, y) do \
					{ \
						if (depthImage.ptr(y)[x] != 0.0f) \
						{ \
							depthValue += depthImage.ptr(y)[x]; \
							depthCounter++; \
						} \
						pointNumCounter++; \
						depthImage.ptr(y)[x] = 200; \
						stkBall.push_back({ x, y }); \
					}while (0)

			//row0
			if (row0 >= 0 && col0 >= 0 && depthImage.ptr(row0)[col0] != 200)
				_PASS_(col0, row0);
			if (row0 >= 0 && depthImage.ptr(row0)[col1] != 200)
				_PASS_(col1, row0);
			if (row0 >= 0 && col2 < depthImage.cols && depthImage.ptr(row0)[col2] != 200)
				_PASS_(col2, row0);
			//row1
			if (col0 >= 0 && depthImage.ptr(row1)[col0] != 200)
				_PASS_(col0, row1);
			if (col2 < depthImage.cols && depthImage.ptr(row1)[col2] != 200)
				_PASS_(col2, row1);
			//row2
			if (row2 < depthImage.rows && col0 >= 0 && depthImage.ptr(row2)[col0] != 200)
				_PASS_(col0, row2);
			if (row2 < depthImage.rows && depthImage.ptr(row2)[col1] != 200)
				_PASS_(col1, row2);
			if (row2 < depthImage.rows && col2 < depthImage.cols && depthImage.ptr(row2)[col2] != 200)
				_PASS_(col2, row2);

#undef _PASS_

			if (pointNumCounter >= 50) { break; }
		}
		
		if (depthCounter != 0)
		{
			depthValue /= (float)depthCounter;

			currentBall[0] = xBasic;
			currentBall[1] = yBasic;
			currentBall[2] = cvRound(depthValue / 255.0f * 4500.0f);

			if (abs(currentBall[2] - orbitTail[2]) < 1000)
			{
				std::cout << currentBall[0] << " " << currentBall[1] << " " << currentBall[2] << endl;

				orbitTail[0] = currentBall[0];
				orbitTail[1] = currentBall[1];
				orbitTail[2] = currentBall[2];

				loseCounter = 0;
			}
			else
			{
				if (loseCounter < 4) { loseCounter++; }
			}

			if (loseCounter == 4) { orbitTail = { 0, 0, 0 }; }
		}
	}
}