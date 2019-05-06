
#include "min_rect.h"
#include "particles.h"


int find_min_rect(Mat image)
{
	cv::Mat src, dst, canny_output;
	RNG rng;
	double minlength = 60, maxlength=150;
	double minarea = 100;

	/// Load source image and convert it to gray  
	src =image;
	blur(src, src, Size(3, 3));

	//Point2f vertices[50][4];//假设轮廓数<=50

	/// Detect edges using canny  
	Canny(src, canny_output, 80, 255, 3);
	/// Find contours  
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	imshow("canny",canny_output);
	

	/////////////delete too long or too short contour/////////////
	for (int i = 0; i<contours.size(); i++)
	{
		double temparea = fabs(contourArea(contours[i]));//面积
		//int tmparea = contours[i].size();//长度

		if (contours[i].size() < minlength)
		{
			//删除长度小于设定值的轮廓  
			contours.erase(contours.begin() + i);
			--i;
			std::wcout << "delete too short area" << std::endl;
			continue;
		}

		if (contours[i].size()> maxlength)
		{
			//删除长度大于设定值的轮廓  
			contours.erase(contours.begin() + i);
			--i;
			std::wcout << "delete too long area" << std::endl;
			continue;
		}

		if (temparea < minarea)
		{
			//删除面积小于设定值的轮廓  
			contours.erase(contours.begin() + i);
			--i;
			std::wcout << "delete too small area" << std::endl;
			continue;
		}

	}

	dst = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		//随机颜色  
		//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(dst, contours, i, Scalar(0, 255, 0), 1, 8, hierarchy, 0, Point());
		
		//min_r_rects.push_back(cv::minAreaRect(contours[i]));
		/*min_r_rects[i].points(vertices[i]);
		for (int j = 0; j < 4; j++)
			line(dst, vertices[i][j], vertices[i][(j+ 1) % 4], Scalar(0, 255, 0));*/
		//min_rects.push_back(min_r_rects[i].boundingRect());
		min_rects.push_back(boundingRect(contours[i]));
		rectangle(dst, min_rects[i], Scalar(255, 0, 0));
	}
	// Create Window  
	//char* source_window = "countours";
	//namedWindow(source_window, CV_WINDOW_NORMAL);//可变窗口大小
	imshow("countours", dst);


	return 0;


}




