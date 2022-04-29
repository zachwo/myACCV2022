#include "waterFilling.h"
#include <ctime>
#include <fstream> 
#include <math.h>

using namespace cv;
using namespace std;

//用于判断double类型的数值是否为零
#define MIN_VALUE 1e-8 
#define IS_DOUBLE_ZERO(d)  (abs(d) < MIN_VALUE)

//单张图片处理

/*
华星光电的rgb2rgby专利算法，用于提取Y色彩通道

此处代码的主要目的在于对Y通道进行简单的二值化（由imagewatch可以看到Y通道的文字部分像素值再100以下，背景部分大约平均像素值为200）
由此代码的运行结果可以看出，经过注水算法对图像进行矫正之后，阴影区域泛黄与CbCr通道有关，
因为将二值化的Y通道merge回原图之后阴影区域依旧泛黄
下一步应该重点放在对CbCr通道的色彩矫正上面来
*/

int main(int argc, const char** argv) {
	//const char* cTxt = "imgsNew.txt";					//imgs.txt存放需要处理的图像名
	const char* cTxt = "evaluation data\\Test00.txt";
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
		sprintf(cInputImagePath, "evaluation data\\Test00_v2.1\\%s", strImgName.c_str());

		cout << strImgName << endl;//控制台输出此时处理的图片文件名
		Mat img = imread(cInputImagePath);//读取图像，将图像数据保存到矩阵img中
		if (!img.empty()) {
			//convert bgr to bgry
			Mat chan[3];
			split(img, chan);
			Mat Bi, Gi, Ri;
			Bi = chan[0];
			Gi = chan[1];
			Ri = chan[2];
			Bi.convertTo(Bi, CV_64FC1);
			Gi.convertTo(Gi, CV_64FC1);
			Ri.convertTo(Ri, CV_64FC1);
			Mat Bo = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			Mat Go = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			Mat Ro = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			Mat Yo = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			double gamma = 2.2;
			for (int i = 0; i < Bi.rows - 1; i++){
				for (int j = 0; j < Bi.cols - 1; j++){
					if ((Bi.at<double>(i,j) < Gi.at<double>(i, j)) && (Bi.at<double>(i, j) < Ri.at<double>(i, j))){
						//signal is yellow
						if (Ri.at<double>(i,j) > Gi.at<double>(i, j)){
							Bo.at<double>(i, j) = Bi.at<double>(i, j);
							Yo.at<double>(i, j) = Gi.at<double>(i, j);
							Go.at<double>(i, j) = 0.0;
							Ro.at<double>(i, j) =pow( ( pow(Ri.at<double>(i,j), gamma)+ pow(Yo.at<double>(i,j), gamma)), pow(gamma,-1));
						}
						else{
							Bo.at<double>(i, j) = Bi.at<double>(i, j);
							Yo.at<double>(i, j) = Ri.at<double>(i, j);
							Ro.at<double>(i, j) = 0.0;
							Go.at<double>(i, j) = pow((pow(Gi.at<double>(i, j), gamma) + pow(Yo.at<double>(i, j), gamma)), pow(gamma, -1));
						}
					}
					else{
						//signal is not yellow
						Yo.at<double>(i, j) = 0;
						Bo.at<double>(i, j) = Bi.at<double>(i, j);
						Go.at<double>(i, j) = Gi.at<double>(i, j);
						Ro.at<double>(i, j) = Ri.at<double>(i, j);
					}
				}
			}
			//change yellow channel
			double avg = 0.0;
			int  sum = 0;
			for (int i = 0; i < Bi.rows - 1; i++) {
				for (int j = 0; j < Bi.cols; j++) {
					if (Yo.at<double>(i,j) != 0){
						avg += Yo.at<double>(i, j);
						sum += 1;
					}
				}
			}
			for (int i = 0; i < Bi.rows - 1; i++) {
				for (int j = 0; j < Bi.cols; j++) {
					if (Yo.at<double>(i, j) != 0) {
						Yo.at<double>(i, j) = avg / sum;
					}
				}
			}
			//convert bgry to bgr
			Mat B_rectify = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			Mat G_rectify = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			Mat R_rectify = Mat(Bi.size(), CV_64F, Scalar(0, 0, 0));
			for (int i = 0; i < Bi.rows - 1; i++){
				for (int j = 0; j < Bi.cols; j++){
					if (IS_DOUBLE_ZERO(Yo.at<double>(i, j))){
						B_rectify.at<double>(i, j) = Bo.at<double>(i, j);
						G_rectify.at<double>(i, j) = Go.at<double>(i, j);
						R_rectify.at<double>(i, j) = Ro.at<double>(i, j);
					}
					else{
						if (IS_DOUBLE_ZERO(Ro.at<double>(i, j))){
							B_rectify.at<double>(i, j) = Bo.at<double>(i, j);
							R_rectify.at<double>(i, j) = Yo.at<double>(i, j);
							G_rectify.at<double>(i, j) = pow((pow(Go.at<double>(i, j), gamma) + pow(Yo.at<double>(i, j), gamma)), pow(gamma, -1));
						}
						else{
							B_rectify.at<double>(i, j) = Bo.at<double>(i, j);
							G_rectify.at<double>(i, j) = Yo.at<double>(i, j);
							R_rectify.at<double>(i, j) = pow((pow(Ro.at<double>(i, j), gamma) + pow(Yo.at<double>(i, j), gamma)), pow(gamma, -1));
						}
					}
				}
			}
			B_rectify.convertTo(B_rectify, CV_8UC1);
			G_rectify.convertTo(G_rectify, CV_8UC1);
			R_rectify.convertTo(R_rectify, CV_8UC1);
			vector<Mat> channels;
			channels.push_back(B_rectify);
			channels.push_back(G_rectify);
			channels.push_back(R_rectify);
			Mat img_rectify;
			merge(channels, img_rectify);

			/*Bo.convertTo(Bo, CV_8UC1);
			Go.convertTo(Go, CV_8UC1);
			Ro.convertTo(Ro, CV_8UC1);
			Yo.convertTo(Yo, CV_8UC1);
			vector<Mat> channels;
			channels.push_back(Bo);
			channels.push_back(Go);
			channels.push_back(Ro);
			channels.push_back(Yo);
			Mat BGRY;
			merge(channels,BGRY);
			
			imshow("BGRY", BGRY);*/
			//imshow("Y", Yo);
			//waitKey(0);
			
			//将处理后的图像写回到cSavePath路径下
			sprintf(cSavePath, "evaluation data\\Test00_v2.2\\%s", strImgName.c_str());
			imwrite(cSavePath, img_rectify);
		}
		else {
			continue;
		}
	}

	fclose(fp);
	return 0;
}
