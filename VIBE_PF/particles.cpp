#include "particles.h"
#include "min_rect.h"


/****hsv�ռ��õ��ı���****/
int TRACK::hist_size[3] = { 16, 16, 16 };
float TRACK::hrange[2] = { 0, 180.0 };
float TRACK::srange[2] = { 0, 256.0 };
float TRACK::vrange[2] = { 0, 256.0 };
const float *ranges[] = { TRACK::hrange, TRACK::srange, TRACK::vrange };//��άָ��

int TRACK::channels[3] = { 0, 1, 2 };//��ɫ�ռ�3��ͨ��

/****�й����Ӵ��ڱ仯�õ�����ر���****/
int A1 = 2;
int A2 = -1;
int B0 = 1;
double sigmax = 1.0;
double sigmay = 0.5;
double sigmas = 0.001;

/****��������Ȩ�غ�****/
double particle_sum[CONTOURS_NUMBER] = { 0.0 };

TRACK::TRACK(void)
{

}

TRACK::~TRACK(void)
{

}

/************************ ����hist ***************************/
void TRACK::hist(int num)
{

	/****����Ŀ��ģ���ֱ��ͼ����****/
	target_img = Mat(current_hsv, min_rects[num]);//�ڴ�֮ǰ�ȶ����target_img,Ȼ��������ֵҲ�У�Ҫѧ��Mat���������
	calcHist(&target_img, 1, channels, Mat(), target_hist, 3, hist_size, ranges);
	//calcHist:����ֱ��ͼ�ĺ���
	//����ͼ��Ϊtarget_img,����ͼ�����Ϊ1����ͨ��Ϊ��ɫ�ռ䣬Mat()Ϊ���루����
	//�������ֱ��ͼΪtarget_hist,ά��Ϊ3��ÿάֱ��ͼ�ϸ���Ϊhist_size
	//rangesΪͳ�Ƶķ�Χ
	normalize(target_hist, target_hist);//��ֱ��ͼ��һ��

}

/************************ ��ʼ�����Ӳ��� ***************************/
void TRACK::init(PARTICLE *pParticle, int num)
{
	/****��ʼ��Ŀ������****/
	for (int j = 0; j < PARTICLE_NUMBER; j++)
	{
		//��ʼ����������λ��Ϊѡ�����ο�����
		//ѡ��Ŀ����ο�����Ϊ��ʼ���Ӵ�������
		pParticle->x = cvRound(min_rects[num].x + 0.5*min_rects[num].width);//cvRound����������
		pParticle->y = cvRound(min_rects[num].y + 0.5*min_rects[num].height);
		pParticle->orix = pParticle->x;//���ӵ�ԭʼ����Ϊѡ�����ο�(��Ŀ��)������
		pParticle->oriy = pParticle->y;
		pParticle->prex = pParticle->x;//������һ�ε�����λ��
		pParticle->prey = pParticle->y;
		pParticle->rect = min_rects[num];
		pParticle->prescale = 1;
		pParticle->scale = 1;
		pParticle->hist = target_hist;
		pParticle->weight = 0;
		pParticle++;
	}

}

/************************ �������Ӳ��� ***************************/
void TRACK::update(particle *pParticle,int num)
{
	RNG rng;//�����������
	particle_sum[num] = 0;//����ǰ�����һ��Ȩֵ������

	/****�������ӽṹ��Ĵ󲿷ֲ���****/

	for (int j=0; j < PARTICLE_NUMBER; j++)
	{
		int x, y;
		int xpre, ypre;
		double s, pres;

		xpre = pParticle->x;
		ypre = pParticle->y;
		pres = pParticle->scale;

		/****�������ӵľ���������������****/
		x = cvRound(A1*(pParticle->x - pParticle->orix) + A2*(pParticle->prex - pParticle->orix) +
			B0*rng.gaussian(sigmax) + pParticle->orix);//��˹�ֲ�����
		pParticle->x = max(0, min(x, current_frame.cols - 1));//����������

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

		//ע����c�����У�x-1.0�����x��int�ͣ�������﷨�д���,�����ǰ�����cvRound(x-0.5)������ȷ��
		pParticle->rect.x = max(0, min(cvRound(pParticle->x - 0.5*pParticle->scale*pParticle->rect.width), current_frame.cols));
		pParticle->rect.y = max(0, min(cvRound(pParticle->y - 0.5*pParticle->scale*pParticle->rect.height), current_frame.rows));
		pParticle->rect.width = min(cvRound(pParticle->rect.width), current_frame.cols - pParticle->rect.x);
		pParticle->rect.height = min(cvRound(pParticle->rect.height), current_frame.rows - pParticle->rect.y);
		//        pParticle->rect.width=min(cvRound(pParticle->scale*pParticle->rect.width),frame.cols-pParticle->rect.x);
		//        pParticle->rect.height=min(cvRound(pParticle->scale*pParticle->rect.height),frame.rows-pParticle->rect.y);

		/****��������������µ�ֱ��ͼ����****/
		track_img = Mat(current_hsv, pParticle->rect);
		calcHist(&track_img, 1, channels, Mat(), track_hist, 3, hist_size, ranges);
		normalize(track_hist, track_hist);

		/****�������ӵ�Ȩֵ****/
		//        pParticle->weight=compareHist(target_hist,track_hist,CV_COMP_INTERSECT);
		//���ð���ϵ���������ƶ�,��Զ���ʼ����һĿ��֡��Ƚ�
		pParticle->weight = 1.0 - compareHist(target_hist, track_hist, CV_COMP_BHATTACHARYYA);
		/****�ۼ�����Ȩֵ****/
		particle_sum[num] += pParticle->weight;
		pParticle++;
	}

}

/************************ ��һ������Ȩ�� ***************************/
void TRACK::weight(particle *pParticle,int num)
{
	for (int j = 0; j < PARTICLE_NUMBER; j++)
	{
		pParticle->weight /= particle_sum[num];
		pParticle++;
	}

}

/************************ �ز��� ***************************/
void TRACK::reparticle(particle *pParticle, particle *particles)
{
	/****��������Ȩ���ز�������****/
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

/************************ ���㲢��ʾ���ٽ�� ***************************/
//void TRACK::result(particle *pParticle)
//{
//
//	/****��������������������������λ�õ�����ֵ��Ϊ���ٽ��****/
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
//	/****�������Ȩ��Ŀ�������λ�ã���Ϊ���ٽ��****/
//	Rect rectTrackingTemp(0, 0, 0, 0);
//	rectTrackingTemp.x = pParticle->x - 0.5*pParticle->rect.width;
//	rectTrackingTemp.y = pParticle->y - 0.5*pParticle->rect.height;
//	rectTrackingTemp.width = pParticle->rect.width;
//	rectTrackingTemp.height = pParticle->rect.height;
//
//
//
//	/****�������Ȩ��Ŀ�������λ�ã�����Ȩֵ����1/4����������Ϊ���ٽ��****/
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
//	/****�������Ȩ��Ŀ�������λ�ã�����������������Ϊ���ٽ��****/
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
//	//����Ŀ���������
//	Rect tracking_rect(rectTrackingTemp);
//
//	/****��ʾ�������˶����****/
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
//	/****��ʾ���ٽ��****/
//	rectangle(frame, tracking_rect, Scalar(0, 0, 255), 3, 8, 0);
//
//	after_select_frames++;//��ѭ��ÿѭ��һ�Σ�������1
//	if (after_select_frames>2)//��ֹ����̫����after_select_frames�������
//		after_select_frames = 2;
//}
