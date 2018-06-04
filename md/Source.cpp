#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2\features2d.hpp"
#include <math.h>
#include <chrono>

#include <iostream>
#include <stdio.h>
#include <ctime>

#define SIGMA 5
#define KERNEL 11
#define TRESH_MIN 100
#define TRESH_MAX 500
#define ALPHA 2
#define BETA -1
#define GAMMA 0
#define WHITE 200

using namespace std;
using namespace std::chrono;
using namespace cv;

struct NGrect {
	Point2f points[4];
	double side1; 
	double side2; 
	double length;
	double width;
	NGrect(Point2f p[4])
	{
		points[0] = p[0];
		points[1] = p[1];
		points[2] = p[2];
		points[3] = p[3];
		side1 = sqrt((points[1].x - points[2].x)*(points[1].x - points[2].x) + (points[1].y - points[2].y)*(points[1].y - points[2].y));
		side2 = sqrt((points[2].x - points[3].x)*(points[2].x - points[3].x) + (points[2].y - points[3].y)*(points[2].y - points[3].y));
		length = max(side1, side2);
		width = min(side1, side2);

	}
};

bool BelongsTo(NGrect r, Point2f P)
{
	
	Point2f A = r.points[0];
	Point2f B = r.points[1];
	Point2f C = r.points[2];
	Point2f D = r.points[3];

	Point2f PA(A.x - P.x, A.y - P.y);
	Point2f PB(B.x - P.x, B.y - P.y);
	Point2f PC(C.x - P.x, C.y - P.y);
	Point2f PD(D.x - P.x, D.y - P.y);

	float PAPB = (PA.x*PB.y - PA.y*PB.x);
	float PBPC = (PB.x*PC.y - PB.y*PC.x);
	float PCPD = (PC.x*PD.y - PC.y*PD.x);
	float PDPA = (PD.x*PA.y - PD.y*PA.x);

//	if (signbit(PAPB) == signbit(PBPC) == signbit(PCPD) == signbit(PDPA)) //bug is here
		if ((signbit(PAPB)&&signbit(PBPC)&&signbit(PCPD)&&signbit(PDPA))|| (!signbit(PAPB) && !signbit(PBPC) && !signbit(PCPD) && !signbit(PDPA)))
		return true;
	else
		return false;



}

