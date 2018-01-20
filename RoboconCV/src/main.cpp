/*
	.__(.)< ~QUAK
	 \___)          <- No-BUG Duck
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <process.h>
#include "Windows.h"
#include "opencv2\opencv.hpp"
#include "CameraApi.h"
#include "locator.h"
#include "decoder.h"
#include "computeTime.h"

using namespace std;
using namespace cv;

#define CAMERA_NEAR 0
#define CAMERA_FAR  1

UINT            m_threadID;		//图像抓取线程的ID
HANDLE          m_hDispThread;	//图像抓取线程的句柄
BOOL            m_bExit = FALSE;		//用来通知图像抓取线程结束
CameraHandle    m_hCameraNear;		//近焦相机句柄
CameraHandle    m_hCameraFar;		//远焦相机句柄
BYTE*           m_pFrameBuffer; //用于将原始图像数据转换为RGB的缓冲区
tSdkFrameHead   m_sFrInfo;		//用于保存当前图像帧的帧头信息

int	            m_iDispFrameNum;	//用于记录当前已经显示的图像帧的数量
float           m_fDispFps;			//显示帧率
float           m_fCapFps;			//捕获帧率
tSdkFrameStatistic  m_sFrameCount;
tSdkFrameStatistic  m_sFrameLast;
int					m_iTimeLast;
char		    g_CameraName[64];

Locator CrtLocator;
Decoder CrtDecoder;
ComputeTime FPSOutput;

//char fileName[32];
//int fileSerial = 0;

/*图像抓取线程，主动调用SDK接口函数获取图像*/
UINT WINAPI uiDisplayThread(LPVOID lpParam)
{
	tSdkFrameHead 	sFrameInfo;
	CameraHandle    hCamera = (CameraHandle)lpParam;
	BYTE*			pbyBuffer;
	CameraSdkStatus status;
	bool cameraSelect = CAMERA_NEAR;
	bool getImageBufferFlag = false;

	//临时变量
	int start = 0;

	while (!m_bExit)
	{
		if (cameraSelect == CAMERA_NEAR)
		{
			if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				//我公司大部分型号的相机，原始数据都是Bayer格式的
				status = CameraImageProcess(hCamera, pbyBuffer, m_pFrameBuffer, &sFrameInfo);//连续模式

				getImageBufferFlag = true;
			}
			else
			{
				getImageBufferFlag = false;
			}
		}
		else
		{
			if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				//我公司大部分型号的相机，原始数据都是Bayer格式的
				status = CameraImageProcess(hCamera, pbyBuffer, m_pFrameBuffer, &sFrameInfo);//连续模式

				getImageBufferFlag = true;
			}
			else
			{
				getImageBufferFlag = false;
			}
		}

		if (getImageBufferFlag == true)
		{
			//分辨率改变了，则刷新背景
			if (m_sFrInfo.iWidth != sFrameInfo.iWidth || m_sFrInfo.iHeight != sFrameInfo.iHeight)
			{
				m_sFrInfo.iWidth = sFrameInfo.iWidth;
				m_sFrInfo.iHeight = sFrameInfo.iHeight;
				//图像大小改变，通知重绘
			}

			if (status == CAMERA_STATUS_SUCCESS)
			{
				////调用SDK封装好的显示接口来显示图像,您也可以将m_pFrameBuffer中的RGB数据通过其他方式显示，比如directX,OpengGL,等方式。
				//CameraImageOverlay(hCamera, m_pFrameBuffer, &sFrameInfo);

				/*
				==========================================================================================================
				                                               Main Task
				==========================================================================================================
				*/

				Mat srcImage(Size(sFrameInfo.iWidth, sFrameInfo.iHeight), CV_8UC3, m_pFrameBuffer);
				imshow("Original", srcImage);

				//Signal dstSignal;
				//dstSignal = CrtLocator.locate(srcImage);
				//imshow("Signal", dstSignal.image);

				///*if (waitKey(2) == ' ')
				//{
				//	sprintf(fileName, "./data/data%d.jpg", fileSerial++);
				//	if (fileSerial >= 99) { fileSerial--; }
				//	imwrite(fileName, dstSignal.image);
				//}*/

				//int message = -1;
				//if (dstSignal.lable == true)
				//{
				//	message = CrtDecoder.decode(dstSignal.image);
				//	//cout << message << endl;
				//}
				//else
				//{
				//	//cout << message << endl;
				//}

				cout << FPSOutput.End() << endl;
				FPSOutput.Begin();
				
				/*
				==========================================================================================================
				                                                   End
				==========================================================================================================
				*/

				m_iDispFrameNum++;
			}

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，知道其他线程中调用CameraReleaseImageBuffer来释放了buffer
			CameraReleaseImageBuffer(hCamera, pbyBuffer);

			memcpy(&m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
		}
		
		if (waitKey(10) == 27)
		{
			m_bExit = TRUE;
			break;
		}
	}

	_endthreadex(0);
	return 0;
}

