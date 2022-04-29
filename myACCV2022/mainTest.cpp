#include "waterFilling.h"
#include <ctime>
#include <fstream> 

using namespace cv;
using namespace std;

//单张图片处理

/*
此处代码的主要目的在于对Y通道进行简单的二值化（由imagewatch可以看到Y通道的文字部分像素基本都在180以下，背景部分大约平均像素值为220）
由此代码的运行结果可以看出：
1、经过注水算法对图像进行矫正之后，阴影区域泛黄与CbCr通道有关，
	因为将二值化的Y通道merge回原图之后阴影区域依旧泛黄，下一步应该重点放在对CbCr通道的色彩矫正上面来
2、可以确定，阴影外沿光晕的Y通道数值在180-220之间，阴影外沿的光晕和内部的泛黄是两个不同的原因，需要分别处理
*/

int main(int argc, const char** argv) {
	const char* cTxt = "imgsNew.txt";					//imgs.txt存放需要处理的图像名

	FILE*  fp;
	if ((fp = fopen(cTxt, "r")) == NULL) {			//打开imgs.txt，找不到文件则返回错误信息
		printf("Open Falied!");
		return -1;
	}

	char cInputImagePath[150];
	char cSavePath[150];
	char strLine[100];
	string strImgName;//每一张待处理的图像的文件名
	while (!feof(fp))//feof(fp)检查文件是否结束，如果结束则返回非零值，否则返回0
	{
	
		fgets(strLine, 100, fp);	//从fp所指向的文件中读取一行字符(最多99个)送入字符串数组strLine中（也就是图像的文件名）
		strImgName = strLine;
		
		if (strImgName[strlen(strImgName.c_str()) - 1] == '\n') {
			strImgName[strlen(strImgName.c_str()) - 1] = 0;
		}
		//将"imgs\\"与图像的文件名（eg. 100.jpg）拼接成一个相对地址路径的字符串存放到cInputImagePath中
		sprintf(cInputImagePath, "imgsNew\\%s", strImgName.c_str());

		cout << strImgName << endl;//控制台输出此时处理的图片文件名
		Mat img = imread(cInputImagePath);//读取图像，将图像数据保存到矩阵img中
		if (!img.empty()) {
			Mat img_YCbCr = RGB2YCbCr(img); //将原图img由bgr格式转换为YCbCr格式
			Mat chan[3];
			split(img_YCbCr, chan);//拆分存储每个通道的数据;
			Mat Y = chan[2];
			//----------------------------------------------------------------------------------------------------------------------------------------------
			//统计Y通道非文字部分的平均值
			int num = 0;
			double YSum = 0.0;
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					//(Y.at<uchar>(i, j) > 180) ? (Y.at<uchar>(i, j) = 220) : (Y.at<uchar>(i, j) = Y.at<uchar>(i, j));
					if ( Y.at<uchar>(i, j) > 180){
						YSum += Y.at<uchar>(i, j) ;
						num += 1;
					}
				}
			}
			//	对Y通道非文字部分的平均值重新赋值
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					//(Y.at<uchar>(i, j) > 180) ? (Y.at<uchar>(i, j) = 220) : (Y.at<uchar>(i, j) = Y.at<uchar>(i, j));
					if (Y.at<uchar>(i, j) > 180) {
						Y.at<uchar>(i, j) = YSum / num; 
					}
				}
			}
			//----------------------------------------------------------------------------------------------------------------------------------------------

			//Mat enhanceY;
			//Mat kernel = getStructuringElement(0, Size(3, 3));
			//dilate(Y, enhanceY, kernel, Point(0,0),3);
			//morphologyEx(Y, enhanceY, MORPH_CLOSE,kernel, Point(0,0),3);
		
			/*namedWindow("Y channel", WINDOW_NORMAL);
			imshow("Y channel", Y);
			waitKey(0);*/
			vector<Mat> channels_(3);//定义了三个Mat类型的向量
			channels_[0] = chan[0];
			channels_[1] = chan[1];
			channels_[2] = Y; 
			Mat img_YCbCr_rec;
			merge(channels_, img_YCbCr_rec);

			Mat rec_img = YCbCr2RGB(img_YCbCr_rec);//将YCbCr格式的图像转回rgb格式
			//将处理后的图像写回到cSavePath路径下
			sprintf(cSavePath, "resultsNew\\%s", strImgName.c_str());
			imwrite(cSavePath, rec_img);
		}
		else {
			continue;
		}
	}

	fclose(fp);
	//system("pause");
	return 0;
}
