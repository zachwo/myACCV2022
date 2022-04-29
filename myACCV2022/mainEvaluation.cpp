#include"common.h"
#include"basic.h"
#include"HardShadowRemoval.h"


#include "waterFilling.h"
#include <ctime>
#include <fstream> 

using namespace cv;
using namespace std;

//����ͼƬ����

int main(int argc, const char** argv) {
	const char* cTxt = "evaluation data\\Test00.txt";					//Test00.txt�����Ҫ�����ͼ����
	//const char* cTxt = "evaluation data\\Natural.txt";
	//const char* cTxt = "evaluation data\\Adobe.txt";
	//const char* cTxt = "evaluation data\\HS.txt";

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
		/*
			fgets(char* str, int size, FILE* stream)���ڴ��ļ��ж�ȡ�ַ�����ÿ��ֻ��ȡһ��
				fgetsÿ�����ֻ�ܶ�ȡsize-1���ַ�������ʱ��size���ַ�ΪNULL
				���������з���EOFʱ����ʹ��ǰλ����n-1֮ǰҲ��������
				���������سɹ����򷵻��ַ�������str���׵�ַ
		*/
		fgets(strLine, 100, fp);	//��fp��ָ����ļ��ж�ȡһ���ַ�(���99��)�����ַ�������strLine�У�Ҳ����ͼ����ļ�����
		strImgName = strLine;
		/*c_str()��������һ��const���ε�ָ���ַ���strImgName��ָ�룬������strImgName��ͬ
		����ɹ���ȡ����һ�����ݣ����Ҳ������һ�У�����'\n'��β���������һ�������ַ���Ϊ0��ascii��ӦΪNULL��
		�����ȡ�ĵ��������һ�����ݣ�����EOF������ַ�����β�Զ���NULL*/
		if (strImgName[strlen(strImgName.c_str()) - 1] == '\n') {
			strImgName[strlen(strImgName.c_str()) - 1] = 0;
		}
		//��"imgs\\"��ͼ����ļ�����eg. 100.jpg��ƴ�ӳ�һ����Ե�ַ·�����ַ�����ŵ�cInputImagePath��
		sprintf(cInputImagePath, "evaluation data\\Test00_Original\\%s.jpg", strImgName.c_str());
		//sprintf(cInputImagePath, "evaluation data\\Natural_Original\\%s.jpg", strImgName.c_str());
		//sprintf(cInputImagePath, "evaluation data\\Adobe_Original\\%s.jpg", strImgName.c_str());
		//sprintf(cInputImagePath, "evaluation data\\HS_Original\\%s.jpg", strImgName.c_str());

		cout << strImgName << endl;//����̨�����ʱ�����ͼƬ�ļ���
		Mat img = imread(cInputImagePath);//��ȡͼ�񣬽�ͼ�����ݱ��浽����img��
		if (!img.empty()) {
			clock_t start;//clock_t��һ����������
			double duration;
			start = clock();

			Mat img_YCbCr = RGB2YCbCr(img); //��ԭͼimg��bgr��ʽת��ΪYCbCr��ʽ
			Mat chan[3];
			split(img_YCbCr, chan);//��ִ洢ÿ��ͨ��������

			//----------------------------------------------------------------------------------------------------------------------
			//�һ�ȡ���ı�����
			Mat G_Y0 = FloodAndEffuse(chan[2]);//��Yͨ��ʹ�ú鷺���õ�G
			G_Y0 = IncrementalFilling(G_Y0);//��Gʹ������עˮ��

			//ʹ����ʦsensor�����еķ�����ȡ������
			Mat bg(img.size(), CV_8UC3, Scalar(0, 0, 0));
			GetRefBg(img, bg);   //����Water_Filling���ͼ��ο�����ͼ
			Mat bg_YCbCr = RGB2YCbCr(bg);
			Mat bg_chan[3];
			split(bg_YCbCr, bg_chan);
			Mat G_Y1;
			bg_chan[2].copyTo(G_Y1);

			Mat  binaryImg_background = GetBinary(G_Y0);	//�Ա�������ж�ֵ����CV_8UC1��ʽ��
			G_Y0 = Lambertian(G_Y0, chan[2]);//ʹ���ʲ�����ģ�ͻ�ù�Ƚ���ͼ��
			G_Y0 = RemoveTinyShadow(G_Y0);	//��һ��ȥ��ϸС��Ӱ����
			Mat  binaryImg_txt0 = GetBinary(G_Y0);

			G_Y1 = Lambertian(G_Y1, chan[2]);//ʹ���ʲ�����ģ�ͻ�ù�Ƚ���ͼ��
			G_Y1 = RemoveTinyShadow(G_Y1);	//��һ��ȥ��ϸС��Ӱ����

			Mat G_Y = GetG_Y(G_Y0, G_Y1, binaryImg_txt0);
			Mat  binaryImg_txt = GetBinary(G_Y);	//�����ֽ��ж�ֵ����CV_8UC1��ʽ��

			//ͳ��Yͨ�������������ǰ����Ӱ�����������ƽ�����Ȳ�ֵ
			float brightness_difference = GetBrightnessDifference(G_Y, chan[2], binaryImg_background, binaryImg_txt0);
			cout << "brightness_difference = " << brightness_difference << endl;

			//������Yͨ����ȥ���Ȳ�ֵ��ʹ����ͼ�����Ⱦ�����������ǰ����Ӱ���������������һ��
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

			vector<Mat> channels_(3);//����������Mat���͵�����
			channels_[0] = chan[0];
			channels_[1] = chan[1];
			channels_[2] = G_Y;

			Mat img_YCbCr_rec;
			merge(channels_, img_YCbCr_rec);
			Mat rec_img = YCbCr2RGB(img_YCbCr_rec);//��YCbCr��ʽ��ͼ��ת��rgb��ʽ

			duration = (clock() - start) / (double)CLOCKS_PER_SEC;//CLOCKS_PER_SEC������ʾһ�����ж��ٸ���ʱ��Ԫ��duration������¼����һ��ͼƬ��Ҫ������
			cout << "Total Time: " << duration << " sec" << endl << endl << endl;//�������ÿ��ͼƬ��ʱ��
			//��������ͼ��д�ص�cSavePath·����
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
		��һ��������д�� system ("pause")���ʱ������˵�Ӹó���������ˡ�pause��������ò���ϵͳ����ͣ�ó�����̵�ִ�У�
		ͬʱ�������е�����䴦ʱ�����ڳ���Ĵ�������ʾ��Press any key to continue . . .��. Ҳ���� �������������...����
		���ȴ��û���������һ������ʹ�ó������ͣ״̬���ص�ִ��״̬�����Ӹղ���ͣ�ĵط���ʼִ�С�
	*/
	system("pause");
	return 0;
}
