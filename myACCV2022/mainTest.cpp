#include "waterFilling.h"
#include <ctime>
#include <fstream> 

using namespace cv;
using namespace std;

//����ͼƬ����

/*
�˴��������ҪĿ�����ڶ�Yͨ�����м򵥵Ķ�ֵ������imagewatch���Կ���Yͨ�������ֲ������ػ�������180���£��������ִ�Լƽ������ֵΪ220��
�ɴ˴�������н�����Կ�����
1������עˮ�㷨��ͼ����н���֮����Ӱ���򷺻���CbCrͨ���йأ�
	��Ϊ����ֵ����Yͨ��merge��ԭͼ֮����Ӱ�������ɷ��ƣ���һ��Ӧ���ص���ڶ�CbCrͨ����ɫ�ʽ���������
2������ȷ������Ӱ���ع��ε�Yͨ����ֵ��180-220֮�䣬��Ӱ���صĹ��κ��ڲ��ķ�����������ͬ��ԭ����Ҫ�ֱ���
*/

int main(int argc, const char** argv) {
	const char* cTxt = "imgsNew.txt";					//imgs.txt�����Ҫ�����ͼ����

	FILE*  fp;
	if ((fp = fopen(cTxt, "r")) == NULL) {			//��imgs.txt���Ҳ����ļ��򷵻ش�����Ϣ
		printf("Open Falied!");
		return -1;
	}

	char cInputImagePath[150];
	char cSavePath[150];
	char strLine[100];
	string strImgName;//ÿһ�Ŵ������ͼ����ļ���
	while (!feof(fp))//feof(fp)����ļ��Ƿ��������������򷵻ط���ֵ�����򷵻�0
	{
	
		fgets(strLine, 100, fp);	//��fp��ָ����ļ��ж�ȡһ���ַ�(���99��)�����ַ�������strLine�У�Ҳ����ͼ����ļ�����
		strImgName = strLine;
		
		if (strImgName[strlen(strImgName.c_str()) - 1] == '\n') {
			strImgName[strlen(strImgName.c_str()) - 1] = 0;
		}
		//��"imgs\\"��ͼ����ļ�����eg. 100.jpg��ƴ�ӳ�һ����Ե�ַ·�����ַ�����ŵ�cInputImagePath��
		sprintf(cInputImagePath, "imgsNew\\%s", strImgName.c_str());

		cout << strImgName << endl;//����̨�����ʱ�����ͼƬ�ļ���
		Mat img = imread(cInputImagePath);//��ȡͼ�񣬽�ͼ�����ݱ��浽����img��
		if (!img.empty()) {
			Mat img_YCbCr = RGB2YCbCr(img); //��ԭͼimg��bgr��ʽת��ΪYCbCr��ʽ
			Mat chan[3];
			split(img_YCbCr, chan);//��ִ洢ÿ��ͨ��������;
			Mat Y = chan[2];
			//----------------------------------------------------------------------------------------------------------------------------------------------
			//ͳ��Yͨ�������ֲ��ֵ�ƽ��ֵ
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
			//	��Yͨ�������ֲ��ֵ�ƽ��ֵ���¸�ֵ
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
			vector<Mat> channels_(3);//����������Mat���͵�����
			channels_[0] = chan[0];
			channels_[1] = chan[1];
			channels_[2] = Y; 
			Mat img_YCbCr_rec;
			merge(channels_, img_YCbCr_rec);

			Mat rec_img = YCbCr2RGB(img_YCbCr_rec);//��YCbCr��ʽ��ͼ��ת��rgb��ʽ
			//��������ͼ��д�ص�cSavePath·����
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
