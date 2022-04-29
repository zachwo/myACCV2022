#include"common.h"
#include"basic.h"
#include"HardShadowRemoval.h"


#include "waterFilling.h"
#include <ctime>
#include <fstream> 

using namespace cv;
using namespace std;

//单张图片处理

int main(int argc, const char** argv) {
	const char* cTxt = "evaluation data\\Test00.txt";					//Test00.txt存放需要处理的图像名
	//const char* cTxt = "evaluation data\\Natural.txt";
	//const char* cTxt = "evaluation data\\Adobe.txt";
	//const char* cTxt = "evaluation data\\HS.txt";

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
		/*
			fgets(char* str, int size, FILE* stream)用于从文件中读取字符串，每次只读取一行
				fgets每次最多只能读取size-1个字符串，此时第size个字符为NULL
				当遇到换行符或EOF时，即使当前位置在n-1之前也读出结束
				若函数返回成功，则返回字符串数组str的首地址
		*/
		fgets(strLine, 100, fp);	//从fp所指向的文件中读取一行字符(最多99个)送入字符串数组strLine中（也就是图像的文件名）
		strImgName = strLine;
		/*c_str()函数返回一个const修饰的指向字符串strImgName的指针，内容与strImgName相同
		如果成功读取到了一行数据，并且不是最后一行（即以'\n'结尾），则将最后一个换行字符改为0（ascii对应为NULL）
		如果读取的到的是最后一行数据，遇到EOF后会在字符串结尾自动加NULL*/
		if (strImgName[strlen(strImgName.c_str()) - 1] == '\n') {
			strImgName[strlen(strImgName.c_str()) - 1] = 0;
		}
		//将"imgs\\"与图像的文件名（eg. 100.jpg）拼接成一个相对地址路径的字符串存放到cInputImagePath中
		sprintf(cInputImagePath, "evaluation data\\Test00_Original\\%s.jpg", strImgName.c_str());
		//sprintf(cInputImagePath, "evaluation data\\Natural_Original\\%s.jpg", strImgName.c_str());
		//sprintf(cInputImagePath, "evaluation data\\Adobe_Original\\%s.jpg", strImgName.c_str());
		//sprintf(cInputImagePath, "evaluation data\\HS_Original\\%s.jpg", strImgName.c_str());

		cout << strImgName << endl;//控制台输出此时处理的图片文件名
		Mat img = imread(cInputImagePath);//读取图像，将图像数据保存到矩阵img中
		if (!img.empty()) {
			clock_t start;//clock_t是一个长整型数
			double duration;
			start = clock();

			Mat img_YCbCr = RGB2YCbCr(img); //将原图img由bgr格式转换为YCbCr格式
			Mat chan[3];
			split(img_YCbCr, chan);//拆分存储每个通道的数据

			//----------------------------------------------------------------------------------------------------------------------
			//我获取到的背景层
			Mat G_Y0 = FloodAndEffuse(chan[2]);//对Y通道使用洪泛法得到G
			G_Y0 = IncrementalFilling(G_Y0);//对G使用增量注水法

			//使用老师sensor论文中的方法获取背景层
			Mat bg(img.size(), CV_8UC3, Scalar(0, 0, 0));
			GetRefBg(img, bg);   //基于Water_Filling获得图像参考背景图
			Mat bg_YCbCr = RGB2YCbCr(bg);
			Mat bg_chan[3];
			split(bg_YCbCr, bg_chan);
			Mat G_Y1;
			bg_chan[2].copyTo(G_Y1);

			Mat  binaryImg_background = GetBinary(G_Y0);	//对背景层进行二值化（CV_8UC1格式）
			G_Y0 = Lambertian(G_Y0, chan[2]);//使用朗伯曲面模型获得光度矫正图像
			G_Y0 = RemoveTinyShadow(G_Y0);	//进一步去除细小阴影区域
			Mat  binaryImg_txt0 = GetBinary(G_Y0);

			G_Y1 = Lambertian(G_Y1, chan[2]);//使用朗伯曲面模型获得光度矫正图像
			G_Y1 = RemoveTinyShadow(G_Y1);	//进一步去除细小阴影区域

			Mat G_Y = GetG_Y(G_Y0, G_Y1, binaryImg_txt0);
			Mat  binaryImg_txt = GetBinary(G_Y);	//对文字进行二值化（CV_8UC1格式）

			//统计Y通道运算后与运算前非阴影非文字区域的平均亮度差值
			float brightness_difference = GetBrightnessDifference(G_Y, chan[2], binaryImg_background, binaryImg_txt0);
			cout << "brightness_difference = " << brightness_difference << endl;

			//运算后的Y通道减去亮度差值，使最后的图像亮度尽可能与运算前非阴影非文字区域的亮度一致
			G_Y.convertTo(G_Y, CV_32F);
			for (int i = 0; i < G_Y.rows; i++) {
				for (int j = 0; j < G_Y.cols; j++) {
					G_Y.at<float>(i, j) = G_Y.at<float>(i, j) - brightness_difference;	//29
				}
			}
			G_Y.convertTo(G_Y, CV_8UC1);

			chan[1] = RectifyCbCr(chan[1], binaryImg_background, binaryImg_txt, chan[0], chan[1]);
			chan[0] = RectifyCbCr(chan[0], binaryImg_background, binaryImg_txt, chan[0], chan[1]);
			//----------------------------------------------------------------------------------------------------------------------

			vector<Mat> channels_(3);//定义了三个Mat类型的向量
			channels_[0] = chan[0];
			channels_[1] = chan[1];
			channels_[2] = G_Y;

			Mat img_YCbCr_rec;
			merge(channels_, img_YCbCr_rec);
			Mat rec_img = YCbCr2RGB(img_YCbCr_rec);//将YCbCr格式的图像转回rgb格式

			duration = (clock() - start) / (double)CLOCKS_PER_SEC;//CLOCKS_PER_SEC用来表示一秒钟有多少个计时单元，duration用来记录处理一张图片需要多少秒
			cout << "Total Time: " << duration << " sec" << endl << endl << endl;//输出处理每张图片的时间
			//将处理后的图像写回到cSavePath路径下
			sprintf(cSavePath, "evaluation data\\Test00_results\\%s-G_Y0.jpg", strImgName.c_str());
			//sprintf(cSavePath, "evaluation data\\Natural_results\\%s.jpg", strImgName.c_str());
			//sprintf(cSavePath, "evaluation data\\Adobe_results\\%s.jpg", strImgName.c_str());
			//sprintf(cSavePath, "evaluation data\\HS_results\\%s.jpg", strImgName.c_str());

			//imwrite(cSavePath, rec_img);
			imwrite(cSavePath, G_Y0);
			//imwrite(cSavePath, binaryImg);
			//imwrite(cSavePath, result);
		}
		else {
			continue;
		}
	}
	fclose(fp);

	/*
		在一个程序中写有 system ("pause")语句时，就是说从该程序里调用了“pause”命令，即让操作系统来暂停该程序进程的执行，
		同时程序运行到此语句处时，会在程序的窗口上显示“Press any key to continue . . .”. 也就是 “按任意键继续...”，
		即等待用户按下任意一个按键使该程序从暂停状态返回到执行状态继续从刚才暂停的地方开始执行。
	*/
	system("pause");
	return 0;
}
