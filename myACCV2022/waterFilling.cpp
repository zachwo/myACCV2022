#include "waterFilling.h"

float MinWithZero(float input) {		//��0�Ƚϣ���min
	float min;
	if (input > 0) {
		min = 0;
	}
	else {
		min = input;
	}
	return min;
}

//�鷺��
Mat FloodAndEffuse(Mat input) {
	//step3:�²���
	input.convertTo(input, CV_32FC1);
	Mat inputSmall;	//��ʼ�ر�߶�h
	resize(input, inputSmall, Size(round(input.cols*0.2), round(input.rows*0.2)), 0, 0, INTER_LINEAR);

	Mat w = Mat(inputSmall.size(), CV_32F, Scalar(0, 0, 0));		//water levelˮƽ��
	Mat G = Mat(inputSmall.size(), CV_32F, Scalar(0, 0, 0));		//overall attitude�ܸ߶�, G = h + w

	double neta = 0.2;			//neta����עˮ���̵��� �ȣ�����С�ڵ���0.25
	double G_peak, G_min;	//G��ȫ�������Сֵ����Сֵ�ò��ϣ�

	for (int t = 0; t <2500; t++) {	//tΪʱ��,Ҳ���趨�ĵ�������
		G = w + inputSmall;			//step 5,G = w + Y
		minMaxLoc(G, &G_min, &G_peak);		//��ȡȫ�������Сֵ����Сֵ�ò�����
		// ���ݵ�ǰ���ܸ߶�G�����Ӧ��flood and effuse ��ˮƽ��w�ĸ߶�
		for (int x = 1; x < (inputSmall.rows - 2); x++) {			//Ϊʲô��1��ʼ����rows/cols��2��������ΪeffuseOut�ļ�������Ҫ��x��y���мӼ�1�Ĳ�������ֹ����Խ��
			for (int y = 1; y < (inputSmall.cols - 2); y++) {
				double w_pre = w.at<float>(x, y);	//�ϴε�����w��ֵ
				double floodIn = exp(-t) * (G_peak - G.at<float>(x, y));	//flooding process, ��ʾ��ȫ����ߵ����뵽�õ��ˮ
				double effuseOut = neta * (
					MinWithZero(G.at<float>(x + 1, y) - G.at<float>(x, y))	//effusion process,��ʾ�Ӹõ������������ˮ
					+ MinWithZero(G.at<float>(x - 1, y) - G.at<float>(x, y))
					+ MinWithZero(G.at<float>(x, y + 1) - G.at<float>(x, y))
					+ MinWithZero(G.at<float>(x, y - 1) - G.at<float>(x, y))
					);
				float w_temp = effuseOut + floodIn + w_pre;
				//CV_32F��ȡֵ����ȡ��ֵ�����Լ���һ���ж�
				(w_temp < 0) ? (w.at<float>(x, y) = 0) : (w.at<float>(x, y) = w_temp);
				/*if ((t % 100== 0) && t != 0 ){
					cout << "t = " << t << endl;
					cout << "w_pre is: " << w_pre << endl;
					cout << "floodIn is: " << floodIn << endl;
					cout << "effuseOut is: " << effuseOut << endl;
					cout << "w.at<float>(x, y): " << w.at<float>(x, y) << endl;
					cout << "G.at<float>(x, y): " << G.at<float>(x, y) << endl;
				}*/
			}
		}
	}

	Mat output;
	resize(G, output, Size(input.cols, input.rows), 0, 0, INTER_LINEAR);//step6��ʹ��˫���Բ�ֵ���ָ�ͼ��ԭʼ�ߴ�
	output.convertTo(output, CV_8UC1);
	return output;
}

