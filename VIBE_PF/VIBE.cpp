#include "ViBe.h"

using namespace std;
using namespace cv;

int c_xoff[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };  //x���ھӵ�
int c_yoff[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };  //y���ھӵ�

ViBe_BGS::ViBe_BGS(void)
{

}
ViBe_BGS::~ViBe_BGS(void)
{

}

/**************** Assign space and init ***************************/
void ViBe_BGS::init(const Mat _image)
{
	//��̬������ά���飬samples[][][NUM_SAMPLES]�洢ǰ�����������Ĵ���
	samples = new unsigned char ***[_image.rows];
	//samples = new unsigned char **[_image.rows];//��ָ�������ָ���ָ�룬�����СΪ����-1
	for (int i = 0; i < _image.rows; i++)
	{
		samples[i] = new unsigned char **[1024];
		//samples[i] = new unsigned char *[1024];//��1024
		for (int j = 0; j < _image.cols; j++)
		{
			samples[i][j] = new unsigned char *[NUM_SAMPLES + 1];
			//samples[i][j] = new unsigned char[NUM_SAMPLES + 1];
			for (int k = 0; k < NUM_SAMPLES + 1; k++)
			{
				samples[i][j][k] = new unsigned char[3];
				samples[i][j][k][0] = 0;
				samples[i][j][k][1] = 0;
				samples[i][j][k][2] = 0;
				//samples[i][j][k] = 0;//��ʼ��ÿ�����ص�ÿ������ֵΪ0
			}

		}

	}
	//��ʼ�������ͼ�����Ϊ0
	m_mask = Mat::zeros(_image.size(), CV_8UC1);// ��CV_8UC1:8λ��ͨ��  �������д洢�Ҷ�ֵ
}

/**************** Init model from first frame ********************/
void ViBe_BGS::processFirstFrame(const Mat _image)
{
	RNG rng;
	int row, col;

	for (int i = 0; i < _image.rows; i++)
	{
		for (int j = 0; j < _image.cols; j++)
		{
			for (int k = 0; k < NUM_SAMPLES; k++)
			{
				// Random pick up NUM_SAMPLES pixel in neighbourhood to construct the model
				int random;

				random = rng.uniform(0, 9);//�����ѡ��9���ھӵ��е�1��
				row = i + c_yoff[random];
				if (row < 0)
					row = 0;
				if (row >= _image.rows)
					row = _image.rows - 1;

				random = rng.uniform(0, 9);
				col = j + c_xoff[random];
				if (col < 0)
					col = 0;
				if (col >= _image.cols)
					col = _image.cols - 1;

				samples[i][j][k][0] = _image.at<cv::Vec3b>(row, col)[0];
				samples[i][j][k][1] = _image.at<cv::Vec3b>(row, col)[1];
				samples[i][j][k][2] = _image.at<cv::Vec3b>(row, col)[2];
				//samples[i][j][k] = _image.at<uchar>(row, col);
			}
		}
	}
}

