#include"HardShadowRemoval.h"


/*******************************************
*Function: Inv_Relu， inverse 
*Input: input_       
*Output:  
*return：float value
*date: 2019.08.08
********************************************/
float Inv_Relu(float input_) {
	float output_;
	if (input_ > 0) {
		output_ = 0;
	}
	else {
		output_ = input_;
	}
	return output_;
}




/*******************************************
*Function: Local water filling, get referance background
*Input: gray,   single channel image  
*       size,   the local neighborhood, (2*size+1)*(2*size+1)
*Output:estimateBg,    the shading map
*return： 
*date: 2019.08.08
********************************************/
void Local_Water_Filling(Mat& gray,int size,Mat& estimateBg)
{
	Mat altitude(gray.size(), CV_8UC1, Scalar(0));//各个点的初始海拔
	gray.copyTo(altitude);
	int height = gray.rows;
	int width = gray.cols;
	Mat peak(gray.size(), CV_8UC1,Scalar(0)); //各个点邻域的最高山峰
	Mat water(gray.size(), CV_8UC1, Scalar(0)); 
	
	for (int t = 0; t<3; t++)
	{
		//1、计算各个点所属的peak
		int minH, minW, maxH, maxW, max_value;
		uchar* puSrcTemp;
		for (int i = 0; i<height; i++)
		{
			uchar* puAltitude = altitude.ptr(i);
			uchar* puPeak = peak.ptr(i);
			for (int j = 0; j<width; j++)
			{
				minH = max(i - size, 0);
				maxH = min(i + size, height - 1);
				minW = max(j - size, 0);
				maxW = min(j + size, width - 1);
				max_value = 0;
				for (int ii = minH; ii <= maxH; ii++)
				{
					puSrcTemp = altitude.ptr(ii);
					for (int jj = minW; jj <= maxW; jj++)
					{
						max_value = max_value>puSrcTemp[jj] ? max_value : puSrcTemp[jj];
					}
				}
				puPeak[j] = max_value;
			}
		}

		//2、Water Filling and Effusion
		uchar* puAltitude = (uchar*)altitude.data;
		uchar* puPeak = (uchar*)peak.data;
		Mat G = Mat(gray.size(), CV_32F, Scalar(0));
		double neta = 0.22;  //0.22
		double pouring;
		double effusion;
		int temp,alti;

		for (int i = 1; i<height - 1; i++)
		{
			for (int j = 1; j<width - 1; j++)
			{
				alti = puAltitude[i*width + j]; //当前的海拔高度
				pouring = puPeak[i*width + j] - puAltitude[i*width + j];  //表示注入的水
				effusion = neta*(Inv_Relu(-puAltitude[i*width + j] + puAltitude[(i - 1)*width + j])  //表示溢出的水
					+ Inv_Relu(-puAltitude[i*width + j] + puAltitude[i*width + j + 1])
					+ Inv_Relu(-puAltitude[i*width + j] + puAltitude[(i + 1)*width + j])
					+ Inv_Relu(-puAltitude[i*width + j] + puAltitude[i*width + j - 1]));
				temp = pouring + effusion + alti;
				if (temp < 0)
				{
					puAltitude[i*width + j] = 0;
				}
				else if (temp>255)
				{
					puAltitude[i*width + j] = 255;
				}
				else
				{
					puAltitude[i*width + j] = temp;
				}
			}//j
		}//i
   }//t
   altitude.copyTo(estimateBg);
}


 
/*******************************************
*Function: Based on local water filling method to 
*          estimate the background.
*Input: img,   three channels image 
*Output:bg,    the estimated bg, three channels
*return：
*date: 2019.08.08
********************************************/
void GetRefBg(Mat& img, Mat& bg)
{
	int size = 2;
	vector<Mat> rgbChannels(3);
	split(img, rgbChannels);
	Mat refBg(img.size(), CV_8UC1, Scalar(0));
	for (int iChannel = 0; iChannel < 3; iChannel++)
	{
		Local_Water_Filling(rgbChannels[iChannel], size, refBg);
		refBg.copyTo(rgbChannels[iChannel]);
	}
	merge(rgbChannels, bg);
}


 
/*******************************************
*Function: once water filling method to   get gray image, 
*          text-(0,255)  background-0
*Input: img,   three channels image
*       size,  the local neighborhood, (2*size+1)*(2*size+1)
*Output: estimateTextImg,     single channel
*return：
*date: 2019.08.08
********************************************/
void Once_Water_Filling(Mat& gray, int size, Mat& estimateTextImg)
{
	Mat altitude(gray.size(), CV_8UC1, Scalar(0));//各个点的初始海拔
	gray.copyTo(altitude);
	int height = gray.rows;
	int width = gray.cols;
	Mat peak(gray.size(), CV_8UC1, Scalar(0)); //各个点邻域的最高山峰
	Mat water(gray.size(), CV_8UC1, Scalar(0));

	for (int t = 0; t<1; t++)  //t<1表示执行一次
	{
		//1、计算各个点所属的peak
		int minH, minW, maxH, maxW, max_value;
		uchar* puSrcTemp;
		for (int i = 0; i<height; i++)
		{
			uchar* puAltitude = altitude.ptr(i);
			uchar* puPeak = peak.ptr(i);
			for (int j = 0; j<width; j++)
			{
				minH = max(i - size, 0);
				maxH = min(i + size, height - 1);
				minW = max(j - size, 0);
				maxW = min(j + size, width - 1);
				max_value = 0;
				for (int ii = minH; ii <= maxH; ii++)
				{
					puSrcTemp = altitude.ptr(ii);
					for (int jj = minW; jj <= maxW; jj++)
					{
						max_value = max_value>puSrcTemp[jj] ? max_value : puSrcTemp[jj];
					}
				}
				puPeak[j] = max_value;
			}
		}

		//2、Water Filling and Effusion
		uchar* puAltitude = (uchar*)altitude.data;
		uchar* puPeak = (uchar*)peak.data;
		Mat G = Mat(gray.size(), CV_32F, Scalar(0));
		double neta = 1;    //neta 为1 表示只进行一次
		double pouring;
		double effusion;
		int temp, alti;

		for (int i = 1; i<height - 1; i++)
		{
			for (int j = 1; j<width - 1; j++)
			{
				alti = puAltitude[i*width + j]; //当前的海拔高度
				pouring = puPeak[i*width + j] - puAltitude[i*width + j];  //表示注入的水
				effusion = neta*(Inv_Relu(-puAltitude[i*width + j] + puAltitude[(i - 1)*width + j])  //表示溢出的水
					+ Inv_Relu(-puAltitude[i*width + j] + puAltitude[i*width + j + 1])
					+ Inv_Relu(-puAltitude[i*width + j] + puAltitude[(i + 1)*width + j])
					+ Inv_Relu(-puAltitude[i*width + j] + puAltitude[i*width + j - 1]));
				temp = pouring + effusion + alti;
				if (temp < 0)
				{
					puAltitude[i*width + j] = 0;
				}
				else if (temp>255)
				{
					puAltitude[i*width + j] = 255;
				}
				else
				{
					puAltitude[i*width + j] = temp;
				}
			}//j
		}//i
	}//t
	altitude.copyTo(estimateTextImg);
	//altitude.copyTo(gray);
}


 
/*******************************************
*Function: Based on  once water filling method to
*          get binarization image,  text-0  background-255    
*Input: img,   three channels image
*Output:binary,     single channel
*return：
*date: 2019.08.08
********************************************/
void GetWordLevelMask(Mat& img, Mat& binary)
{
	vector<Mat> rgbChannels(3);
	split(img, rgbChannels);
	Mat dst(img.size(), CV_8UC1, Scalar(0));
	Mat refBg(img.size(), CV_8UC3, Scalar(0,0,0));
	int size = 2;
	for (int iChannel = 0; iChannel < 3; iChannel++)
	{
		Once_Water_Filling(rgbChannels[iChannel], size, dst);
		dst.copyTo(rgbChannels[iChannel]);
		 
	}
	
	merge(rgbChannels, refBg);

	int height = img.rows;
	int width = img.cols;
	int maxValue;
	for (int i=0;i<height;i++)
	{
		uchar* puBg = refBg.ptr(i);
		uchar* puBinary= binary.ptr(i);
		for (int j=0;j<width;j++)
		{
			maxValue = MAX(puBg[3 * j], puBg[3 * j + 1]);
			maxValue = MAX(maxValue, puBg[3 * j + 2]);
			if (maxValue > 1)
			{
				puBinary[j] = 255;
			}
			else
			{
				puBinary[j] = 0;
			}
			if (puBg[3 * j] == 0 && puBg[3 * j + 1] == 0 && puBg[3 * j + 2] == 255)
			{
				puBinary[j] = 0;
			}
			if (puBg[3 * j] == 0 && puBg[3 * j + 1] == 255 && puBg[3 * j + 2] == 0)
			{
				puBinary[j] = 0;
			}
			if (puBg[3 * j] == 255 && puBg[3 * j + 1] == 0 && puBg[3 * j + 2] == 0)
			{
				puBinary[j] = 0;
			}

		}
	}

	binary = 255 - binary;
}



