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

////单摄像头调试模式
//#define SINGLE_CAMERA_MODE

#define CAMERA_NEAR 0
#define CAMERA_FAR  1

BOOL            m_bExit = FALSE;		//用来通知图像抓取线程结束
CameraHandle    m_hCamera;              //单相机模式时相机句柄
CameraHandle    m_hCameraNear;		    //近焦相机句柄
CameraHandle    m_hCameraFar;		    //远焦相机句柄
BYTE*           m_pFrameBuffer;         //用于将原始图像数据转换为RGB的缓冲区
tSdkFrameHead   m_sFrInfo;		        //用于保存当前图像帧的帧头信息
char		    g_CameraName[64];

//类的实例化
Locator CrtLocator;
Decoder CrtDecoder;
ComputeTime FPSOutput;

//用于Debug图像输出
//char fileName[32];
//int fileSerial = 0;

int main(int argc, char* argv[])
{
#ifdef SINGLE_CAMERA_MODE

	tSdkCameraDevInfo sCameraList[1];
	INT iCameraNums;

#endif

	CameraSdkStatus status;
	tSdkCameraCapbility sCameraInfo;

	tSdkFrameHead 	sFrameInfo;
	BYTE*			pbyBuffer;

	bool cameraSelect = CAMERA_NEAR;
	bool getImageBufferFlag = false;
	int keyStatus = 0;

#ifdef SINGLE_CAMERA_MODE

	//枚举设备，获得设备列表
	iCameraNums = 1;

	if (CameraEnumerateDevice(sCameraList, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		return FALSE;
	}

	if ((status = CameraInit(&sCameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the camera! Error code is %d\n", status);
		printf(msg);
		printf(CameraGetErrorString(status));
		return FALSE;
	}

	//根据安装方式设置源图像镜像操作
	CameraSetMirror(m_hCamera, 0, FALSE);
	CameraSetMirror(m_hCamera, 1, TRUE);

	//获得该相机的特性描述
	CameraGetCapability(m_hCamera, &sCameraInfo);

	m_pFrameBuffer = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera, CAMERA_MEDIA_TYPE_MONO8);
	}

	strcpy_s(g_CameraName, sCameraList[0].acFriendlyName);

	//通知SDK内部建该相机的属性页面
	CameraCreateSettingPage(m_hCamera, NULL, g_CameraName, NULL, NULL, 0);

	//进入工作模式开始采集图像
	CameraPlay(m_hCamera);

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCamera, TRUE);//TRUE显示相机配置界面。FALSE则隐藏。

#else

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("W...Where's my camera? QAQ\n");
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
	
	//根据安装方式设置源图像镜像操作
	CameraSetMirror(m_hCameraNear, 0, FALSE);
	CameraSetMirror(m_hCameraNear, 1, TRUE );
	CameraSetMirror(m_hCameraFar , 0, TRUE );
	CameraSetMirror(m_hCameraFar , 1, FALSE);

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

	//进入工作模式开始采集图像
	CameraPlay(m_hCameraNear);
	CameraPlay(m_hCameraFar );

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCameraNear, TRUE);
	CameraShowSettingPage(m_hCameraFar , TRUE);

#endif

	//主循环
	while (!m_bExit)
	{
#ifdef SINGLE_CAMERA_MODE

		if (CameraGetImageBuffer(m_hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
			status = CameraImageProcess(m_hCamera, pbyBuffer, m_pFrameBuffer, &sFrameInfo);

			getImageBufferFlag = true;
		}
		else
		{
			getImageBufferFlag = false;
		}

#else

		if (cameraSelect == CAMERA_NEAR)
		{
			if (CameraGetImageBuffer(m_hCameraNear, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				status = CameraImageProcess(m_hCameraNear, pbyBuffer, m_pFrameBuffer, &sFrameInfo);

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
				status = CameraImageProcess(m_hCameraFar, pbyBuffer, m_pFrameBuffer, &sFrameInfo);

				getImageBufferFlag = true;
			}
			else
			{
				getImageBufferFlag = false;
			}
		}

#endif

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
				/*
				==========================================================================================================
				                                              Main Task
				==========================================================================================================
				*/

				Mat srcImage(Size(sFrameInfo.iWidth, sFrameInfo.iHeight), CV_8UC3, m_pFrameBuffer);
				imshow("Original", srcImage);

				Signal dstSignal;
				dstSignal = CrtLocator.locate(srcImage);
				imshow("Signal", dstSignal.image);

				/*if (waitKey(2) == ' ')
				{
					sprintf(fileName, "./data/data%d.jpg", fileSerial++);
					if (fileSerial >= 99) { fileSerial--; }
					imwrite(fileName, dstSignal.image);
				}*/

				int message = -1;
				if (dstSignal.lable == true)
				{
					message = CrtDecoder.decode(dstSignal.image);
					//cout << message << endl;
				}
				else
				{
					//cout << message << endl;
				}

				cout << FPSOutput.End() << endl;
				FPSOutput.Begin();

				/*
				==========================================================================================================
				                                                  End
				==========================================================================================================
				*/
			}

#ifdef SINGLE_CAMERA_MODE

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
			CameraReleaseImageBuffer(m_hCamera, pbyBuffer);

#else

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
			if (cameraSelect == CAMERA_NEAR)
			{
				CameraReleaseImageBuffer(m_hCameraNear, pbyBuffer);
			}
			else
			{
				CameraReleaseImageBuffer(m_hCameraFar, pbyBuffer);
			}

#endif

			memcpy(&m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
		}

		//延时及检测按键事件
		keyStatus = waitKey(1);
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

#ifdef SINGLE_CAMERA_MODE

	CameraUnInit(m_hCamera);

#else

	CameraUnInit(m_hCameraNear);
	CameraUnInit(m_hCameraFar );

#endif

	CameraAlignFree(m_pFrameBuffer);

	return 0;
}