/** @function main */
int main(int argc, char** argv)
{


	//img = imread(argv[1], 1
	//templ = imread(argv[2], 1);
	Mat img; Mat templ; Mat result;
	char* image_window = "Source Image";
	char* result_window = "Result window";
	string path = "D:\\testimages\\new\\ipm_3.jpg";
	string templpath;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	
	string perspective = "ipm";
	if (perspective == "np")
		templpath = "D:\\testimages\\new\\thick_sh.jpg";
	if (perspective == "ipm")
		templpath = "D:\\testimages\\new\\thin_sh.jpg";
	string contrast = "low";
	
	
	int ch1;

	//for (int cyclop = 0; cyclop < 10000; cyclop++)
	//{
		img = imread(path);
		Mat oldimg = img;
		templ = imread(templpath);
		
		/// Create windows
	namedWindow(image_window, CV_WINDOW_AUTOSIZE);
		//namedWindow(result_window, CV_WINDOW_AUTOSIZE);


		matchTemplate(img, templ, img, TM_CCOEFF); //the most suitable method
		normalize(img, img, 0, 1, NORM_MINMAX, -1, Mat());
		double minVal, maxVal;
		minMaxLoc(img, &minVal, &maxVal);
		Mat  draw, image;
		img.convertTo(image, CV_8U, 255.0 / (maxVal - minVal), -255.0*minVal / (maxVal - minVal));
		vector <Point2f> railpoints;
		double alpha = 0;
		if (contrast == "low")
		{
			Canny(image, image, 100, 400);
			bitwise_not(image, image);
			alpha = 2.2;//2.2!!!!!
		}
		if (contrast == "high")
		{
			alpha = 3;
		}
		Mat blurred;
		GaussianBlur(image, image, cv::Size(11, 11), 5);
		GaussianBlur(image, blurred, cv::Size(11, 11), 3);

		addWeighted(image, alpha, blurred, -1, 0, image);
		minMaxLoc(image, &minVal, &maxVal);

		if (contrast == "low")
			threshold(image, image, 254, 255, THRESH_BINARY);
		if (contrast == "high")
			threshold(image, image, minVal + (maxVal - minVal) / 1.5, 255, THRESH_BINARY);

		copyMakeBorder(image, image, templ.rows / 2, templ.rows / 2, templ.cols / 2, templ.cols / 2, BORDER_CONSTANT, 255);

		for (int i = 1; i < image.rows; i++)
			for (int j = 1; j < image.cols; j++)
				if (image.at<uchar>(i, j) < 255)
				{
					railpoints.push_back(Point(j, i));
				}


		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		vector<Vec4f> lines;
		vector<NGrect> rectangles;
		vector<NGrect> railsrects;
		Vec4f lin;
		double maxlength = 0;
		for (int i = 0; i < contours.size(); i++)
		{
			RotatedRect r = minAreaRect(contours[i]);
		//	rectangles.push_back(r);
			Point2f rect_points[4];
			r.points(rect_points);
			NGrect rect(rect_points);
			rectangles.push_back(rect);
		}
		for (int i = 0; i < rectangles.size(); i++)
		{
			if ((rectangles[i].length > maxlength) && (rectangles[i].width!= image.cols))
			         	maxlength = rectangles[i].length;
			
		}

 		for (int i = 0; i < rectangles.size(); i++)
		{
			//здесь какой-то кривой-кривой отсев по площади.
			if (rectangles[i].length >= 0.55*maxlength && rectangles[i].length>=15*rectangles[i].width) //THIS NEEDS TO BE FIXED
			{
			for (int j = 0; j < 4; j++)
				{
					line(image, rectangles[i].points[j], rectangles[i].points[(j + 1) % 4], (0, 0, 0), 1, 8);
				}
				railsrects.push_back(rectangles[i]);
			}
		}
		vector<vector<Point2f>> rails(railsrects.size());
		vector<vector<Point2i>> rails_short(railsrects.size());
		//more advanced method
		for (int i = 0; i < railpoints.size(); i++)
			for (int j = 0; j < railsrects.size(); j++)
				if (BelongsTo(railsrects[j], railpoints[i]))
				{
					rails[j].push_back(railpoints[i]);
					break;
				}
		float sum_x = 0, sum_y = 0;
		int k = 0;
		int m = 0;

		for (int i = 0; i < rails.size(); i++)
		{
			k = 1;
			m = 0;
			double min = rails[i][0].y;
			double max = rails[i][rails[i].size() - 1].y;
			double step = (max - min) / 10;
			rails_short[i].push_back(Point2i((rails[i][0].x + rails[i][20].x) / 2, rails[i][0].y));
			for (int j = 0; j < rails[i].size(); j++)
			{

				if (rails[i][j].y <= min + step*k)
				{
					sum_x += rails[i][j].x;
					sum_y += rails[i][j].y;
					m++;
				}
				else
				{
					if (m != 0)
					{
						Point2d p(sum_x / m, sum_y / m);

						if (BelongsTo(railsrects[i], p))
						{
							rails_short[i].push_back(Point2i(sum_x / m, sum_y / m));
							circle(image, Point2i(sum_x / m, sum_y / m), 2, 250, 1);
							//	imshow(image_window, image);
							//	waitKey(0);


						}
						m = 0;
						sum_x = 0;
						sum_y = 0;
					}
					k++;

				}
			}
			rails_short[i].push_back(Point2i((rails[i][rails[i].size() - 1].x + rails[i][rails[i].size() - 19].x) / 2, rails[i][rails[i].size() - 1].y));
		}



		for (int i = 0; i < rails_short.size(); i++)
			for (int j = 1; j < rails_short[i].size(); j++)
			{
				line(oldimg, rails_short[i][j - 1], rails_short[i][j], (255, 0, 255), 2, 8);
				//	imshow(image_window, oldimg);
				//	waitKey(0);
			}
			//*/
	//}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	auto duration = duration_cast<milliseconds>(t2 - t1).count()/10000;

	cout << duration;

	
	imshow(image_window, oldimg); 
   imshow(result_window, image);

	
	waitKey(0);
	
	return 0;
}