/*******************************************
*Function: Handle Penumbra by the operation between  binary and wlBinary 
*Input: binary, wlBinary       single channel
*       penumbra        single channel
*Output:binary ,    (0-shadow ,  255-bg)
*return：
*date: 2019.08.08
********************************************/
void HandlePenumbra(Mat& binary,Mat& wlBinary,Mat& penumbra)
{
	//dilate penumbra
	int size = 1;
	Mat element = getStructuringElement(MORPH_RECT, Size(2 * size + 1, 2 * size + 1), Point(size, size));
	for (int i=0;i<2;i++)
	{
		dilate(penumbra, penumbra, element);
	}

	int height = binary.rows;
	int width = binary.cols;
	for (int i = 0; i < height; i++)
	{
		uchar* puWlBinary = wlBinary.ptr(i);
		uchar* puBinary = binary.ptr(i);
		uchar* puPenumbra = penumbra.ptr(i);
		for (int j = 0; j < width; j++)
		{
			if (puPenumbra[j]==255 && puBinary[j]!= puWlBinary[j])
			{
				puBinary[j] = 255;
			}
		}
	}
}


 
/*******************************************
*Function: Get umbra and penumbra images from the estimated background image
*Input: bg,    estimated background image with 3 channels  
*Output:umbra ,    (255-shadow region,  0-unshadowed region)
*       penumbra,  (255-shadow region,  0-unshadowed region)
*return：
*date: 2019.08.08
********************************************/
void GetUmbraAndPenumbra(Mat& bg, Mat& umbra, Mat& penumbra)
{
	int height = bg.rows;
	int width = bg.cols;
	Mat mask(bg.size(), CV_8UC1, Scalar(0));
	vector<Mat> rgbChannels(3);
	split(bg, rgbChannels);

	for (int iChannel = 0; iChannel < 3; iChannel++)
	{
		medianBlur(rgbChannels[iChannel], rgbChannels[iChannel], 3);
		threshold(rgbChannels[iChannel], mask, 0, 255, THRESH_OTSU);
		mask = 255 - mask;
		mask.copyTo(rgbChannels[iChannel]);
	}

	int countNum = 0;
	for (int i = 0; i<height; i++)
	{
		uchar* puB = rgbChannels[0].ptr(i);
		uchar* puG = rgbChannels[1].ptr(i);
		uchar* puR = rgbChannels[2].ptr(i);
		uchar* puMask = mask.ptr(i);
		for (int j = 0; j < width; j++)
		{
			countNum = 0;
			if (puB[j] == 255) countNum++;
			if (puG[j] == 255) countNum++;
			if (puR[j] == 255) countNum++;
			if (countNum > 1)
			{
				puMask[j] = 255;
			}
			else {
				puMask[j] = 0;
			}

		}
	}

	int border = 2;  //set the border ( 2 pixels width) as 0
	for (int i = 0; i<height; i++)
	{
		uchar* puMask = mask.ptr(i);
		for (int j = 0; j<width; j++)
		{
			if (i<border || i>height - border || j<border || j>width - border)
			{
				puMask[j] = 0;
			}
		}
	}
    //obtain the umbra region
	int size = 2;
	Mat element = getStructuringElement(MORPH_RECT, Size(2 * size + 1, 2 * size + 1), Point(size, size));
	dilate(mask, umbra, element);   


	//obtain the penumbra region
	Mat dilateMask(bg.size(), CV_8UC1, Scalar(0));
	size = 2;
	int ite_num = 2;	
	element = getStructuringElement(MORPH_RECT, Size(2 * size + 1, 2 * size + 1), Point(size, size));
	for (int i = 0; i < ite_num; i++)
	{
		dilate(mask, dilateMask, element);
		dilateMask.copyTo(mask);
	}
	penumbra = dilateMask - umbra;    
	dilate(penumbra, penumbra, element);
}

 
/*******************************************
*Function: Get colored images that label umbra (blue) and penumbra (red), respectively.  
*Input: img,     3 channels
*       umbra ,    (255-shadow region,  0-unshadowed region)
*       penumbra,  (255-shadow region,  0-unshadowed region)
*Output:img, colored  
*return：
*date: 2019.08.08
********************************************/
void GetColorImg(Mat& img, Mat& umbra, Mat& penumbra)
{
	int height = img.rows;
	int width = img.cols;
	for (int i = 0; i<height; i++)
	{
		uchar* puImg = img.ptr(i);
		uchar* puUmbra = umbra.ptr(i);
		uchar* puPenumbra = penumbra.ptr(i);

		for (int j = 0; j<width; j++)
		{
			if (puUmbra[j] == 255) //blue represents umbra
			{
				puImg[3 * j] = puImg[3 * j] * 0.3 + 255 * 0.7;
			}
			if (puPenumbra[j] == 255)  //red presents penumbra  
			{
				puImg[3 * j + 2] = puImg[3 * j + 2] * 0.3 + 255 * 0.7;
			}
			 
		}
	}
}


 
/*******************************************
*Function: Handle umbra  
*Input: img,  3 channels
*       binary,   single channel
*       bg,    estimated bg images,  3 channels      
*       umbra ,    (255-shadow region,  0-unshadowed region)
*       penumbra,  (255-shadow region,  0-unshadowed region)
*Output:refLightRegionBgValue ,   the reference bg value in unshadowed region
        refLightRegionTextValue , the reference text value in unshadowed region
        refShadowRegionBgValue ,  the reference bg value in shadowed region
        refShadowRegionTextValue , the reference text value in shadowed region
        result, 3 channels image ,  the umbra has been removed 
*return：
*date: 2019.08.08
********************************************/
void HandleUmbra(Mat& img, Mat& binary,Mat& bg, Mat& umbra, Mat& penumbra, int refLightRegionBgValue[3], int refLightRegionTextValue[3], int refShadowRegionBgValue[3], int refShadowRegionTextValue[3],Mat& result)
{
	int height = img.rows;
	int width = img.cols;
	Mat shadingArea = umbra + penumbra;  //获取整个阴影区域
	//imshow("shadingArea", shadingArea);
	//imshow("umbra", umbra);

	double  sumLightRegionBg[3] = { 0 };  //统计非阴影区域整体亮度信息
	int numLightRegionBg=0;
	double sumLightRegionText[3] = { 0 }; //统计非阴影区域文本亮度信息
	int numLightRegionText = 0;

	double  sumShadowRegionBg[3] = { 0 }; //统计阴影区域整体亮度信息
	int numShadowRegionBg = 0;
	double  sumShadowRegionText[3] = { 0 }; //统计阴影区域文本亮度信息
	int numShadowRegionText = 0;

	for (int i = 0; i < height; i++)
	{
		uchar* puImg = img.ptr(i);
		uchar* puBg = bg.ptr(i);
		uchar* puShadingArea = shadingArea.ptr(i);
		uchar* puBinary = binary.ptr(i);
		uchar* puUmbra = umbra.ptr(i);

		for (int j = 0; j < width; j++)
		{
			if (puShadingArea[j] == 255)   //表示阴影区域
			{
				//阴影区域背景
				sumShadowRegionBg[0] += puBg[3 * j];
				sumShadowRegionBg[1] += puBg[3 * j+1];
				sumShadowRegionBg[2] += puBg[3 * j+2];
				numShadowRegionBg++;
				if (puUmbra[j] == 255 && puBinary[j] == 0)   //阴影区域文本
				{
					sumShadowRegionText[0] += puImg[3 * j];
					sumShadowRegionText[1] += puImg[3 * j + 1];
					sumShadowRegionText[2] += puImg[3 * j + 2];
					numShadowRegionText++;
				}
			}
			else   // 表示非阴影区域
			{
				//非阴影区域背景
				sumLightRegionBg[0] += puBg[3 * j];
				sumLightRegionBg[1] += puBg[3 * j+1];
				sumLightRegionBg[2] += puBg[3 * j+2];
				numLightRegionBg++;

				if (puBinary[j] == 0)  //非阴影区域文本
				{
					sumLightRegionText[0] += puImg[3 * j];
					sumLightRegionText[1] += puImg[3 * j+1];
					sumLightRegionText[2] += puImg[3 * j+2];
					numLightRegionText++;
				}
			}
		}
	}

    //计算总体亮暗比例  
	double overal_ratio[3] = { 0 };
	for (int k = 0; k<3; k++)
	{
		if (numLightRegionBg>0)
		{
			refLightRegionBgValue[k] = sumLightRegionBg[k] / numLightRegionBg;     //计算参考非阴影区域背景像素值
		}
		if (numLightRegionText>0)
		{
			refLightRegionTextValue[k] = sumLightRegionText[k] / numLightRegionText;  //计算参考非阴影区域文本像素值
		}
		if (numShadowRegionBg>0)
		{
			refShadowRegionBgValue[k] = sumShadowRegionBg[k] / numShadowRegionBg;
		}
		if (numShadowRegionText>0)
		{
			refShadowRegionTextValue[k] = sumShadowRegionText[k] / numShadowRegionText;   //参考阴影区域文本
		}
		
		overal_ratio[k] = 1.0*(refLightRegionBgValue[k] + 1) / (refShadowRegionBgValue[k] + 1);  //亮暗比例
	}
	//cout << "refLightRegionBgValue: " << refLightRegionBgValue[0] << " " << refLightRegionBgValue[1] << " " << refLightRegionBgValue[2] << endl;
	//cout << "refLightRegionTextValue: " << refLightRegionTextValue[0] << " " << refLightRegionTextValue[1] << " " << refLightRegionTextValue[2] << endl;
	//cout << "refShadowRegionBgValue: " << refShadowRegionBgValue[0] << " " << refShadowRegionBgValue[1] << " " << refShadowRegionBgValue[2] << endl;
	//cout << "refShadowRegionTextValue: " << refShadowRegionTextValue[0] << " " << refShadowRegionTextValue[1] << " " << refShadowRegionTextValue[2] << endl;
	
	cout << "Bg Ratio: " << 1.0*refLightRegionBgValue[0]/refShadowRegionBgValue[0] << " " << 1.0*refLightRegionBgValue[1]/refShadowRegionBgValue[1] << " " << 1.0*refLightRegionBgValue[2]/refShadowRegionBgValue[2] << endl;
	cout << "Text Ratio " << 1.0*refLightRegionTextValue[0]/refShadowRegionTextValue[0] << " " << 1.0*refLightRegionTextValue[1]/refShadowRegionTextValue[1] << " " << 1.0*refLightRegionTextValue[2]/refShadowRegionTextValue[2] << endl;




	img.copyTo(result);
	//去除本影 计算局部亮暗比例，以便于点亮阴影区域
	for (int i = 0; i < height; i++)
	{
		uchar* puImg = img.ptr(i);
		uchar* puBg = bg.ptr(i);
		uchar* puUmbra = umbra.ptr(i);
		uchar* puResult = result.ptr(i);
		for (int j = 0; j < width; j++)
		{
			if (puUmbra[j] == 255)
			{
				for (int k = 0; k<3; k++)
				{
					puResult[3 * j + k] = puImg[3 * j + k] * 1.0*refLightRegionBgValue[k] / (puBg[3 * j + k] + 1);
				}
			}
		}
	}

}



