///////////////////////////////////////////////////
//////////////定义粒子结构体particle///////////////
///////////////////定义类 TRACK////////////////////
///////////////////////////////////////////////////

#include "stdafx.h"

extern int after_select_frames;//选择矩形区域完后的帧计数
extern Mat current_frame,current_hsv;//定义当前帧图像和hsv空间的声明

/****定义使用粒子数目宏****/
#define PARTICLE_NUMBER 100 

/****定义粒子结构体****/
typedef struct particle
{
	int orix, oriy;//原始粒子坐标
	int x, y;//当前粒子的坐标
	double scale;//当前粒子窗口的尺寸
	int prex, prey;//上一帧粒子的坐标
	double prescale;//上一帧粒子窗口的尺寸
	Rect rect;//当前粒子矩形窗口
	Mat hist;//当前粒子窗口直方图特征
	double weight;//当前粒子权值
}PARTICLE;

class TRACK
{
public:
	TRACK(void);
	~TRACK(void);

	void hist(int num);//计算选中矩形区域直方图
	void init(PARTICLE *pParticle,int num);//初始化粒子参数
	void update(PARTICLE *pParticle,int num);//更新粒子参数
	void weight(PARTICLE *pParticle,int num);//归一化粒子权重
	void reparticle(PARTICLE *pParticle, particle *particles);//重采样
	void result(PARTICLE *pParticle);//计算并显示跟踪结果
	void show(PARTICLE *pParticle);//显示跟踪过程

public :
	
	Mat target_img, target_hist;//初始选取矩形区域的直方图
	Mat track_img, track_hist;//当前区域直方图
	//Mat target_img[CONTOURS_NUMBER], target_hist[CONTOURS_NUMBER];
	//Mat track_img[CONTOURS_NUMBER], track_hist[CONTOURS_NUMBER];

	Rect select;//（矩形选择框）
	bool select_flag=false;
	bool tracking=false;//跟踪标志位
	bool select_show=false;
	Point origin;

	/****rgb空间用到的变量****/
	//int hist_size[]={16,16,16};//rgb空间各维度的bin个数
	//float rrange[]={0,255.0};
	//float grange[]={0,255.0};
	//float brange[]={0,255.0};
	//const float *ranges[] ={rrange,grange,brange};//range相当于一个二维数组指针

	/****hsv空间用到的变量****/
	static int hist_size[3];
	static float hrange[2];
	static float srange[2];
	static float vrange[2];

	//int hist_size[]={32,32,32};
	//float hrange[]={0,359.0.0};
	//float srange[]={0,1.0};
	//float vrange[]={0,1.0};
	//static float *ranges[];//二维指针

	static int channels[3];//颜色空间3个通道
};
