#include "ViBe.h"
#include "particles.h"
#include "min_rect.h"
#include "stdafx.h"

vector<vector<Point>> contours;
vector<Vec4i> hierarchy;

vector<RotatedRect> min_r_rects;
vector<Rect> min_rects;

//PARTICLE particles[PARTICLE_NUMBER];
PARTICLE particles[CONTOURS_NUMBER][PARTICLE_NUMBER];
TRACK track[CONTOURS_NUMBER];

int after_select_frames = 0;//选择矩形区域完后的帧计数//全局变量

int vibe_frame = 0;//进行VIBE的帧数

//粒子降序排列函数声明
int particle_decrease(const void *p1, const void *p2);

//定义当前帧图像和hsv空间的声明
Mat current_frame, current_hsv;

Mat mask;

///****hsv空间用到的变量****/
//int hist_size[3] = { 16, 16, 16 };
//float hrange[2] = { 0, 180.0 };
//float srange[2] = { 0, 256.0 };
//float vrange[2] = { 0, 256.0 };
//float *ranges[] = { hrange, srange, vrange };//二维指针
//
//int channels[3] = { 0, 1, 2 };//颜色空间3个通道




////////////////////////VIBE main函数/////////////////////////////////////

int main(int argc, char* argv[])
{
	Mat frame, lab, canny;

	//Mat temp_hsv;

	PARTICLE *pParticle[PARTICLE_NUMBER];

	//读取视频
	VideoCapture capture;
	capture.open(VIDEO_NAME);
	//capture.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	//capture.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

	if (!capture.isOpened())
	{
		cout << "No camera or video input!\n" << endl;
		return -1;
	}

	//获取视频帧率
	double fps = capture.get(CV_CAP_PROP_FPS);
	cout << "视频帧率为" << fps << endl;

	//获取视频帧数  
	long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "视频帧数为" << totalFrameNumber << endl;


	ViBe_BGS Vibe_Bgs;
	bool count = true;

	capture >> frame;

	while (1)
	{
		capture >> frame;
		//capture.read(frame);
		if (frame.empty())
			continue;

		////cvtColor(frame, gray, CV_RGB2GRAY);//￥转为灰度图像
		cvtColor(frame, lab, CV_RGB2Lab);//￥转为Lab颜色空间

		current_frame = frame;

		if (count == true)
		{
			Vibe_Bgs.init(lab);//初始化
			Vibe_Bgs.processFirstFrame(lab);//以第一帧建立背景模型
			cout << " Training ViBe complete!" << endl;
		}
		else
		{
			if (count == false)
			{
				Vibe_Bgs.testAndUpdate(lab);//前景检测与背景模型更新

				//Vibe_Bgs.modify(lab);

				mask = Vibe_Bgs.getMask();
				//Vibe_Bgs.modify2();

				morphologyEx(mask, mask, MORPH_OPEN, Mat());//￥开运算

				//Mat element = getStructuringElement(MORPH_RECT, Size(20,20));
				//morphologyEx(mask, mask, MORPH_CLOSE,element);//闭运算			

				cv::imshow("mask", mask);


				////////////////////particle_tracking////////////////////////////

				/****将rgb空间转换为hsv空间****/
				cvtColor(current_frame, current_hsv, CV_RGB2HSV);
				//track.hsv.push_back(temp_hsv);

				if (1 == after_select_frames)//VIBE计算n帧之后
				{
					Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
					morphologyEx(mask, mask, MORPH_CLOSE, element);//闭运算
					find_min_rect(mask);//得到运动物体最小外接矩形（水平）

					for (int i = 0; i < min_rects.size(); i++)
					{
						/*********计算 hist*********/
						track[i].hist(i);

						/****初始化目标粒子****/
						track[i].init(particles[i], i);
					}
				}
				else
				{
					if (after_select_frames >= 2)//从第二帧开始就可以开始跟踪了
					{
						Rect rectTrackingTemp[CONTOURS_NUMBER] = { Rect(0, 0, 0, 0) };
						Rect tracking_rect[CONTOURS_NUMBER] = { Rect(0, 0, 0, 0) };

						for (int i = 0; i < min_rects.size(); i++)
						{
							/****更新粒子结构体的大部分参数****/
							track[i].update(particles[i], i);

							/****归一化粒子权重****/
							track[i].weight(particles[i], i);

							/****根据粒子的权值降序排列****/
							pParticle[i] = particles[i];
							qsort(particles[i], PARTICLE_NUMBER, sizeof(PARTICLE), &particle_decrease);

							/////////////重采样///////////////
							/****根据粒子权重重采样粒子****/
							pParticle[i] = particles[i];
							PARTICLE newParticle[PARTICLE_NUMBER];
							int np = 0, k = 0;
							for (int q = 0; q < PARTICLE_NUMBER; q++)
							{
								np = cvRound(pParticle[i]->weight*PARTICLE_NUMBER);
								for (int j = 0; j < np; j++)
								{
									newParticle[k] = particles[i][q];
									k++;
									if (k == PARTICLE_NUMBER)
										goto EXITOUT;
								}
							}
							while (k < PARTICLE_NUMBER)
								newParticle[k++] = particles[i][0];
						EXITOUT:
							for (int q = 0; q < PARTICLE_NUMBER; q++)
								particles[i][q] = newParticle[q];


							/****计算最大权重目标的期望位置，作为跟踪结果****/

							/*for (int i = 0; i < min_rects.size(); i++)
							{*/

							pParticle[i] = particles[i];

							rectTrackingTemp[i].x = particles[i][0].x - 0.5*particles[i][0].rect.width;
							rectTrackingTemp[i].y = particles[i][0].y - 0.5*particles[i][0].rect.height;
							rectTrackingTemp[i].width = particles[i][0].rect.width;
							rectTrackingTemp[i].height = particles[i][0].rect.height;


							//创建目标矩形区域
							tracking_rect[i] = rectTrackingTemp[i];

							//pParticle = particles;
							pParticle[i] = particles[i];

							/****显示各粒子运动结果****/
							for (int m = 0; m < PARTICLE_NUMBER; m++)
							{
								//rectangle(frame, pParticle->rect, Scalar(255, 0, 0), 1, 8, 0);
								//pParticle++;

								int x0, y0, x1, y1, x2, y2;
								CvScalar color;
								color = CV_RGB(255, 255, 0);
								x0 = cvRound(pParticle[i]->x - 0.5 * pParticle[i]->scale*pParticle[i]->rect.width);
								y0 = cvRound(pParticle[i]->y - 0.5 * pParticle[i]->scale*pParticle[i]->rect.height);
								x1 = cvRound(pParticle[i]->x + 0.5 * pParticle[i]->scale*pParticle[i]->rect.width);
								y1 = cvRound(pParticle[i]->y + 0.5 * pParticle[i]->scale*pParticle[i]->rect.height);
								x2 = cvRound(particles[i][m].x);
								y2 = cvRound(particles[i][m].y);
								//rectangle(frame, cvPoint(x0, y0), cvPoint(x1, y1), color, 1, 8, 0);
								circle(frame, cvPoint(x2, y2), 3, color, -1, 8, 0);

								//pParticle++;
							}

							/****显示跟踪结果****/
							rectangle(frame, tracking_rect[i], Scalar(0, 0, 255), 1, 8, 0);

							//after_select_frames++;//总循环每循环一次，计数加1
							//if (after_select_frames >= 2)//防止跟踪太长，after_select_frames计数溢出
							//	after_select_frames = 1;
							//}
						}
						after_select_frames++;//总循环每循环一次，计数加1
						if (after_select_frames > 2)//防止跟踪太长，after_select_frames计数溢出
							after_select_frames = 2;
					}
				}
			}
		}
		count = false;
		vibe_frame++;
		if (vibe_frame > 50)
		{
			after_select_frames++;//从此时开始跟踪
			vibe_frame = 51;
		}

		cv::imshow("input", frame);

		if (cvWaitKey(40) == 27)//ESC键可结束程序
			break;
	}

	return 0;
}