/*******************************************
*Function: Handle strong shadow and get final results
*Input: img,      3 channels
*       binary,   1 channel, (0-text , 255-bg)
*       wlBinary, 1 channel, (0-text , 255-bg), come from once-water filling result
*       shadowRegion, 1 channel, (255-shadow, 0-bg)
*       refLightRegionBgValue ,   the reference bg value in unshadowed region
*       refLightRegionTextValue , the reference text value in unshadowed region
*       refShadowRegionTextValue , the reference text value in shadowed region
*Output:result,   3 channels without shadows
*return：
*date: 2019.08.08
********************************************/
void GetStrongShadowEnhancedImg(Mat& img, Mat& binary, Mat& wlBinary,Mat& shadowRegion,int refLightRegionBgValue[3],int refLightRegionTextValue[3], int refShadowRegionTextValue[3], Mat& result)
{
	int height = img.rows;
	int width = img.cols;

	int size = 3;
	Mat element = getStructuringElement(MORPH_RECT, Size(2 * size + 1, 2 * size + 1), Point(size, size));
	dilate(shadowRegion, shadowRegion, element);

	//cout <<"   "<< refShadowRegionTextValue[0] << " " << refShadowRegionTextValue[1] << " " << refShadowRegionTextValue[2] << endl;
	double ratio[3] = {1.5};
	double ratioTemp;
	for (int i=0;i<3;i++)
	{
		if (refShadowRegionTextValue[i] > 0)
		{
			ratioTemp = refLightRegionTextValue[i] / refShadowRegionTextValue[i];
			ratioTemp = ratioTemp*1.2;
			ratio[i] = MAX(ratio[i], ratioTemp);
		}
	}

	for (int i=0;i<height;i++)
	{
		uchar* puImg = img.ptr(i);
		uchar* puBinary= binary.ptr(i);
		uchar* puResult= result.ptr(i);
		uchar* puShadowRegion = shadowRegion.ptr(i);
		for (int j=0;j<width;j++)
		{
			if (puBinary[j]==255) //整幅图像非文字区域设置为统一的参考背景值
			{
				puResult[3 * j] = refLightRegionBgValue[0];
				puResult[3 * j+1] = refLightRegionBgValue[1];
				puResult[3 * j+2] = refLightRegionBgValue[2];
			}
			else  //整幅图像文字区域
			{
				puResult[3 * j] = puImg[3 * j];
				puResult[3 * j + 1] = puImg[3 * j + 1];
				puResult[3 * j + 2] = puImg[3 * j + 2];

				if (puShadowRegion[j] == 255) 
				{  	//阴影区域紧邻文字的残余半影 	
					if (puImg[3 * j] > refShadowRegionTextValue[0] * ratio[0]
						&& puImg[3 * j + 1] > refShadowRegionTextValue[1] * ratio[1]
						&& puImg[3 * j + 2] > refShadowRegionTextValue[2] * ratio[2])
					{
						puResult[3 * j] = refLightRegionBgValue[0];
						puResult[3 * j + 1] = refLightRegionBgValue[1];
						puResult[3 * j + 2] = refLightRegionBgValue[2];
					}
					else  //阴影区域的文字信息  
					{
						puResult[3 * j] = puImg[3 * j ]*0.5 + refLightRegionTextValue[0]*0.5;
						puResult[3 * j + 1] = puImg[3 * j + 1] * 0.5 + refLightRegionTextValue[1]*0.5;
						puResult[3 * j + 2] = puImg[3 * j + 2]*0.5 + refLightRegionTextValue[2]*0.5;
 
					}
				}
				else  //非阴影区的文本采用原图给的值
				{
					puResult[3 * j] = puImg[3 * j];
					puResult[3 * j + 1] = puImg[3 * j + 1];
					puResult[3 * j + 2] = puImg[3 * j + 2];
				}

			}
		}
	}


	//依据binary    wlBinary  并联合img result 获得最后结果
	size = 2;
	element = getStructuringElement(MORPH_RECT, Size(2 * size + 1, 2 * size + 1), Point(size, size));
	erode(wlBinary, wlBinary, element);
	for (int i = 0; i<height; i++)
	{
		uchar* puBinary = binary.ptr(i);
		uchar* puWLBinary = wlBinary.ptr(i);
		uchar* puImg = img.ptr(i);
		uchar* puResult = result.ptr(i);

		for (int j = 0; j<width; j++)
		{
			if (puWLBinary[j] == 255 && puBinary[j] == 0)
			{
				puResult[3 * j] = refLightRegionBgValue[0];
				puResult[3 * j + 1] = refLightRegionBgValue[1];
				puResult[3 * j + 2] = refLightRegionBgValue[2];
				puBinary[j] = 255;
			}
		}
	}

}