//����עˮ��v2
Mat IncrementalFilling(Mat GAfterFlood)
{
	GAfterFlood.convertTo(GAfterFlood, CV_32F);

	Mat w = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
	Mat G = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
	Mat G_pre = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));//ÿ�ε�����Ӧ����һ�ε���������G_���ڵ���������һ���������ص��Ӧ��G_ - G_pre�� < 5��ֹͣ����
	
	double neta = 0.2;
	int count_t = 0;

	while (true) {
		G.copyTo(G_pre);//����G����һ�ε������
		G = w + GAfterFlood; //����G
		bool flag = true;
		for (int x = 1; x < (GAfterFlood.rows - 2); x++) {
			for (int y = 1; y < (GAfterFlood.cols - 2); y++) {
				if (G.at<float>(x, y) - G_pre.at<float>(x, y) > 5) {	//best 5
					flag = false;
					break;
				}
				/*
				cout << G_ptr[x + y * elem_step] - G_pre_ptr[x + y * elem_step] << endl;

				if (x == 1){
					cout << "G_ptr_pre: " << G_pre_ptr[x + y * elem_step] / 256<< endl;
					cout << "G_ptr: " << G_ptr[x + y * elem_step] / 256<< endl;
				}*/
			}
			if (flag == false) {
				break;
			}
		}
		if (flag) {
			cout << "incre_filling�ܵ������� t: " << count_t << endl;
			break;
		}
		count_t++;

		for (int x = 1; x < (GAfterFlood.rows - 2); x++) {
			for (int y = 1; y < (GAfterFlood.cols - 2); y++) {
				double w_pre = w.at<float>(x, y);
				double fillIn = neta * (G.at<float>(x + 1, y) + G.at<float>(x - 1, y) + G.at<float>(x, y + 1) + G.at<float>(x, y - 1) - 4 * G.at<float>(x, y));
				float w_temp = fillIn + w_pre; 
				(w_temp < 0) ? (w.at<float>(x, y) = 0) : (w.at<float>(x, y) = w_temp);
			}
		}

	}
	
	G.convertTo(G,CV_8UC1);
	return G;
}

Mat Lambertian(Mat GAfterFloodAndIncre, Mat originalY) {
	GAfterFloodAndIncre.convertTo(GAfterFloodAndIncre, CV_32F);
	originalY.convertTo(originalY, CV_32F);
	Mat output = 0.85 * (originalY / GAfterFloodAndIncre) * 255;	//��������l=0.85; Y = l*Y/G��֮����Ҫ��255����ΪOriginal / G_�ǳ��ӽ�1
	output.convertTo(output, CV_8UC1);
	return output;
}