/**************** Test a new frame and update model ********************/
void ViBe_BGS::testAndUpdate(const Mat _image)
{
	RNG rng;

	for (int i = 0; i < _image.rows; i++)
	{
		for (int j = 0; j < _image.cols; j++)
		{
			int matches(0), count(0);
			//int dist;
			float dist, dist1, dist2, dist3;

			while (matches < MIN_MATCHES && count < NUM_SAMPLES)
			{
				//�����ֵ
				dist1 = pow(samples[i][j][count][0] - _image.at<cv::Vec3b>(i, j)[0], 2);
				dist2 = pow(samples[i][j][count][1] - _image.at<cv::Vec3b>(i, j)[1], 2);
				dist3 = pow(samples[i][j][count][2] - _image.at<cv::Vec3b>(i, j)[2], 2);
				dist = sqrt(0.25*dist1 + dist2 + dist3);
				//dist = abs(samples[i][j][count] - _image.at<uchar>(i, j));
				//abs():���ؾ���ֵ

				if (dist < RADIUS)
					matches++;
				count++;
			}

			if (matches >= MIN_MATCHES)
			{
				// It is a background pixel
				samples[i][j][NUM_SAMPLES][0] = 0;
				samples[i][j][NUM_SAMPLES][1] = 0;
				samples[i][j][NUM_SAMPLES][2] = 0;
				//samples[i][j][NUM_SAMPLES] = 0 ;

				// Set background pixel to 0
				m_mask.at<uchar>(i, j) = 0;

				// ���һ�������Ǳ����㣬��ô���� 1 / defaultSubsamplingFactor �ĸ���ȥ�����Լ���ģ������ֵ
				int random = rng.uniform(0, SUBSAMPLE_FACTOR);
				if (random == 0)
				{
					//��������¸õ�����һ������ֵ��ʹ����ڸõ��ֵ
					random = rng.uniform(0, NUM_SAMPLES);

					samples[i][j][random][0] = _image.at<cv::Vec3b>(i, j)[0];
					samples[i][j][random][1] = _image.at<cv::Vec3b>(i, j)[1];
					samples[i][j][random][2] = _image.at<cv::Vec3b>(i, j)[2];
					//samples[i][j][random] = _image.at<uchar>(i, j);
				}

				// ͬʱҲ�� 1 / defaultSubsamplingFactor �ĸ���ȥ���������ھӵ��ģ������ֵ
				random = rng.uniform(0, SUBSAMPLE_FACTOR);
				if (random == 0)
				{
					int row, col;

					random = rng.uniform(0, 9);//�����ѡ������һ���ھӵ�
					row = i + c_yoff[random];
					if (row < 0)
						row = 0;
					if (row >= _image.rows)
						row = _image.rows - 1;

					random = rng.uniform(0, 9);
					col = j + c_xoff[random];
					if (col < 0)
						col = 0;
					if (col >= _image.cols)
						col = _image.cols - 1;

					random = rng.uniform(0, NUM_SAMPLES);//�����ѡ��õ���ھӵ������һ������ֵ��ʹ����ڸõ��ֵ

					samples[row][col][random][0] = _image.at<cv::Vec3b>(i, j)[0];
					samples[row][col][random][1] = _image.at<cv::Vec3b>(i, j)[1];
					samples[row][col][random][2] = _image.at<cv::Vec3b>(i, j)[2];
					//samples[i][j][random] = _image.at<uchar>(i, j);
					////samples[row][col][random] = _image.at<uchar>(i, j);
				}
			}
			else//matches < MIN_MATCHES
			{
				// It is a foreground pixel
				samples[i][j][NUM_SAMPLES][0]++;
				//samples[i][j][NUM_SAMPLES]++;

				// Set background pixel to 255
				m_mask.at<uchar>(i, j) = 255;

				//���ĳ�����ص�����N�α����Ϊǰ��������Ϊһ�龲ֹ��������Ϊ�˶����������Ϊ������
				if (samples[i][j][NUM_SAMPLES][0] > 15)
					//if (samples[i][j][NUM_SAMPLES]>20)
				{

					{
						//���������һ���ھӵ�Ϊ�����㣬���ı���ھӵ�����һ������ֵΪ�����ص�
						int random = rng.uniform(0, 9);//�����ѡ������һ���ھӵ�
						int row, col;
						row = i + c_yoff[random];
						if (row < 0)
							row = 0;
						if (row >= _image.rows)
							row = _image.rows - 1;

						random = rng.uniform(0, 9);
						col = j + c_xoff[random];
						if (col < 0)
							col = 0;
						if (col >= _image.cols)
							col = _image.cols - 1;

						random = rng.uniform(0, NUM_SAMPLES);//�����ѡ��õ���ھӵ������һ������ֵ��ʹ����ڸõ��ֵ

						samples[row][col][random][0] = _image.at<cv::Vec3b>(i, j)[0];
						samples[row][col][random][1] = _image.at<cv::Vec3b>(i, j)[1];
						samples[row][col][random][2] = _image.at<cv::Vec3b>(i, j)[2];
						//samples[i][j][random] = _image.at<uchar>(i, j);
						////samples[row][col][random] = _image.at<uchar>(i, j);

						samples[row][col][NUM_SAMPLES][0] = 0;
						m_mask.at<uchar>(row, col) = 0;
					}

					// It is a background pixel
					samples[i][j][NUM_SAMPLES][0] = 0;
					//samples[i][j][NUM_SAMPLES] = 0;

					// Set background pixel to 0
					m_mask.at<uchar>(i, j) = 0;
				}
			}
		}
	}
}

/**************** Modify ********************/
void ViBe_BGS::modify(const Mat _image)
{
	for (int i = 0; i < _image.rows; i++)
	{
		for (int j = 0; j < _image.cols; j++)
		{
			int nc = 0, tn = 5;//8������һ�¸���nc����ֵtn
			for (int p = -1; p <= 1; p++)
			{
				for (int q = -1; q <= 1; q++)
				{
					int row, col;
					row = i + p;
					col = j + q;
					if (row < 0)
						row = 0;
					if (row >= _image.rows)
						row = _image.rows - 1;
					if (col < 0)
						col = 0;
					if (col >= _image.cols)
						col = _image.cols - 1;

					if (samples[i][j][NUM_SAMPLES][0] == 0 && samples[row][col][NUM_SAMPLES][0] == 0)//background
						nc++;
					else
					{
						if (samples[i][j][NUM_SAMPLES][0] > 0 && samples[row][col][NUM_SAMPLES][0] > 0)//foreground
							nc++;
					}
				}
			}
			if (nc < tn)
			{
				if (samples[i][j][NUM_SAMPLES][0] == 0)
				{
					samples[i][j][NUM_SAMPLES][0]++;
					m_mask.at<uchar>(i, j) = 255;
				}
				else
				{
					samples[i][j][NUM_SAMPLES][0] = 0;
					m_mask.at<uchar>(i, j) = 0;
				}
			}
		}
	}
}


void ViBe_BGS::modify2()
{
	int nc , tn ;
	tn = 4;
	for (int i = 0; i < m_mask.rows; i++)
	{
		for (int j = 0; j < m_mask.cols; j++)
		{
			nc = 0;
			for (int p = -2; p <= 2; p++)
			{
				for (int q = -2; q <= 2; q++)
				{
					int row, col;
					row = i + p;
					col = j + q;
					if (row < 0)
						row = 0;
					if (row >= m_mask.rows)
						row = m_mask.rows - 1;
					if (col < 0)
						col = 0;
					if (col >= m_mask.cols)
						col = m_mask.cols - 1;

					if (m_mask.at<uchar>(row, col) == 255)//foreground
						nc++;
				}
			}
			if (nc > tn)
			{
				//samples[row][col][NUM_SAMPLES][0]++;
				mask.at<uchar>(i, j) = 255;
			}
		}
	}
}