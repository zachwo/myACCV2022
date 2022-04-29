#include "waterFilling.h"
#include <ctime>
#include <fstream> 

using namespace cv;
using namespace std;

//����ͼƬ����

/*
�˴��������ҪĿ�����ڶ�Yͨ�����м򵥵Ķ�ֵ������imagewatch���Կ���Yͨ�������ֲ�������ֵ��100���£��������ִ�Լƽ������ֵΪ200��
�ɴ˴�������н�����Կ���������עˮ�㷨��ͼ����н���֮����Ӱ���򷺻���CbCrͨ���йأ�
��Ϊ����ֵ����Yͨ��merge��ԭͼ֮����Ӱ�������ɷ���
��һ��Ӧ���ص���ڶ�CbCrͨ����ɫ�ʽ���������
*/

int main(int argc, const char** argv) {
	const char* cTxt = "evaluation data\\Test00.txt";					//imgs.txt�����Ҫ�����ͼ����
	//const char* cTxt = "imgsNew.txt";

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
		sprintf(cInputImagePath, "evaluation data\\Test00_v2.1\\%s", strImgName.c_str());

		cout << strImgName << endl;//����̨�����ʱ�����ͼƬ�ļ���
		Mat img = imread(cInputImagePath);//��ȡͼ�񣬽�ͼ�����ݱ��浽����img��
		if (!img.empty()) {
			Mat img_YCbCr = RGB2YCbCr(img); //��ԭͼimg��bgr��ʽת��ΪYCbCr��ʽ
			Mat chan[3];
			split(img_YCbCr, chan);//��ִ洢ÿ��ͨ��������;
			Mat Y = chan[2];
			Mat Cb = chan[1];
			Mat Cr = chan[0];
			
			//ͳ��Yͨ�������ֲ��ֵ�ƽ��ֵ
			int num = 0;
			double YSum = 0.0;
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					//(Y.at<uchar>(i, j) > 180) ? (Y.at<uchar>(i, j) = 220) : (Y.at<uchar>(i, j) = Y.at<uchar>(i, j));
					if (Y.at<uchar>(i, j) > 180) {
						YSum += Y.at<uchar>(i, j);
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


			//Mat kernel = getStructuringElement(0, Size(3, 3));
			//dilate(Cb, Cb, kernel, Point(0, 0), 3);
			//morphologyEx(Cb, Cb, MORPH_CLOSE, kernel, Point(0, 0), 3);

			int countCb = 0;
			double sumCb = 0.0;
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					//Y.at<uchar>(i, j) = 215;
					//(Y.at<uchar>(i, j) > 180) ? (Y.at<uchar>(i, j) = 220) : (Y.at<uchar>(i, j) = Y.at<uchar>(i, j));
					sumCb += Cb.at<uchar>(i,j);
					countCb += 1;
				}
			}
			double avgCb = sumCb / countCb;
			//cout << "avg=" << avg << endl;
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					if (Cb.at<uchar>(i, j) < avgCb){
						Cb.at<uchar>(i, j)= avgCb;
					}
				}
			}
			
			int countCr = 0;
			double sumCr = 0.0;
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					//Y.at<uchar>(i, j) = 215;
					//(Y.at<uchar>(i, j) > 180) ? (Y.at<uchar>(i, j) = 220) : (Y.at<uchar>(i, j) = Y.at<uchar>(i, j));
					sumCr += Cr.at<uchar>(i, j);
					countCr += 1;
				}
			}
			double avgCr = sumCr / countCr;
			//cout << "avg=" << avg << endl;
			for (int i = 0; i < Y.rows - 1; i++) {
				for (int j = 0; j < Y.cols - 1; j++) {
					if (Cr.at<uchar>(i, j) < avgCr) {
						Cr.at<uchar>(i, j) = avgCr;
					}
				}
			}
			

			vector<Mat> channels_(3);//����������Mat���͵�����
			channels_[0] = Cr;
			channels_[1] = Cb;
			channels_[2] = Y;
			Mat img_YCbCr_rec;
			merge(channels_, img_YCbCr_rec);

			Mat rec_img = YCbCr2RGB(img_YCbCr_rec);//��YCbCr��ʽ��ͼ��ת��rgb��ʽ
			//-----------------------------------------------------------------------------------------------------------------------------
			//imshow("chan[0]", chan[0]);
			
			//imshow("Cb", Cb);
			
			//Mat gray;
			//cvtColor(rec_img, gray,COLOR_BGR2GRAY);
			//imshow("gray", gray);
			//waitKey(0);
			//-----------------------------------------------------------------------------------------------------------------------------
			//��������ͼ��д�ص�cSavePath·����
			sprintf(cSavePath, "evaluation data\\Test00_v2.2\\%s", strImgName.c_str());
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