//
//
////PARTICLE particles[PARTICLE_NUMBER];
////TRACK track;
////
/////************************************************************************************************************************/
/////****                            如果采用这个onMouse()函数的话，则可以画出鼠标拖动矩形框的4种情形                        ****/
/////************************************************************************************************************************/
////void onMouse(int event, int x, int y, int, void*)
////{
////	//Point origin;//不能在这个地方进行定义，因为这是基于消息响应的函数，执行完后origin就释放了，所以达不到效果。
////	if (track.select_flag)
////	{
////		track.select.x = MIN(track.origin.x, x);//不一定要等鼠标弹起才计算矩形框，而应该在鼠标按下开始到弹起这段时间实时计算所选矩形框
////		track.select.y = MIN(track.origin.y, y);
////		track.select.width = abs(x - track.origin.x);//算矩形宽度和高度
////		track.select.height = abs(y - track.origin.y);
////		track.select &= Rect(0, 0, track.frame.cols, track.frame.rows);//保证所选矩形框在视频显示区域之内
////		//a &=b,即a=a&b
////		//        rectangle(frame,select,Scalar(0,0,255),3,8,0);//显示手动选择的矩形框
////	}
////	if (event == CV_EVENT_LBUTTONDOWN)
////	{
////		track.select_flag = true;//鼠标按下的标志赋真值
////		track.tracking = false;
////		track.select_show = true;
////		track.after_select_frames = 0;//还没开始选择，或者重新开始选择，计数为0
////		track.origin = Point(x, y);//保存下来单击是捕捉到的点
////		track.select = Rect(x, y, 0, 0);//这里一定要初始化，因为在opencv中Rect矩形框类内的点是包含左上角那个点的，但是不含右下角那个点。
////	}
////	else if (event == CV_EVENT_LBUTTONUP)
////	{
////		track.select_flag = false;
////		track.tracking = true;
////		track.select_show = false;
////		track.after_select_frames = 1;//选择完后的那一帧当做第1帧
////	}
////}
////
/************************ 粒子权值降序排列 ***************************/
int particle_decrease(const void *p1, const void *p2)
{
	PARTICLE* _p1 = (PARTICLE*)p1;//？？定义_p1为指向指针p1的particle型指针？？
	PARTICLE* _p2 = (PARTICLE*)p2;
	if (_p1->weight<_p2->weight)
		return 1;
	else if (_p1->weight>_p2->weight)
		return -1;
	return 0;//相等的情况下返回0
}




