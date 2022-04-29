#include "waterFilling.h"
#include <ctime>
#include <fstream> 
#include <math.h>

using namespace cv;
using namespace std;

//�����ж�double���͵���ֵ�Ƿ�Ϊ��
#define MIN_VALUE 1e-8 
#define IS_DOUBLE_ZERO(d)  (abs(d) < MIN_VALUE)

//����ͼƬ����

/*
���ǹ���rgb2rgbyר���㷨��������ȡYɫ��ͨ��

�˴��������ҪĿ�����ڶ�Yͨ�����м򵥵Ķ�ֵ������imagewatch���Կ���Yͨ�������ֲ�������ֵ��100���£��������ִ�Լƽ������ֵΪ200��
�ɴ˴�������н�����Կ���������עˮ�㷨��ͼ����н���֮����Ӱ���򷺻���CbCrͨ���йأ�
��Ϊ����ֵ����Yͨ��merge��ԭͼ֮����Ӱ�������ɷ���
��һ��Ӧ���ص���ڶ�CbCrͨ����ɫ�ʽ���������
*/

int main(int argc, const char** argv) {
	//const char* cTxt = "imgsNew.txt";					//imgs.txt�����Ҫ�����ͼ����
	const char* cTxt = "evaluation data\\Test00.txt";
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
			
			//��������ͼ��д�ص�cSavePath·����
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
