#include "waterFilling.h"
#include <ctime>
#include <fstream> 

using namespace cv;
using namespace std;

//����ͼƬ����

int main(int argc, const char** argv) {
	const char* cTxt = "imgs.txt";					//imgs.txt�����Ҫ�����ͼ����

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
		sprintf(cInputImagePath, "imgs\\%s.jpg", strImgName.c_str());

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
			Mat G_Y = FloodAndEffuse(chan[2]);//��Yͨ��ʹ�ú鷺���õ�G
			G_Y = IncrementalFilling(G_Y);//��Gʹ������עˮ��
			Mat  binaryImg_background = GetBinary(G_Y);	//�Ա�������ж�ֵ����CV_8UC1��ʽ��
			G_Y = Lambertian(G_Y, chan[2]);//ʹ���ʲ�����ģ�ͻ�ù�Ƚ���ͼ��
			Mat  binaryImg_txt = GetBinary(G_Y);	//�����ֽ��ж�ֵ����CV_8UC1��ʽ��

			//chan[1] = RectifyCbCr(chan[1], binaryImg_background, binaryImg_txt);
			//chan[0] = RectifyCbCr(chan[0], binaryImg_background, binaryImg_txt);

			//G_Y = FilterTextValue(G_Y, binaryImg_txt);
			chan[1] = FilterTextValue(chan[1], binaryImg_txt);
			chan[0] = FilterTextValue(chan[0], binaryImg_txt);
			//----------------------------------------------------------------------------------------------------------------------

			vector<Mat> channels_(3);//����������Mat���͵�����
			channels_[0] = chan[0];
			channels_[1] = chan[1];
			channels_[2] = G_Y;

			Mat img_YCbCr_rec;
			merge(channels_, img_YCbCr_rec); 
			//imshow("text area", img_YCbCr_rec);
			Mat rec_img = YCbCr2RGB(img_YCbCr_rec);//��YCbCr��ʽ��ͼ��ת��rgb��ʽ

			duration = (clock() - start) / (double)CLOCKS_PER_SEC;//CLOCKS_PER_SEC������ʾһ�����ж��ٸ���ʱ��Ԫ��duration������¼����һ��ͼƬ��Ҫ������
			cout << "Total Time: " << duration << " sec" << endl << endl << endl;//�������ÿ��ͼƬ��ʱ��

			//��������ͼ��д�ص�cSavePath·����
			sprintf(cSavePath, "results\\%s-v3.7.jpg", strImgName.c_str());
			//imwrite(cSavePath, rec_img);

			//imwrite(cSavePath, G_Y);
			//imwrite(cSavePath, binaryImg);
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
