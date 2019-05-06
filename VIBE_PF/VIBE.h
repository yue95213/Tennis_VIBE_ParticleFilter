///////////////////////////////////////////////////
/////////////////VIBE相关参数定义//////////////////
///////////////////////////////////////////////////

#include "stdafx.h"


#define NUM_SAMPLES 20		//每个像素点的样本个数
#define MIN_MATCHES 2		//#min指数
#define RADIUS 20		//Sqthere半径
#define SUBSAMPLE_FACTOR 16	//子采样概率

extern Mat mask;


class ViBe_BGS
{
public:
	ViBe_BGS(void);
	~ViBe_BGS(void);

	void init(const Mat _image);   //初始化
	void processFirstFrame(const Mat _image);
	void testAndUpdate(const Mat _image);  //更新
	Mat getMask(void){ return m_mask; };
	void deleteSamples(){ delete samples; };
	void modify(const Mat _image);
	void modify2();

private:
	//unsigned char ***samples;//￥三级指针，指向一个三维数组
	unsigned char ****samples;
	//	float samples[1024][1024][NUM_SAMPLES+1];//保存每个像素点的样本值

	/*
	Mat m_samples[NUM_SAMPLES];
	Mat m_foregroundMatchCount;*/

	Mat m_mask;
};