/*******************************************
*Function: Handle medium shadow and get final results        
*Input: img,      3 channels
*       binary,   1 channel, (0-text , 255-bg)
*       refLightRegionBgValue, reference background values
*Output:result,   3 channels without shadows
*return：
*date: 2019.08.08
********************************************/
void GetMediumShadowEnhancedResult(Mat& img,Mat& binary,int refLightRegionBgValue[3],Mat& result)
{
	img.copyTo(result);
	int height = img.rows;
	int width = img.cols;
	for (int i = 0; i<height; i++)
	{
		uchar* puBinary = binary.ptr(i);
		uchar* puResult = result.ptr(i);
		for (int j = 0; j<width; j++)
		{
			if (puBinary[j] >50)
			{
				puResult[3 * j] = refLightRegionBgValue[0];
				puResult[3 * j + 1] = refLightRegionBgValue[1];
				puResult[3 * j + 2] = refLightRegionBgValue[2];
			}
			/*else
			{
				puResult[3 * j] = puBinary[j];
				puResult[3 * j + 1] = puBinary[j];
				puResult[3 * j + 2] = puBinary[j];
			}*/
		}
	}
}


/*******************************************
*Function: Get colored images that label umbra (blue) and penumbra (red), respectively.
*Input: img,      3 channels
*Output:result,   3 channels without shadows
*return：
*date: 2019.08.08
********************************************/
void DocumentShadowRemoval(Mat& img, Mat& result)
{
	Mat bg(img.size(), CV_8UC3, Scalar(0, 0, 0));
	Mat gray(img.size(), CV_8UC1, Scalar(0));
	Mat binary(img.size(), CV_8UC1, Scalar(0));
	Mat wlBinary(img.size(), CV_8UC1, Scalar(0));  //water_filling 得到的二值化图
	Mat umbra(img.size(), CV_8UC1, Scalar(0));
	Mat penumbra(img.size(), CV_8UC1, Scalar(0));

	Mat imgTemp(img.size(), CV_8UC3, Scalar(0, 0, 0));
	img.copyTo(imgTemp);

	cvtColor(img, gray, COLOR_BGR2GRAY);
	ThresholdIntegral(gray, 0.95, binary);   //原图进行二值化
	//imshow("binary", binary);
	//Rect rc(428, 160, 140, 130);  // (428, 160, 140, 130)

	//Mat smllImg = img(rc);
	//imwrite("smll.jpg", smllImg);

	


	GetRefBg(img, bg);   //基于Water_Filling获得图像参考背景图
	//imshow("bg", bg);

	//Mat bgImg = bg(rc);
	//imwrite("bg.jpg", bgImg);

	GetUmbraAndPenumbra(bg, umbra, penumbra);   //计算本影和半影区域

	//获取阴影区域图像和非阴影区域图像，最多分成两个区域来进行处理
	GetColorImg(img, umbra, penumbra);   //获取本影半影区域颜色表示图
	Mat shadowArea = umbra + penumbra;
	//Mat umPenumImg = shadowArea(rc);
	//imwrite("shadowArea.jpg", umPenumImg);

	//Mat colorImg = img(rc);
	//imwrite("color.jpg", colorImg);
	//imshow("bg",bg );
	imgTemp.copyTo(img);	

	//处理本影  并获得参考背景值
	int refLightRegionBgValue[3] = { 255 };
	int refLightRegionTextValue[3] = { 0 };
	int refShadowRegionBgValue[3] = { 255 };
	int refShadowRegionTextValue[3] = { 0 };

	HandleUmbra(img,binary, bg, umbra, penumbra, refLightRegionBgValue, refLightRegionTextValue, refShadowRegionBgValue, refShadowRegionTextValue, result);
	//cout << refBgValue[0] << " " << refBgValue[1] << " " << refBgValue[2] << endl;
	
	//Mat removedUmbraImg = result(rc);
	//imwrite("removedUmbraImg.jpg", removedUmbraImg);

	//imshow("bg", bg);
	result.copyTo(img);
	cvtColor(img, gray, COLOR_BGR2GRAY);
	ThresholdIntegral(gray, 0.95, binary);   //处理完本影后的图像进行二值化
	//Mat binImg = binary(rc);
	//imwrite("binary.jpg", binImg);
											 
											 //imshow("binary", binary);
	//处理半影
	GetWordLevelMask(img, wlBinary);   //获取word-level级别的二值化图
	//imshow("World_level", wlBinary);

	//Mat wordBinImg = wlBinary(rc);
	//imwrite("wordBinImg.jpg", wordBinImg);

	HandlePenumbra(binary, wlBinary, penumbra);   //获得消除阴影的二值化图
	//imshow("binary", binary);

	//Mat finalBinImg = binary(rc);
	//imwrite("finalBinImg.jpg", finalBinImg);


	//考虑基于阴影的强度不同选择不同的参数进行处理
	double ratioBg[3] = { 0 };
	for (int i = 0; i<3; i++)
	{
		ratioBg[i] = 1.0*refLightRegionBgValue[i] / refShadowRegionBgValue[i];
	}
	double avgRatioBg = (ratioBg[0] + ratioBg[1] + ratioBg[2]) / 3;
	imgTemp.copyTo(img);

	if (avgRatioBg<1.6)
	{
		GetMediumShadowEnhancedResult(img, binary, refLightRegionBgValue, result);
	}
	else 
	{
		//基于参考背景，获得最终的结果图
		Mat shadowRegion = umbra + penumbra;
		GetStrongShadowEnhancedImg(img, binary, wlBinary, shadowRegion, refLightRegionBgValue, refLightRegionTextValue, refShadowRegionTextValue, result);
	}

	//Mat resultImg = result(rc);
	//imwrite("resultImg.jpg", resultImg);

}

