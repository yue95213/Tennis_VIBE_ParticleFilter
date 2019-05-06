#include "particles.h"
#include "min_rect.h"


/****hsv空间用到的变量****/
int TRACK::hist_size[3] = { 16, 16, 16 };
float TRACK::hrange[2] = { 0, 180.0 };
float TRACK::srange[2] = { 0, 256.0 };
float TRACK::vrange[2] = { 0, 256.0 };
const float *ranges[] = { TRACK::hrange, TRACK::srange, TRACK::vrange };//二维指针

int TRACK::channels[3] = { 0, 1, 2 };//颜色空间3个通道

/****有关粒子窗口变化用到的相关变量****/
int A1 = 2;
int A2 = -1;
int B0 = 1;
double sigmax = 1.0;
double sigmay = 0.5;
double sigmas = 0.001;

/****定义粒子权重和****/
double particle_sum[CONTOURS_NUMBER] = { 0.0 };

TRACK::TRACK(void)
{

}

TRACK::~TRACK(void)
{

}

/************************ 计算hist ***************************/
void TRACK::hist(int num)
{

	/****计算目标模板的直方图特征****/
	target_img = Mat(current_hsv, min_rects[num]);//在此之前先定义好target_img,然后这样赋值也行，要学会Mat的这个操作
	calcHist(&target_img, 1, channels, Mat(), target_hist, 3, hist_size, ranges);
	//calcHist:计算直方图的函数
	//输入图像为target_img,输入图像个数为1个，通道为颜色空间，Mat()为掩码（？）
	//计算出的直方图为target_hist,维数为3，每维直方图上个数为hist_size
	//ranges为统计的范围
	normalize(target_hist, target_hist);//对直方图归一化

}

/************************ 初始化粒子参数 ***************************/
void TRACK::init(PARTICLE *pParticle, int num)
{
	/****初始化目标粒子****/
	for (int j = 0; j < PARTICLE_NUMBER; j++)
	{
		//初始化所有粒子位置为选定矩形框中心
		//选定目标矩形框中心为初始粒子窗口中心
		pParticle->x = cvRound(min_rects[num].x + 0.5*min_rects[num].width);//cvRound：四舍五入
		pParticle->y = cvRound(min_rects[num].y + 0.5*min_rects[num].height);
		pParticle->orix = pParticle->x;//粒子的原始坐标为选定矩形框(即目标)的中心
		pParticle->oriy = pParticle->y;
		pParticle->prex = pParticle->x;//更新上一次的粒子位置
		pParticle->prey = pParticle->y;
		pParticle->rect = min_rects[num];
		pParticle->prescale = 1;
		pParticle->scale = 1;
		pParticle->hist = target_hist;
		pParticle->weight = 0;
		pParticle++;
	}

}

