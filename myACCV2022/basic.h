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
*return：void
*date: 2019.01.15   wangbingshu
********************************************/
void ThresholdIntegral(Mat &inputMat, double thre, Mat &outputMat);



/*******************************************
*Function: Calculate the Mean Square Error between two images
*Input:src, the original image
*      predict, an predicted image obtained by methods
*Output:
*return：MSE value, double
*date: 2019.01.15   wangbingshu
********************************************/
double CalulateOneImgMSE(Mat& src, Mat& predict);



//struct 二值化标定区域结构体
typedef struct _Feather
{
	int label;              // 连通域的label值
	int area;               // 连通域的面积
	Rect boundingbox;       // 连通域的外接矩形框
} Feather;


/*******************************************
*Function:对二值化(只有0和255)的连通区域标定
*Input:  src,待检测连通域的二值化图像,单通道
*Output: dst,标记后的图像，单通道
*        featherList: 连通域特征的清单
*return：连通域数量。
********************************************/
int LabelRegions(Mat & src, Mat & dst, vector<Feather> & featherList);





void ImageEnhancement(Mat& img, Mat& result);





#endif // !#ifndef _BASIC_H
 
