/*
	.__(.)< ~QUAK
	 \___)          <- No-BUG Duck
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "Windows.h"
#include "process.h"
#include "opencv2\opencv.hpp"
#include "CameraApi.h"
#include "locator.h"
#include "decoder.h"
#include "SerialPort.h"
#include "computeTime.h"
#include "modeConfig.h"

using namespace std;
using namespace cv;

#define CAMERA_NEAR 0
#define CAMERA_FAR  1

//双摄像头句柄组
typedef struct
{
	CameraHandle    handleNear;		    //近焦相机句柄
	CameraHandle    handleFar;		    //远焦相机句柄

	//重载[]运算符以用序号访问两个句柄
	CameraHandle operator[](uchar i)
	{
		if (i == 0) { return handleNear; }
		else if (i == 1) { return handleFar; }
		else { cout << "Camera handle access denied!" << endl; return -1; }
	}
}CameraHandleGroup;

UINT				m_threadID;				//图像抓取线程的ID
HANDLE				m_hFrameGetThread;	    //图像抓取线程的句柄
BOOL				m_bExit = FALSE;		//用来通知图像抓取线程结束
CameraHandle		m_hCamera;              //单相机模式时相机句柄
CameraHandleGroup   m_hCameraGroup;         //双相机比赛模式相机句柄组
BYTE*			    m_pFrameBuffer;         //用于将原始图像数据转换为RGB的缓冲区
tSdkFrameHead		m_sFrInfo;		        //用于保存当前图像帧的帧头信息
CameraSdkStatus     frameGetStatus = CAMERA_STATUS_FAILED;

//类的实例化
Locator CrtLocator;
Decoder CrtDecoder;
CSerialPort CrtSerialPort;
ComputeTime FPSOutput;

#ifdef IMWRITE_DEBUG_IMAGE

char fileName[32];
int fileSerial = 0;

#endif //IMWRITE_DEBUG_IMAGE

#ifdef SINGLE_CAMERA_DEBUG

UINT WINAPI frameGetThread(LPVOID lpParam)
{
	CameraHandle    hCamera = (CameraHandle)lpParam;
	BYTE*			pbyBuffer;
	tSdkFrameHead	sFrameInfo;

	while (!m_bExit)
	{
		if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			setFrameBufferLock(true);
			{
				//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
				frameGetStatus = CameraImageProcess(hCamera, pbyBuffer, m_pFrameBuffer, &sFrameInfo);

				//复制帧头信息
				memcpy(&m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
			}
			setFrameBufferLock(false);

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
			CameraReleaseImageBuffer(m_hCamera, pbyBuffer);
		}
	}

	_endthreadex(0);
	return 0;
}

#else

UINT WINAPI frameGetThread(LPVOID lpParam)
{
	CameraHandleGroup*    hCameraGroup = (CameraHandleGroup*)lpParam;
	BYTE*			      pbyBuffer;
	tSdkFrameHead		  sFrameInfo;

	int taskStatus = INIT_DONE;
	setTaskStatus(INIT_DONE);
	int timeSleepMs = 0;
	while (!m_bExit)
	{
		taskStatus = getTaskStatus();
		switch (taskStatus)
		{
		case INIT_DONE:
			timeSleepMs = 50;
			cout << "Waiting for GayQiao's trigger signal. Sleep for " << timeSleepMs << "ms" << endl;
			Sleep(timeSleepMs);
			break;

		case SUSPEND_BOTH:
			timeSleepMs = 5;
			cout << "Suspending. Sleep for " << timeSleepMs << "ms" << endl;
			Sleep(timeSleepMs);
			break;

		case OPEN_NEAR:
			if (CameraGetImageBuffer(hCameraGroup->handleNear, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				setFrameBufferLock(true);
				{
					//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
					frameGetStatus = CameraImageProcess(hCameraGroup->handleNear, pbyBuffer, m_pFrameBuffer, &sFrameInfo);

					//复制帧头信息
					memcpy(&m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
				}
				setFrameBufferLock(false);

				//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
				//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
				CameraReleaseImageBuffer(hCameraGroup->handleNear, pbyBuffer);
			}

			break;

		case OPEN_FAR:
			if (CameraGetImageBuffer(hCameraGroup->handleFar, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
			{
				setFrameBufferLock(true);
				{
					//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
					frameGetStatus = CameraImageProcess(hCameraGroup->handleFar, pbyBuffer, m_pFrameBuffer, &sFrameInfo);

					//复制帧头信息
					memcpy(&m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
				}
				setFrameBufferLock(false);

				//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
				//否则再次调用CameraGetImageBuffer时，程序将被挂起，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
				CameraReleaseImageBuffer(hCameraGroup->handleFar, pbyBuffer);
			}

			break;

		default:
			break;
		}
	}

	_endthreadex(0);
	return 0;
}

#endif //SINGLE_CAMERA_DEBUG

int main(int argc, char* argv[])
{
#ifdef SINGLE_CAMERA_DEBUG

	tSdkCameraDevInfo sCameraList[1];
	INT iCameraNums;
	char g_CameraName[64];

#endif //SINGLE_CAMERA_DEBUG

	CameraSdkStatus cameraInitStatus;
	tSdkCameraCapbility sCameraInfo;

	int keyStatus = 0;

#ifndef DISABLE_SERIAL_PORT

	//初始化串口并打开监听线程
	int serialPortNumber = 7;
	if (!CrtSerialPort.InitPort(serialPortNumber, CBR_256000, 'N', 8, 1))
	{
		std::cout << "Serial port " << serialPortNumber << " init failed." << std::endl;
	}
	else
	{
		std::cout << "Serial port " << serialPortNumber << " init succeed." << std::endl;

		if (!CrtSerialPort.OpenListenThread())
		{
			std::cout << "Open serial port listening thread failed." << std::endl;
		}
		else
		{
			std::cout << "Open serial port listening thread succeed." << std::endl;
		}
	}

#endif //DISABLE_SERIAL_PORT

#ifdef SINGLE_CAMERA_DEBUG

	//枚举设备，获得设备列表
	iCameraNums = 1;

	if (CameraEnumerateDevice(sCameraList, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		return FALSE;
	}

	if ((cameraInitStatus = CameraInit(&sCameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the camera! Error code is %d\n", cameraInitStatus);
		printf(msg);
		printf(CameraGetErrorString(cameraInitStatus));
		return FALSE;
	}

	//根据安装方式设置源图像镜像操作
	CameraSetMirror(m_hCamera, 0, FALSE);
	CameraSetMirror(m_hCamera, 1, TRUE);

	//设置对比度
	CameraSetContrast(m_hCamera, 100);

	//获得该相机的特性描述
	CameraGetCapability(m_hCamera, &sCameraInfo);

	m_pFrameBuffer = (BYTE*)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax * sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera, CAMERA_MEDIA_TYPE_MONO8);
	}

	strcpy_s(g_CameraName, sCameraList[0].acFriendlyName);

	//通知SDK内部建该相机的属性页面
	CameraCreateSettingPage(m_hCamera, NULL, g_CameraName, NULL, NULL, 0);

	//开启摄像头数据获取线程
	m_hFrameGetThread = (HANDLE)_beginthreadex(NULL, 0, &frameGetThread, (PVOID)m_hCamera, 0, &m_threadID);

	//进入工作模式开始采集图像
	CameraPlay(m_hCamera);

#ifdef IMSHOW_DEBUG_IMAGE

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCamera, TRUE);//TRUE显示相机配置界面。FALSE则隐藏。

#else

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCamera, FALSE);//TRUE显示相机配置界面。FALSE则隐藏。

#endif //IMSHOW_DEBUG_IMAGE

#else

	if (CameraEnumerateDeviceEx() == 0)
	{
		printf("W...Where's my camera? QAQ\n");
		return FALSE;
	}
	
	//用相机名初始化近焦和远焦相机，并给两个相机句柄赋值
	if ((cameraInitStatus = CameraInitEx2("CameraNear", &m_hCameraGroup.handleNear)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init CameraNear! Error code is %d\n", cameraInitStatus);
		printf(msg);
		printf(CameraGetErrorString(cameraInitStatus));
		return FALSE;
	}
	if ((cameraInitStatus = CameraInitEx2("CameraFar", &m_hCameraGroup.handleFar)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the CameraFar! Error code is %d\n", cameraInitStatus);
		printf(msg);
		printf(CameraGetErrorString(cameraInitStatus));
		return FALSE;
	}
	
	//根据安装方式设置源图像镜像操作
	CameraSetMirror(m_hCameraGroup.handleNear, 0, FALSE);
	CameraSetMirror(m_hCameraGroup.handleNear, 1, TRUE );
	CameraSetMirror(m_hCameraGroup.handleFar , 0, TRUE );
	CameraSetMirror(m_hCameraGroup.handleFar , 1, FALSE);

	//设置对比度
	CameraSetContrast(m_hCameraGroup.handleNear, 100);
	CameraSetContrast(m_hCameraGroup.handleFar, 100);

	//获得相机的特性描述，两个相机型号及硬件设置完全相同，所以只需要获取一台相机的信息
	CameraGetCapability(m_hCameraGroup.handleNear, &sCameraInfo);

	m_pFrameBuffer = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCameraGroup.handleNear, CAMERA_MEDIA_TYPE_RGB8);
		CameraSetIspOutFormat(m_hCameraGroup.handleFar , CAMERA_MEDIA_TYPE_RGB8);
	}

	//通知SDK内部建该相机的属性页面
	CameraCreateSettingPage(m_hCameraGroup.handleNear, NULL, "CameraNear", NULL, NULL, 0);
	CameraCreateSettingPage(m_hCameraGroup.handleFar , NULL, "CameraFar" , NULL, NULL, 0);

	//开启摄像头数据获取线程
	m_hFrameGetThread = (HANDLE)_beginthreadex(NULL, 0, &frameGetThread, (PVOID)&m_hCameraGroup, 0, &m_threadID);

	//进入工作模式开始采集图像
	CameraPlay(m_hCameraGroup.handleNear);
	CameraPlay(m_hCameraGroup.handleFar );

#ifdef IMSHOW_DEBUG_IMAGE

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCameraGroup.handleNear, TRUE);
	CameraShowSettingPage(m_hCameraGroup.handleFar , TRUE);

#else

	//TRUE显示相机配置界面。FALSE则隐藏。
	CameraShowSettingPage(m_hCameraGroup.handleNear, FALSE);
	CameraShowSettingPage(m_hCameraGroup.handleFar, FALSE);

#endif //IMSHOW_DEBUG_IMAGE

#endif //SINGLE_CAMERA_DEBUG


	while (!m_bExit)
	{
		if (frameGetStatus == CAMERA_STATUS_SUCCESS)
		{
			/*
			==========================================================================================================
															Main Task
			==========================================================================================================
			*/

			while (getFrameBufferLock() == true);
			Mat srcImage(Size(m_sFrInfo.iWidth, m_sFrInfo.iHeight), CV_8UC3, m_pFrameBuffer);
