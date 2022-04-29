#include "waterFilling.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

//Convert Image from RGB color space to YCbCr color Space
Mat RGB2YCbCr(Mat img) {

	int width = img.cols;
	int height = img.rows;
	float dummy;
	Mat img_copy(height, width, CV_8UC3, Scalar(0, 0, 0)); //����һ����imgͬ�ߴ�ͬ��ʽ�Ŀհ�ͼƬ

	for (int i = 0; i < height; i++) {
		for (int k = 0; k < width; k++) { // indexing every channels of a 2-D array form of images.
			/* Y = 0.299*imImg_R + 0.587*imImg_G + 0.114*imImg_B;
			Cb = 0.564*(imImg_B - Y);
			Cr = 0.713*(imImg_R - Y);*/

			//assigning Y values
			dummy = round(16 + 65.481 / 255.0*float(img.at<Vec3b>(i, k)[2]) + 128.553 / 255.0*float(img.at<Vec3b>(i, k)[1]) + 24.966 / 255.0*float(img.at<Vec3b>(i, k)[0]));
			if (dummy < 0) {
				dummy = 0;
			}//��ֹɫ�ʿռ�Խ��
			if (dummy > 255) {
				dummy = 255;
			}
			img_copy.at<Vec3b>(i, k)[2] = dummy;  //����ĵ���λ��Yֵ


			//assinging Cb
			dummy = round(128 - 37.797 / 255.0 * float(img.at<Vec3b>(i, k)[2]) - 74.203 / 255.0 * float(img.at<Vec3b>(i, k)[1]) + 112.0 / 255.0 * float(img.at<Vec3b>(i, k)[0]));
			if (dummy < 0) {
				dummy = 0;
			}
			if (dummy > 255) {
				dummy = 255;
			}
			img_copy.at<Vec3b>(i, k)[1] = dummy; //����ĵڶ�λ��Cbֵ

			//assigning Cr
			dummy = round(128 + 112.0 / 255 * float(img.at<Vec3b>(i, k)[2]) - 93.786 / 255.0 * float(img.at<Vec3b>(i, k)[1]) - 18.214 / 255.0 * float(img.at<Vec3b>(i, k)[0]));
			if (dummy < 0) {
				dummy = 0;
			}
			if (dummy > 255) {
				dummy = 255;
			}
			img_copy.at<Vec3b>(i, k)[0] = dummy;//����ĵ�һλ��Crֵ



		}
	}

	return img_copy;

}


//Convert Image from YCbCr color space to RGB color Space
Mat YCbCr2RGB(Mat img) {

	int width = img.cols;
	int height = img.rows;
	float dummy;

	Mat img_copy(height, width, CV_8UC3, Scalar(0, 0, 0));

	for (int i = 0; i < height; i++) {
		for (int k = 0; k < width; k++) { // indexing every channels of a 2-D array form of images.
			/* Y = 0.299*imImg_R + 0.587*imImg_G + 0.114*imImg_B;
			Cb = 0.564*(imImg_B - Y);
			Cr = 0.713*(imImg_R - Y);*/

			//assigning R values
			dummy = round(1.1644 * float(img.at<Vec3b>(i, k)[2] - 16) + 1.5960 * (float(img.at<Vec3b>(i, k)[0]) - 128));
			if (dummy < 0) {
				dummy = 0;
			}
			if (dummy > 255) {
				dummy = 255;
			}
			img_copy.at<Vec3b>(i, k)[2] = dummy;


			//assinging G
			dummy = round(1.1644 * (float(img.at<Vec3b>(i, k)[2]) - 16) - 0.3918 * (float(img.at<Vec3b>(i, k)[1]) - 128) - 0.8130*(float(img.at<Vec3b>(i, k)[0]) - 128));
			if (dummy < 0) {
				dummy = 0;
			}
			if (dummy > 255) {
				dummy = 255;
			}
			img_copy.at<Vec3b>(i, k)[1] = dummy;

			//assigning B
			dummy = round(1.1644 * (float(img.at<Vec3b>(i, k)[2]) - 16) + 2.0172* (float(img.at<Vec3b>(i, k)[1]) - 128));
			if (dummy < 0) {
				dummy = 0;
			}
			if (dummy > 255) {
				dummy = 255;
			}
			img_copy.at<Vec3b>(i, k)[0] = dummy;
		}
	}

	return img_copy;
}