Mat RemoveTinyShadow(Mat input) {
	Mat output;
	input.copyTo(output);
	output.convertTo(output, CV_32F);

	
	//��һ���޳���Ӱ
	int num = 0;
	double YSum = 0.0;
	for (int i = 1; i < (output.rows - 2); i++) {
		for (int j = 1; j < (output.cols - 2); j++) {
			if (output.at<float>(i, j) > 180) {//180
				YSum += output.at<float>(i, j);
				num += 1;
			}
		}
	}
	//	��Yͨ�������ֲ��ֵ�ƽ��ֵ���¸�ֵ
	for (int i = 1; i < (output.rows - 2); i++) {
		for (int j = 1; j < (output.cols - 2); j++) {
			if (output.at<float>(i, j) > 170) {	//170
				output.at<float>(i, j) = YSum / num;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------
	////���һ����ʴ/�����������ǿ����������
	//Mat newOutput, myStruct;
	//myStruct = getStructuringElement(0, Size(3, 3));
	////erode(output, newOutput, myStruct);	//��ʴ
	//morphologyEx(output, newOutput, MORPH_OPEN, myStruct);		//������
	//newOutput.copyTo(output);
	//--------------------------------------------------------------------------------------------------

	output.convertTo(output, CV_8UC1);
	return output;
}
Mat GetG_Y(Mat G_Y0, Mat G_Y1, Mat  binaryImg_txt) {
	G_Y0.convertTo(G_Y0, CV_32F);
	G_Y1.convertTo(G_Y1, CV_32F);
	
	int num = 0;
	double YSum = 0.0;
	for (int i = 1; i < (G_Y1.rows - 2); i++) {
		for (int j = 1; j < (G_Y1.cols - 2); j++) {
			if (G_Y1.at<float>(i, j) > 180) {//180		��ȡG_Y1�����������ֵĵ�ƽ��ֵ
				YSum += G_Y1.at<float>(i, j);
				num += 1;
			}
		}
	}
	double avg = YSum / num;
	Mat G_Y;
	G_Y1.copyTo(G_Y);
	for (int i = 1; i < (G_Y.rows - 2); i++) {
		for (int j = 1; j < (G_Y.cols - 2); j++) {
			if (abs(G_Y1.at<float>(i, j) - G_Y0.at<float>(i, j)) > 30 && G_Y1.at<float>(i, j) < G_Y0.at<float>(i, j) && binaryImg_txt.at<uchar>(i, j) == 255){
				G_Y.at<float>(i, j) = avg;	//ֻ���Ƿ���������G_Y0��G_Y1����ֵ���ϴ���G_Y1 < G_Y0�����ص㣬��G_Y1�����������ֵĵ�ƽ��ֵ�滻
			}
		}
	}

	G_Y.convertTo(G_Y, CV_8UC1);
	return G_Y;
}

Mat GetBinary(Mat input) {
	Mat output;
	input.copyTo(output);
	output.convertTo(output, CV_32F);
	double sum = 0.0;

	for (int x = 1; x < (output.rows - 2); x++) {
		for (int y = 1; y < (output.cols - 2); y++) {
			sum += output.at<float>(x, y);
		}
	}

	double avg = sum / ((output.rows - 1) * (output.cols - 1));

	for (int x = 1; x < (output.rows - 2); x++) {
		for (int y = 1; y < (output.cols - 2); y++) {
			(output.at<float>(x, y) > avg) ? (output.at<float>(x, y) = 255) : (output.at<float>(x, y) = 0);
		}
	}
	output.convertTo(output, CV_8UC1);
	return output;
}

Mat GetBinaryTxt(Mat input) {
	Mat output;
	input.copyTo(output);
	output.convertTo(output, CV_32F);
	double sum = 0.0;

	for (int x = 1; x < (output.rows - 2); x++) {
		for (int y = 1; y < (output.cols - 2); y++) {
			sum += output.at<float>(x, y);
		}
	}

	double avg = sum / ((output.rows - 1) * (output.cols - 1));

	for (int x = 1; x < (output.rows - 2); x++) {
		for (int y = 1; y < (output.cols - 2); y++) {
			(output.at<float>(x, y) >( avg-10)) ? (output.at<float>(x, y) = 255) : (output.at<float>(x, y) = 0);
		}
	}
	output.convertTo(output, CV_8UC1);
	return output;
}

Mat RectifyCbCr(Mat input, Mat  binaryImg_background, Mat  binaryImg_txt, Mat Cr, Mat Cb) {
	Mat output;
	input.copyTo(output);
	output.convertTo(output, CV_32F);

	double light_sum = 0.0;
	int light_count = 0;

	for (int x = 1; x < (output.rows - 2); x++) {
		for (int y = 1; y < (output.cols - 2); y++) {
			if (binaryImg_background.at<uchar>(x,y) == 255 && binaryImg_txt.at<uchar>(x, y) == 255){	//ֻ��ȡ�����ֲ��ֵķ���Ӱ���򱳾���
				light_sum += output.at<float>(x, y);
				light_count += 1;
			}
		}
	}
	double light_avg = light_sum / light_count;
	for (int x = 0; x < output.rows; x++) {
		for (int y = 0; y < output.cols; y++) {
			if (binaryImg_txt.at<uchar>(x, y) == 255) {	//�����ֲ���
				output.at<float>(x, y) = light_avg;	
			}
			else{
				if ((Cr.at<uchar>(x,y)>118 && Cr.at<uchar>(x, y) < 138) && (Cb.at<uchar>(x, y) > 118 && Cb.at<uchar>(x, y) < 138))	{	//��ɫ���ֲ���
					output.at<float>(x, y) = light_avg;
				}
			}
		}
	}
	
	output.convertTo(output, CV_8UC1);
	return output;
}

float GetBrightnessDifference(Mat Y_after, Mat chan_before, Mat  binaryImg_background, Mat  binaryImg_txt) {
	float brightness_difference = 0.0;
	float sum = 0.0;
	float count = 0.0;
	
	Y_after.convertTo(Y_after, CV_32F);
	chan_before.convertTo(chan_before, CV_32F);

	for (int x = 0; x < Y_after.rows; x++) {
		for (int y = 0; y < Y_after.cols; y++) {
			if (binaryImg_background.at<uchar>(x, y) == 255 && binaryImg_txt.at<uchar>(x, y) == 255) {	//ͳ�Ʒ���Ӱ�ҷ���������
				float difference = Y_after.at<float>(x, y) - chan_before.at<float>(x, y);
				sum += difference;
				count += 1;	
			}
		}
	}
	//cout << "sum = " << sum << endl;
	//cout << "count = " << count << endl;
	brightness_difference = sum / count;
	return brightness_difference;

}

//Mat RectifyCbCr(Mat input, Mat  binaryImg_background) {
//	Mat output;
//	input.copyTo(output);
//	output.convertTo(output, CV_32F);
//
//	double light_sum = 0.0;
//	int light_count = 0;
//
//	for (int x = 1; x < (output.rows - 2); x++) {
//		for (int y = 1; y < (output.cols - 2); y++) {
//			if (binaryImg_background.at<uchar>(x, y) == 255) {	//ֻ��ȡ�����ֲ��ֵķ���Ӱ���򱳾���
//				light_sum += output.at<float>(x, y);
//				light_count += 1;
//			}
//		}
//	}
//	double light_avg = light_sum / light_count;
//	for (int x = 0; x < output.rows; x++) {
//		for (int y = 0; y < output.cols; y++) {
//			output.at<float>(x, y) = light_avg;	//���������ֲ��֣�ȫ���滻֮ǰ��cbcr��ֵ
//		}
//	}
//
//	output.convertTo(output, CV_8UC1);
//	return output;
//}

Mat FilterTextValue(Mat input, Mat  binaryImg_txt) {
	Mat output;
	input.copyTo(output);
	output.convertTo(output, CV_32F);

	for (int x = 1; x < (output.rows - 2); x++) {
		for (int y = 1; y < (output.cols - 2); y++) {
			if (binaryImg_txt.at<uchar>(x, y) == 255) {
				output.at<float>(x, y) = 0.0;	//������������Ϊ��
			}
		}
	}
	output.convertTo(output, CV_8UC1);
	return output;
}

//����עˮ��v1
//Mat IncrementalFilling(Mat GAfterFlood, Mat originalY)//G_, chan[2]
//{
//	GAfterFlood.convertTo(GAfterFlood, CV_32F);
//	originalY.convertTo(originalY, CV_32F);
//
//	Mat w = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
//	Mat G = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
//	Mat G_pre = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));//ÿ�ε�����Ӧ����һ�ε���������G_���ڵ���������һ���������ص��Ӧ��G_ - G_pre�� < 5��ֹͣ����
//
//	double neta = 0.2;
//	for (int t = 0; t < 100; t++){
//		G = w + GAfterFlood; //����G
//		for (int x = 1; x < (GAfterFlood.rows - 2); x++) {
//			for (int y = 1; y < (GAfterFlood.cols - 2); y++) {
//				double w_pre = w.at<float>(x, y);
//				double fillIn = neta * (G.at<float>(x + 1, y) + G.at<float>(x - 1, y) + G.at<float>(x, y + 1) + G.at<float>(x, y - 1) - 4 * G.at<float>(x, y));
//				float w_temp = fillIn + w_pre;
//				(w_temp < 0) ? (w.at<float>(x, y) = 0) : (w.at<float>(x, y) = w_temp);
//			}
//		}
//	}
//	//step 9
//	double brightness_factor = 0.85;
//	Mat output = brightness_factor * (originalY / G) * 255;	//��������l=0.85; Y = l*Y/G��֮����Ҫ��255����ΪOriginal / G_�ǳ��ӽ�1
//	output.convertTo(output, CV_8UC1);
//	return output;
//
//	//return G;
//}