#ifdef IMSHOW_DEBUG_IMAGE
			imshow("Original", srcImage);
#endif //IMSHOW_DEBUG_IMAGE

			Signal dstSignal;
			dstSignal = CrtLocator.locate(srcImage);
#ifdef IMSHOW_DEBUG_IMAGE
			imshow("Signal", dstSignal.image);
#endif //IMSHOW_DEBUG_IMAGE

#ifdef IMWRITE_DEBUG_IMAGE

			if (waitKey(1) == ' ')
			{
				sprintf(fileName, "./data/data%d.jpg", fileSerial++);
				if (fileSerial >= 99) { fileSerial--; }
				imwrite(fileName, dstSignal.image);
			}

#endif //IMWRITE_DEBUG_IMAGE

			//输出得到的信息
			int signalData = -1;
			if (dstSignal.lable == true)
			{
				signalData = CrtDecoder.decode(dstSignal.image);
			}
			else {}
			cout << signalData << endl;

#ifndef DISABLE_SERIAL_PORT

			uchar* pData = new uchar;
			*pData = static_cast<uchar>(signalData);
			CrtSerialPort.WriteData(pData, 1);
			delete pData;

#endif //DISABLE_SERIAL_PORT

			//输出帧率
			cout << FPSOutput.End() << endl;
			FPSOutput.Begin();

			/*
			==========================================================================================================
														    End Main Task
			==========================================================================================================
			*/
		}

		//延时及检测按键事件
		keyStatus = waitKey(1);
		if (keyStatus == 27)
		{
			m_bExit = TRUE;
			break;
		}
	}


#ifdef SINGLE_CAMERA_DEBUG

	CameraUnInit(m_hCamera);

#else

	CameraUnInit(m_hCameraGroup.handleNear);
	CameraUnInit(m_hCameraGroup.handleFar );

#endif //SINGLE_CAMERA_DEBUG

	CameraAlignFree(m_pFrameBuffer);

	return 0;
}
