#include "waterFilling.h"

float MinWithZero(float input) {		//与0比较，求min
	float min;
	if (input > 0) {
		min = 0;
	}
	else {
		min = input;
	}
	return min;
}

//洪泛法
Mat FloodAndEffuse(Mat input) {
	//step3:下采样
	input.convertTo(input, CV_32FC1);
	Mat inputSmall;	//初始地表高度h
	resize(input, inputSmall, Size(round(input.cols*0.2), round(input.rows*0.2)), 0, 0, INTER_LINEAR);

	Mat w = Mat(inputSmall.size(), CV_32F, Scalar(0, 0, 0));		//water level水平面
	Mat G = Mat(inputSmall.size(), CV_32F, Scalar(0, 0, 0));		//overall attitude总高度, G = h + w

	double neta = 0.2;			//neta决定注水过程的速 度，必须小于等于0.25
	double G_peak, G_min;	//G的全局最大最小值（最小值用不上）

	for (int t = 0; t <2500; t++) {	//t为时间,也是设定的迭代次数
		G = w + inputSmall;			//step 5,G = w + Y
		minMaxLoc(G, &G_min, &G_peak);		//获取全局最大最小值（最小值用不到）
		// 根据当前的总高度G计算对应的flood and effuse 后水平面w的高度
		for (int x = 1; x < (inputSmall.rows - 2); x++) {			//为什么从1开始，到rows/cols－2结束：因为effuseOut的计算里需要对x、y进行加减1的操作，防止数组越界
			for (int y = 1; y < (inputSmall.cols - 2); y++) {
				double w_pre = w.at<float>(x, y);	//上次迭代后w的值
				double floodIn = exp(-t) * (G_peak - G.at<float>(x, y));	//flooding process, 表示从全局最高点流入到该点的水
				double effuseOut = neta * (
					MinWithZero(G.at<float>(x + 1, y) - G.at<float>(x, y))	//effusion process,表示从该点向四周溢出的水
					+ MinWithZero(G.at<float>(x - 1, y) - G.at<float>(x, y))
					+ MinWithZero(G.at<float>(x, y + 1) - G.at<float>(x, y))
					+ MinWithZero(G.at<float>(x, y - 1) - G.at<float>(x, y))
					);
				float w_temp = effuseOut + floodIn + w_pre;
				//CV_32F的取值不能取负值，所以加入一个判断
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
	resize(G, output, Size(input.cols, input.rows), 0, 0, INTER_LINEAR);//step6：使用双线性插值法恢复图像原始尺寸
	output.convertTo(output, CV_8UC1);
	return output;
}

//增量注水法v2
Mat IncrementalFilling(Mat GAfterFlood)
{
	GAfterFlood.convertTo(GAfterFlood, CV_32F);

	Mat w = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
	Mat G = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
	Mat G_pre = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));//每次迭代对应的上一次迭代产生的G_，在迭代过程中一旦所有像素点对应的G_ - G_pre都 < 5则停止迭代
	
	double neta = 0.2;
	int count_t = 0;

	while (true) {
		G.copyTo(G_pre);//复制G的上一次迭代结果
		G = w + GAfterFlood; //更新G
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
			cout << "incre_filling总迭代次数 t: " << count_t << endl;
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
	Mat output = 0.85 * (originalY / GAfterFloodAndIncre) * 255;	//亮度因子l=0.85; Y = l*Y/G，之所以要乘255是因为Original / G_非常接近1
	output.convertTo(output, CV_8UC1);
	return output;
}

Mat RemoveTinyShadow(Mat input) {
	Mat output;
	input.copyTo(output);
	output.convertTo(output, CV_32F);

	
	//进一步剔除阴影
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
	//	对Y通道非文字部分的平均值重新赋值
	for (int i = 1; i < (output.rows - 2); i++) {
		for (int j = 1; j < (output.cols - 2); j++) {
			if (output.at<float>(i, j) > 170) {	//170
				output.at<float>(i, j) = YSum / num;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------
	////添加一步腐蚀/开运算操作，强化文字区域
	//Mat newOutput, myStruct;
	//myStruct = getStructuringElement(0, Size(3, 3));
	////erode(output, newOutput, myStruct);	//腐蚀
	//morphologyEx(output, newOutput, MORPH_OPEN, myStruct);		//开运算
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
			if (G_Y1.at<float>(i, j) > 180) {//180		获取G_Y1光亮背景部分的的平均值
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
				G_Y.at<float>(i, j) = avg;	//只考虑非文字区域G_Y0和G_Y1绝对值相差较大，且G_Y1 < G_Y0的像素点，用G_Y1光亮背景部分的的平均值替换
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
			if (binaryImg_background.at<uchar>(x,y) == 255 && binaryImg_txt.at<uchar>(x, y) == 255){	//只提取非文字部分的非阴影区域背景层
				light_sum += output.at<float>(x, y);
				light_count += 1;
			}
		}
	}
	double light_avg = light_sum / light_count;
	for (int x = 0; x < output.rows; x++) {
		for (int y = 0; y < output.cols; y++) {
			if (binaryImg_txt.at<uchar>(x, y) == 255) {	//非文字部分
				output.at<float>(x, y) = light_avg;	
			}
			else{
				if ((Cr.at<uchar>(x,y)>118 && Cr.at<uchar>(x, y) < 138) && (Cb.at<uchar>(x, y) > 118 && Cb.at<uchar>(x, y) < 138))	{	//黑色文字部分
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
			if (binaryImg_background.at<uchar>(x, y) == 255 && binaryImg_txt.at<uchar>(x, y) == 255) {	//统计非阴影且非文字区域
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
//			if (binaryImg_background.at<uchar>(x, y) == 255) {	//只提取非文字部分的非阴影区域背景层
//				light_sum += output.at<float>(x, y);
//				light_count += 1;
//			}
//		}
//	}
//	double light_avg = light_sum / light_count;
//	for (int x = 0; x < output.rows; x++) {
//		for (int y = 0; y < output.cols; y++) {
//			output.at<float>(x, y) = light_avg;	//不考虑文字部分，全局替换之前的cbcr数值
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
				output.at<float>(x, y) = 0.0;	//非文字区域设为零
			}
		}
	}
	output.convertTo(output, CV_8UC1);
	return output;
}

//增量注水法v1
//Mat IncrementalFilling(Mat GAfterFlood, Mat originalY)//G_, chan[2]
//{
//	GAfterFlood.convertTo(GAfterFlood, CV_32F);
//	originalY.convertTo(originalY, CV_32F);
//
//	Mat w = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
//	Mat G = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));
//	Mat G_pre = Mat(GAfterFlood.size(), CV_32F, Scalar(0, 0, 0));//每次迭代对应的上一次迭代产生的G_，在迭代过程中一旦所有像素点对应的G_ - G_pre都 < 5则停止迭代
//
//	double neta = 0.2;
//	for (int t = 0; t < 100; t++){
//		G = w + GAfterFlood; //更新G
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
//	Mat output = brightness_factor * (originalY / G) * 255;	//亮度因子l=0.85; Y = l*Y/G，之所以要乘255是因为Original / G_非常接近1
//	output.convertTo(output, CV_8UC1);
//	return output;
//
//	//return G;
//}