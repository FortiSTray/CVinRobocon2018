/*
	.__(.)< ~QUAK
	 \___)
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <process.h>
#include "Windows.h"
#include "opencv2\opencv.hpp"
#include "CameraApi.h"
#include "locator.h"
#include "recognizer.h"

using namespace std;
using namespace cv;

UINT            m_threadID;		//ͼ��ץȡ�̵߳�ID
HANDLE          m_hDispThread;	//ͼ��ץȡ�̵߳ľ��
BOOL            m_bExit = FALSE;		//����֪ͨͼ��ץȡ�߳̽���
CameraHandle    m_hCamera;		//��������������ͬʱʹ��ʱ���������������	
BYTE*           m_pFrameBuffer; //���ڽ�ԭʼͼ������ת��ΪRGB�Ļ�����
tSdkFrameHead   m_sFrInfo;		//���ڱ��浱ǰͼ��֡��֡ͷ��Ϣ

int	            m_iDispFrameNum;	//���ڼ�¼��ǰ�Ѿ���ʾ��ͼ��֡������
float           m_fDispFps;			//��ʾ֡��
float           m_fCapFps;			//����֡��
tSdkFrameStatistic  m_sFrameCount;
tSdkFrameStatistic  m_sFrameLast;
int					m_iTimeLast;
char		    g_CameraName[64];

Locator CrtLocator;

//��ʱ����
int start = 0;

/*ͼ��ץȡ�̣߳���������SDK�ӿں�����ȡͼ��*/
UINT WINAPI uiDisplayThread(LPVOID lpParam)
{
	tSdkFrameHead 	sFrameInfo;
	CameraHandle    hCamera = (CameraHandle)lpParam;
	BYTE*			pbyBuffer;
	CameraSdkStatus status;

	while (!m_bExit)
	{

		if (CameraGetImageBuffer(hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			//����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
			//�ҹ�˾�󲿷��ͺŵ������ԭʼ���ݶ���Bayer��ʽ��
			status = CameraImageProcess(hCamera, pbyBuffer, m_pFrameBuffer, &sFrameInfo);//����ģʽ

																						 //�ֱ��ʸı��ˣ���ˢ�±���
			if (m_sFrInfo.iWidth != sFrameInfo.iWidth || m_sFrInfo.iHeight != sFrameInfo.iHeight)
			{
				m_sFrInfo.iWidth = sFrameInfo.iWidth;
				m_sFrInfo.iHeight = sFrameInfo.iHeight;
				//ͼ���С�ı䣬֪ͨ�ػ�
			}

			if (status == CAMERA_STATUS_SUCCESS)
			{
				////����SDK��װ�õ���ʾ�ӿ�����ʾͼ��,��Ҳ���Խ�m_pFrameBuffer�е�RGB����ͨ��������ʽ��ʾ������directX,OpengGL,�ȷ�ʽ��
				//CameraImageOverlay(hCamera, m_pFrameBuffer, &sFrameInfo);

				/*
				==========================================================================================================
				                                               Main Task
				==========================================================================================================
				*/
				Mat srcImage(Size(sFrameInfo.iWidth, sFrameInfo.iHeight), CV_8UC3, m_pFrameBuffer);
				imshow("original", srcImage);
					
				CrtLocator.locate(srcImage);

				int time = clock() - start;
				cout << time << endl;
				start = clock();
				/*
				==========================================================================================================
				                                                   End
				==========================================================================================================
				*/

				m_iDispFrameNum++;
			}

			//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
			//�����ٴε���CameraGetImageBufferʱ�����򽫱�����֪�������߳��е���CameraReleaseImageBuffer���ͷ���buffer
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

	//ö���豸������豸�б�
	iCameraNums = 10;//����CameraEnumerateDeviceǰ��������iCameraNums = 10����ʾ���ֻ��ȡ10���豸�������Ҫö�ٸ�����豸�������sCameraList����Ĵ�С��iCameraNums��ֵ

	if (CameraEnumerateDevice(sCameraList, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
	{
		printf("No camera was found!\n");
		return FALSE;
	}

	//��ʾ���У�����ֻ����������һ���������ˣ�ֻ��ʼ����һ�������(-1,-1)��ʾ�����ϴ��˳�ǰ����Ĳ���������ǵ�һ��ʹ�ø�����������Ĭ�ϲ���.
	//In this demo ,we just init the first camera.
	if ((status = CameraInit(&sCameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the camera! Error code is %d\n", status);
		printf(msg);
		printf(CameraGetErrorString(status));
		return FALSE;
	}


	//Get properties description for this camera.
	CameraGetCapability(m_hCamera, &sCameraInfo);//"��ø��������������"

	m_pFrameBuffer = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);

	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera, CAMERA_MEDIA_TYPE_MONO8);
	}

	strcpy_s(g_CameraName, sCameraList[0].acFriendlyName);

	CameraCreateSettingPage(m_hCamera, NULL,
		g_CameraName, NULL, NULL, 0);//"֪ͨSDK�ڲ��������������ҳ��";

#ifdef USE_CALLBACK_GRAB_IMAGE //���Ҫʹ�ûص�������ʽ������USE_CALLBACK_GRAB_IMAGE�����
	//Set the callback for image capture
	CameraSetCallbackFunction(m_hCamera, GrabImageCallback, 0, NULL);//"����ͼ��ץȡ�Ļص�����";
#else
	m_hDispThread = (HANDLE)_beginthreadex(NULL, 0, &uiDisplayThread, (PVOID)m_hCamera, 0, &m_threadID);
#endif

	CameraPlay(m_hCamera);

	CameraShowSettingPage(m_hCamera, TRUE);//TRUE��ʾ������ý��档FALSE�����ء�

	while (m_bExit != TRUE)
	{
		waitKey(10);
	}

	CameraUnInit(m_hCamera);

	CameraAlignFree(m_pFrameBuffer);

	destroyWindow(g_CameraName);

#ifdef USE_CALLBACK_GRAB_IMAGE
	if (g_iplImage)
	{
		cvReleaseImageHeader(&g_iplImage);
	}
#endif
	return 0;
}