/************************ 更新粒子参数 ***************************/
void TRACK::update(particle *pParticle,int num)
{
	RNG rng;//随机数产生器
	particle_sum[num] = 0;//更新前清除上一次权值计算结果

	/****更新粒子结构体的大部分参数****/

	for (int j=0; j < PARTICLE_NUMBER; j++)
	{
		int x, y;
		int xpre, ypre;
		double s, pres;

		xpre = pParticle->x;
		ypre = pParticle->y;
		pres = pParticle->scale;

		/****更新粒子的矩形区域即粒子中心****/
		x = cvRound(A1*(pParticle->x - pParticle->orix) + A2*(pParticle->prex - pParticle->orix) +
			B0*rng.gaussian(sigmax) + pParticle->orix);//高斯分布粒子
		pParticle->x = max(0, min(x, current_frame.cols - 1));//？？？？？

		y = cvRound(A1*(pParticle->y - pParticle->oriy) + A2*(pParticle->prey - pParticle->oriy) +
			B0*rng.gaussian(sigmay) + pParticle->oriy);
		pParticle->y = max(0, min(y, current_frame.rows - 1));

		s = A1*(pParticle->scale - 1) + A2*(pParticle->prescale - 1) + B0*(rng.gaussian(sigmas)) + 1.0;
		pParticle->scale = max(1.0, min(s, 3.0));

		pParticle->prex = xpre;
		pParticle->prey = ypre;
		pParticle->prescale = pres;
		//        pParticle->orix=pParticle->orix;
		//        pParticle->oriy=pParticle->oriy;

		//注意在c语言中，x-1.0，如果x是int型，则这句语法有错误,但如果前面加了cvRound(x-0.5)则是正确的
		pParticle->rect.x = max(0, min(cvRound(pParticle->x - 0.5*pParticle->scale*pParticle->rect.width), current_frame.cols));
		pParticle->rect.y = max(0, min(cvRound(pParticle->y - 0.5*pParticle->scale*pParticle->rect.height), current_frame.rows));
		pParticle->rect.width = min(cvRound(pParticle->rect.width), current_frame.cols - pParticle->rect.x);
		pParticle->rect.height = min(cvRound(pParticle->rect.height), current_frame.rows - pParticle->rect.y);
		//        pParticle->rect.width=min(cvRound(pParticle->scale*pParticle->rect.width),frame.cols-pParticle->rect.x);
		//        pParticle->rect.height=min(cvRound(pParticle->scale*pParticle->rect.height),frame.rows-pParticle->rect.y);

		/****计算粒子区域的新的直方图特征****/
		track_img = Mat(current_hsv, pParticle->rect);
		calcHist(&track_img, 1, channels, Mat(), track_hist, 3, hist_size, ranges);
		normalize(track_hist, track_hist);

		/****更新粒子的权值****/
		//        pParticle->weight=compareHist(target_hist,track_hist,CV_COMP_INTERSECT);
		//采用巴氏系数计算相似度,永远与最开始的那一目标帧相比较
		pParticle->weight = 1.0 - compareHist(target_hist, track_hist, CV_COMP_BHATTACHARYYA);
		/****累加粒子权值****/
		particle_sum[num] += pParticle->weight;
		pParticle++;
	}

}

/************************ 归一化粒子权重 ***************************/
void TRACK::weight(particle *pParticle,int num)
{
	for (int j = 0; j < PARTICLE_NUMBER; j++)
	{
		pParticle->weight /= particle_sum[num];
		pParticle++;
	}

}

/************************ 重采样 ***************************/
void TRACK::reparticle(particle *pParticle, particle *particles)
{
	/****根据粒子权重重采样粒子****/
	PARTICLE newParticle[PARTICLE_NUMBER];
	int np = 0, k = 0;
	for (int i = 0; i < PARTICLE_NUMBER; i++)
	{
		np = cvRound(pParticle[i].weight*PARTICLE_NUMBER);
		for (int j = 0; j < np; j++)///////////////?????????????????<=??????????
		{
			newParticle[k++] = *(particles + i);
			if (k == PARTICLE_NUMBER)
				goto EXITOUT;
		}
	}
	while (k < PARTICLE_NUMBER)
		newParticle[k++] = *pParticle;
EXITOUT:
	for (int i = 0; i < PARTICLE_NUMBER; i++)
		pParticle[i] = newParticle[i];
}