int main(int argc, char* argv[])
{
	tSdkCameraDevInfo sCameraList[10];
	INT iCameraNums;
	CameraSdkStatus status;
	tSdkCameraCapbility sCameraInfo;

	tSdkFrameHead 	sFrameInfo;
	BYTE*			pbyBuffer;

	bool cameraSelect = CAMERA_NEAR;
	bool getImageBufferFlag = false;
	int keyStatus = 0;

	//临时变量
	int start = 0;

	//枚举设备，获得设备列表
	iCameraNums = 10;//调用CameraEnumerateDevice前，先设置iCameraNums = 10，表示最多只读取10个设备，如果需要枚举更多的设备，请更改sCameraList数组的大小和iCameraNums的值

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("Where's my camera? (ﾟДﾟ≡ﾟдﾟ)!?\n");
		return FALSE;
	}
	
	//用相机名初始化近焦和远焦相机，并给两个相机句柄赋值
	if ((status = CameraInitEx2("CameraNear", &m_hCameraNear)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init CameraNear! Error code is %d\n", status);
		printf(msg);
		printf(CameraGetErrorString(status));
		return FALSE;
	}
	if ((status = CameraInitEx2("CameraFar", &m_hCameraFar)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the CameraFar! Error code is %d\n", status);
		printf(msg);
		printf(CameraGetErrorString(status));
		return FALSE;
	}
	
	//获得相机的特性描述，两个相机型号及硬件设置完全相同，所以只需要获取一台相机的信息
	CameraGetCapability(m_hCameraNear, &sCameraInfo);

	m_pFrameBuffer = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCameraNear, CAMERA_MEDIA_TYPE_RGB8);
		CameraSetIspOutFormat(m_hCameraFar , CAMERA_MEDIA_TYPE_RGB8);
	}

	//通知SDK内部建该相机的属性页面
	CameraCreateSettingPage(m_hCameraNear, NULL, "CameraNear", NULL, NULL, 0);
	CameraCreateSettingPage(m_hCameraFar , NULL, "CameraFar" , NULL, NULL, 0);

	CameraPlay(m_hCameraNear);
	CameraPlay(m_hCameraFar );

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCameraNear, TRUE);
	CameraShowSettingPage(m_hCameraFar , TRUE);

	while (!m_bExit)
	{
		if (cameraSelect == CAMERA_NEAR)
		{
			if (CameraGetImageBuffer(m_hCameraNear, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				//我公司大部分型号的相机，原始数据都是Bayer格式的
				status = CameraImageProcess(m_hCameraNear, pbyBuffer, m_pFrameBuffer, &sFrameInfo);//连续模式

				getImageBufferFlag = true;
			}
			else
			{
				getImageBufferFlag = false;
			}
		}
		else
		{
			if (CameraGetImageBuffer(m_hCameraFar, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				//我公司大部分型号的相机，原始数据都是Bayer格式的
				status = CameraImageProcess(m_hCameraFar, pbyBuffer, m_pFrameBuffer, &sFrameInfo);//连续模式

				getImageBufferFlag = true;
			}
			else
			{
				getImageBufferFlag = false;
			}
		}

		if (getImageBufferFlag == true)
		{
			//分辨率改变了，则刷新背景
			if (m_sFrInfo.iWidth != sFrameInfo.iWidth || m_sFrInfo.iHeight != sFrameInfo.iHeight)
			{
				m_sFrInfo.iWidth = sFrameInfo.iWidth;
				m_sFrInfo.iHeight = sFrameInfo.iHeight;
				//图像大小改变，通知重绘
			}

			if (status == CAMERA_STATUS_SUCCESS)
			{
				////调用SDK封装好的显示接口来显示图像,您也可以将m_pFrameBuffer中的RGB数据通过其他方式显示，比如directX,OpengGL,等方式。
				//CameraImageOverlay(hCamera, m_pFrameBuffer, &sFrameInfo);

				/*
				==========================================================================================================
				Main Task
				==========================================================================================================
				*/

				Mat srcImage(Size(sFrameInfo.iWidth, sFrameInfo.iHeight), CV_8UC3, m_pFrameBuffer);
				imshow("Original", srcImage);

				//Signal dstSignal;
				//dstSignal = CrtLocator.locate(srcImage);
				//imshow("Signal", dstSignal.image);

				///*if (waitKey(2) == ' ')
				//{
				//	sprintf(fileName, "./data/data%d.jpg", fileSerial++);
				//	if (fileSerial >= 99) { fileSerial--; }
				//	imwrite(fileName, dstSignal.image);
				//}*/

				//int message = -1;
				//if (dstSignal.lable == true)
				//{
				//	message = CrtDecoder.decode(dstSignal.image);
				//	//cout << message << endl;
				//}
				//else
				//{
				//	//cout << message << endl;
				//}

				cout << FPSOutput.End() << endl;
				FPSOutput.Begin();

				/*
				==========================================================================================================
				End
				==========================================================================================================
				*/

				m_iDispFrameNum++;
			}

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，知道其他线程中调用CameraReleaseImageBuffer来释放了buffer
			if (cameraSelect == CAMERA_NEAR)
			{
				CameraReleaseImageBuffer(m_hCameraNear, pbyBuffer);
			}
			else
			{
				CameraReleaseImageBuffer(m_hCameraFar, pbyBuffer);
			}

			memcpy(&m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
		}

		keyStatus = waitKey(10);
		if (keyStatus == 27)
		{
			m_bExit = TRUE;
			break;
		}
		else if (keyStatus == ' ')
		{
			cameraSelect = !cameraSelect;
		}
	}

	CameraUnInit(m_hCameraNear);
	CameraUnInit(m_hCameraFar );

	CameraAlignFree(m_pFrameBuffer);

#ifdef USE_CALLBACK_GRAB_IMAGE
	if (g_iplImage)
	{
		cvReleaseImageHeader(&g_iplImage);
	}
#endif
	return 0;
}
