#pragma once
#ifndef _BASIC_H
#define _BASIC_H
#include"common.h"
#include <stack> 

/*******************************************
*Function: Binarization using integral method
which is from https://github.com/phryniszak/AdaptiveIntegralThresholding
*Input: inputMat, gray-scale image
*       thre, the threshold to classify the fg and bg
*Output: outputMat, gray-scale, (0 bg, 255 fg)
*
*return��void
*date: 2019.01.15   wangbingshu
********************************************/
void ThresholdIntegral(Mat &inputMat, double thre, Mat &outputMat);



/*******************************************
*Function: Calculate the Mean Square Error between two images
*Input:src, the original image
*      predict, an predicted image obtained by methods
*Output:
*return��MSE value, double
*date: 2019.01.15   wangbingshu
********************************************/
double CalulateOneImgMSE(Mat& src, Mat& predict);



//struct ��ֵ���궨����ṹ��
typedef struct _Feather
{
	int label;              // ��ͨ���labelֵ
	int area;               // ��ͨ������
	Rect boundingbox;       // ��ͨ�����Ӿ��ο�
} Feather;


/*******************************************
*Function:�Զ�ֵ��(ֻ��0��255)����ͨ����궨
*Input:  src,�������ͨ��Ķ�ֵ��ͼ��,��ͨ��
*Output: dst,��Ǻ��ͼ�񣬵�ͨ��
*        featherList: ��ͨ���������嵥
*return����ͨ��������
********************************************/
int LabelRegions(Mat & src, Mat & dst, vector<Feather> & featherList);





void ImageEnhancement(Mat& img, Mat& result);





#endif // !#ifndef _BASIC_H
 