/************************ 计算并显示跟踪结果 ***************************/
//void TRACK::result(particle *pParticle)
//{
//
//	/****计算粒子期望，采用所有粒子位置的期望值做为跟踪结果****/
//	/*Rect_<double> rectTrackingTemp(0.0,0.0,0.0,0.0);
//	pParticle=particles;
//	for(int i=0;i<PARTICLE_NUMBER;i++)
//	{
//	rectTrackingTemp.x+=pParticle->rect.x*pParticle->weight;
//	rectTrackingTemp.y+=pParticle->rect.y*pParticle->weight;
//	rectTrackingTemp.width+=pParticle->rect.width*pParticle->weight;
//	rectTrackingTemp.height+=pParticle->rect.height*pParticle->weight;
//	pParticle++;
//	}*/
//
//
//	/****计算最大权重目标的期望位置，作为跟踪结果****/
//	Rect rectTrackingTemp(0, 0, 0, 0);
//	rectTrackingTemp.x = pParticle->x - 0.5*pParticle->rect.width;
//	rectTrackingTemp.y = pParticle->y - 0.5*pParticle->rect.height;
//	rectTrackingTemp.width = pParticle->rect.width;
//	rectTrackingTemp.height = pParticle->rect.height;
//
//
//
//	/****计算最大权重目标的期望位置，采用权值最大的1/4个粒子数作为跟踪结果****/
//	/*Rect rectTrackingTemp(0,0,0,0);
//	double weight_temp=0.0;
//	pParticle=particles;
//	for(int i=0;i<PARTICLE_NUMBER/4;i++)
//	{
//	weight_temp+=pParticle->weight;
//	pParticle++;
//	}
//	pParticle=particles;
//	for(int i=0;i<PARTICLE_NUMBER/4;i++)
//	{
//	pParticle->weight/=weight_temp;
//	pParticle++;
//	}
//	pParticle=particles;
//	for(int i=0;i<PARTICLE_NUMBER/4;i++)
//	{
//	rectTrackingTemp.x+=pParticle->rect.x*pParticle->weight;
//	rectTrackingTemp.y+=pParticle->rect.y*pParticle->weight;
//	rectTrackingTemp.width+=pParticle->rect.width*pParticle->weight;
//	rectTrackingTemp.height+=pParticle->rect.height*pParticle->weight;
//	pParticle++;
//	}*/
//
//
//	/****计算最大权重目标的期望位置，采用所有粒子数作为跟踪结果****/
//	/*Rect rectTrackingTemp(0,0,0,0);
//	pParticle=particles;
//	for(int i=0;i<PARTICLE_NUMBER;i++)
//	{
//	rectTrackingTemp.x+=cvRound(pParticle->rect.x*pParticle->weight);
//	rectTrackingTemp.y+=cvRound(pParticle->rect.y*pParticle->weight);
//	pParticle++;
//	}
//	pParticle=particles;
//	rectTrackingTemp.width = pParticle->rect.width;
//	rectTrackingTemp.height = pParticle->rect.height;*/
//
//	//创建目标矩形区域
//	Rect tracking_rect(rectTrackingTemp);
//
//	/****显示各粒子运动结果****/
//	for (int m = 0; m<PARTICLE_NUMBER; m++)
//	{
//		//rectangle(frame, pParticle->rect, Scalar(255, 0, 0), 1, 8, 0);
//		//pParticle++;
//
//		int x0, y0, x1, y1, x2, y2;
//		CvScalar color;
//		color = CV_RGB(255, 255, 0);
//		x0 = cvRound(pParticle->x - 0.5 * pParticle->scale*pParticle->rect.width);
//		y0 = cvRound(pParticle->y - 0.5 * pParticle->scale*pParticle->rect.height);
//		x1 = cvRound(pParticle->x + 0.5 * pParticle->scale*pParticle->rect.width);
//		y1 = cvRound(pParticle->y + 0.5 * pParticle->scale*pParticle->rect.height);
//		x2 = cvRound(pParticle->x);
//		y2 = cvRound(pParticle->y);
//		//rectangle(frame, cvPoint(x0, y0), cvPoint(x1, y1), color, 1, 8, 0);
//		circle(frame, cvPoint(x2, y2), 3, color, -1, 8, 0);
//		pParticle++;
//	}
//
//	/****显示跟踪结果****/
//	rectangle(frame, tracking_rect, Scalar(0, 0, 255), 3, 8, 0);
//
//	after_select_frames++;//总循环每循环一次，计数加1
//	if (after_select_frames>2)//防止跟踪太长，after_select_frames计数溢出
//		after_select_frames = 2;
//}
