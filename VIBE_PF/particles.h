///////////////////////////////////////////////////
//////////////�������ӽṹ��particle///////////////
///////////////////������ TRACK////////////////////
///////////////////////////////////////////////////

#include "stdafx.h"

extern int after_select_frames;//ѡ�������������֡����
extern Mat current_frame,current_hsv;//���嵱ǰ֡ͼ���hsv�ռ������

/****����ʹ��������Ŀ��****/
#define PARTICLE_NUMBER 100 

/****�������ӽṹ��****/
typedef struct particle
{
	int orix, oriy;//ԭʼ��������
	int x, y;//��ǰ���ӵ�����
	double scale;//��ǰ���Ӵ��ڵĳߴ�
	int prex, prey;//��һ֡���ӵ�����
	double prescale;//��һ֡���Ӵ��ڵĳߴ�
	Rect rect;//��ǰ���Ӿ��δ���
	Mat hist;//��ǰ���Ӵ���ֱ��ͼ����
	double weight;//��ǰ����Ȩֵ
}PARTICLE;

class TRACK
{
public:
	TRACK(void);
	~TRACK(void);

	void hist(int num);//����ѡ�о�������ֱ��ͼ
	void init(PARTICLE *pParticle,int num);//��ʼ�����Ӳ���
	void update(PARTICLE *pParticle,int num);//�������Ӳ���
	void weight(PARTICLE *pParticle,int num);//��һ������Ȩ��
	void reparticle(PARTICLE *pParticle, particle *particles);//�ز���
	void result(PARTICLE *pParticle);//���㲢��ʾ���ٽ��
	void show(PARTICLE *pParticle);//��ʾ���ٹ���

public :
	
	Mat target_img, target_hist;//��ʼѡȡ���������ֱ��ͼ
	Mat track_img, track_hist;//��ǰ����ֱ��ͼ
	//Mat target_img[CONTOURS_NUMBER], target_hist[CONTOURS_NUMBER];
	//Mat track_img[CONTOURS_NUMBER], track_hist[CONTOURS_NUMBER];

	Rect select;//������ѡ���
	bool select_flag=false;
	bool tracking=false;//���ٱ�־λ
	bool select_show=false;
	Point origin;

	/****rgb�ռ��õ��ı���****/
	//int hist_size[]={16,16,16};//rgb�ռ��ά�ȵ�bin����
	//float rrange[]={0,255.0};
	//float grange[]={0,255.0};
	//float brange[]={0,255.0};
	//const float *ranges[] ={rrange,grange,brange};//range�൱��һ����ά����ָ��

	/****hsv�ռ��õ��ı���****/
	static int hist_size[3];
	static float hrange[2];
	static float srange[2];
	static float vrange[2];

	//int hist_size[]={32,32,32};
	//float hrange[]={0,359.0.0};
	//float srange[]={0,1.0};
	//float vrange[]={0,1.0};
	//static float *ranges[];//��άָ��

	static int channels[3];//��ɫ�ռ�3��ͨ��
};
