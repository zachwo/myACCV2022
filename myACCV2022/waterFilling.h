#include <imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
using namespace cv;
using namespace std;

Mat RGB2YCbCr(Mat img);
Mat YCbCr2RGB(Mat img);
Mat FloodAndEffuse(Mat input);
Mat IncrementalFilling(Mat input);
Mat GetBinary(Mat input);
Mat GetBinaryTxt(Mat input);
Mat Lambertian(Mat GAfterFloodAndIncre, Mat originalY);
Mat RemoveTinyShadow(Mat input);
//Mat RectifyCbCr(Mat input, Mat  binaryImg_background);
Mat RectifyCbCr(Mat input, Mat  binaryImg_background, Mat  binaryImg_txt, Mat Cr, Mat Cb);

//过滤文字区域对应的像素的CbCr(UV)数值，黑色的YUV值是(0, 128, 128)
Mat FilterTextValue(Mat input, Mat  binaryImg_txt);
Mat GetG_Y(Mat G_Y0, Mat G_Y1, Mat  binaryImg_txt);
float GetBrightnessDifference(Mat Y_after, Mat chan_before, Mat  binaryImg_background, Mat  binaryImg_txt);