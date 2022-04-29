#pragma once
#ifndef _HARD_SHADOW_REMOVAL_H
#define _HARD_SHADOW_REMOVAL_H
#include"common.h"
#include"basic.h"

/*******************************************
*Function: Inv_Relu£¬ inverse
*Input: input_
*Output:
*return£ºfloat value
*date: 2019.08.08
********************************************/
float Inv_Relu(float input_);
 

/*******************************************
*Function: Local water filling, get referance background
*Input: gray,   single channel image
*       size,   the local neighborhood, (2*size+1)*(2*size+1)
*Output:estimateBg,    the shading map
*return£º
*date: 2019.08.08
********************************************/
void Local_Water_Filling(Mat& gray, int size, Mat& estimateBg);

/*******************************************
*Function: Based on local water filling method to
*          estimate the background.
*Input: img,   three channels image
*Output:bg,    the estimated bg, three channels
*return£º
*date: 2019.08.08
********************************************/
void GetRefBg(Mat& img, Mat& bg);


/*******************************************
*Function: once water filling method to   get gray image,
*          text-(0,255)  background-0
*Input: img,   three channels image
*       size,  the local neighborhood, (2*size+1)*(2*size+1)
*Output: estimateTextImg,     single channel
*return£º
*date: 2019.08.08
********************************************/
void Once_Water_Filling(Mat& gray, int size, Mat& estimateTextImg);


/*******************************************
*Function: Based on  once water filling method to
*          get binarization image,  text-0  background-255
*Input: img,   three channels image
*Output:binary,     single channel
*return£º
*date: 2019.08.08
********************************************/
void GetWordLevelMask(Mat& img, Mat& binary);

/*******************************************
*Function: Handle Penumbra by the operation between  binary and wlBinary
*Input: binary, wlBinary       single channel
*       penumbra        single channel
*Output:binary ,    (0-shadow ,  255-bg)
*return£º
*date: 2019.08.08
********************************************/
void HandlePenumbra(Mat& binary, Mat& wlBinary, Mat& penumbra);

/*******************************************
*Function: Get umbra and penumbra images from the estimated background image
*Input: bg,    estimated background image with 3 channels
*Output:umbra ,    (255-shadow region,  0-unshadowed region)
*       penumbra,  (255-shadow region,  0-unshadowed region)
*return£º
*date: 2019.08.08
********************************************/
void GetUmbraAndPenumbra(Mat& bg, Mat& umbra, Mat& penumbra);


/*******************************************
*Function: Get colored images that label umbra (blue) and penumbra (red), respectively.
*Input: img,     3 channels
*       umbra ,    (255-shadow region,  0-unshadowed region)
*       penumbra,  (255-shadow region,  0-unshadowed region)
*Output:img, colored
*return£º
*date: 2019.08.08
********************************************/
void GetColorImg(Mat& img, Mat& umbra, Mat& penumbra);



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
*return£º
*date: 2019.08.08
********************************************/
void HandleUmbra(Mat& img, Mat& binary, Mat& bg, Mat& umbra, Mat& penumbra, int refLightRegionBgValue[3], int refLightRegionTextValue[3], int refShadowRegionBgValue[3], int refShadowRegionTextValue[3], Mat& result);

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
*return£º
*date: 2019.08.08
********************************************/
void GetStrongShadowEnhancedImg(Mat& img, Mat& binary, Mat& wlBinary, Mat& shadowRegion, int refLightRegionBgValue[3], int refLightRegionTextValue[3], int refShadowRegionTextValue[3], Mat& result);


/*******************************************
*Function: Handle medium shadow and get final results
*Input: img,      3 channels
*       binary,   1 channel, (0-text , 255-bg)
*       refLightRegionBgValue, reference background values
*Output:result,   3 channels without shadows
*return£º
*date: 2019.08.08
********************************************/
void GetMediumShadowEnhancedResult(Mat& img, Mat& binary, int refLightRegionBgValue[3], Mat& result);


/*******************************************
*Function: Get colored images that label umbra (blue) and penumbra (red), respectively.
*Input: img,      3 channels
*Output:result,   3 channels without shadows
*return£º
*date: 2019.08.08
********************************************/
void DocumentShadowRemoval(Mat& img, Mat& result);


#endif