///////////////////////////////////////////////////
/////////////////VIBE��ز�������//////////////////
///////////////////////////////////////////////////

#include "stdafx.h"


#define NUM_SAMPLES 20		//ÿ�����ص����������
#define MIN_MATCHES 2		//#minָ��
#define RADIUS 20		//Sqthere�뾶
#define SUBSAMPLE_FACTOR 16	//�Ӳ�������

extern Mat mask;


class ViBe_BGS
{
public:
	ViBe_BGS(void);
	~ViBe_BGS(void);

	void init(const Mat _image);   //��ʼ��
	void processFirstFrame(const Mat _image);
	void testAndUpdate(const Mat _image);  //����
	Mat getMask(void){ return m_mask; };
	void deleteSamples(){ delete samples; };
	void modify(const Mat _image);
	void modify2();

private:
	//unsigned char ***samples;//������ָ�룬ָ��һ����ά����
	unsigned char ****samples;
	//	float samples[1024][1024][NUM_SAMPLES+1];//����ÿ�����ص������ֵ

	/*
	Mat m_samples[NUM_SAMPLES];
	Mat m_foregroundMatchCount;*/

	Mat m_mask;
};