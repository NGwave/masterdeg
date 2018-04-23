#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2\features2d.hpp"
#include <math.h>

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
using namespace cv;
//вообще ненужная функция
/*
Mat rotate(Mat src, double angle)
{
	Mat dst;
	double c = cos(angle);
	double s = sin(angle);
	Point2f pt(src.cols / 2., src.rows / 2.);
	Mat r = getRotationMatrix2D(pt, -angle, 1.0);
	int nW = int((src.cols * s) + (src.rows * c));
		int	nH = int((src.cols * c) + (src.rows * s));
		r.at<float>(0,2) += (nW / 2) - src.cols / 2;
		r.at<float>(1,2) += (nH / 2) - src.rows / 2;

	warpAffine(src, dst, r, Size(src.cols, src.rows));
	return dst;
}*/
bool BelongsTo(RotatedRect r, Point2f P)
{
	Point2f rpoints[4];
	r.points(rpoints);
	Point2f A = rpoints[0];
	Point2f B = rpoints[1];
	Point2f C = rpoints[2];
	Point2f D = rpoints[3];

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
	string path = "D:\\testimages\\new\\ipm_10.jpg";
	string templpath = "D:\\testimages\\new\\shablon_x.jpg";
	img = imread(path);
	templ = imread(templpath);

		
	Mat oldimg = img;
	int ch1;
	/// Create windows
	namedWindow(image_window, CV_WINDOW_AUTOSIZE);
//	namedWindow(result_window, CV_WINDOW_AUTOSIZE);
	GaussianBlur(img, img, cv::Size(3, 3), 0); //for better work of Canny
	matchTemplate(img, templ, img, TM_CCOEFF); //the most suitable method
	normalize(img, img, 0, 1, NORM_MINMAX, -1, Mat());
	double minVal, maxVal;
	minMaxLoc(img, &minVal, &maxVal);  //find  minimum  and  maximum  intensities
	Mat  draw, image;
	img.convertTo(image, CV_8U, 255.0 / (maxVal - minVal), -255.0*minVal/(maxVal-minVal));
	vector <Point2f> railpoints; 
	
	Canny(image, image, 100, 500);
	Mat blurred;
	GaussianBlur(image, image, cv::Size(11, 11), 5);
	GaussianBlur(image, blurred, cv::Size(11, 11), 3);
	addWeighted(image, 3, blurred, -1, 0,image);


	copyMakeBorder(image, image, templ.rows / 2, templ.rows / 2, templ.cols / 2, templ.cols / 2, BORDER_CONSTANT, 0);
	bitwise_not(image, image);
     minMaxLoc(image, &minVal, &maxVal);
  threshold(image, image, maxVal*(1-minVal/maxVal), 255, THRESH_BINARY);

  for (int i = 1; i < image.cols; i++)
	  for (int j = 1; j < image.rows; j++)
		  if (image.at<uchar>(j,i) <255)
		  {
			  railpoints.push_back(Point(i,j));
		  }


   vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<Vec4f> lines;
	vector<RotatedRect> rectangles;
	Vec4f lin;

for (int i = 0; i < contours.size(); i++)
	{
	if((contourArea(contours[i]) > max(image.rows,image.cols)) && (contours[i].size()>4)&& arcLength(contours[i],1)>max(image.rows, image.cols)) //THIS NEEDS TO BE FIXED
		{
			RotatedRect r = minAreaRect(contours[i]);
			rectangles.push_back(r);
			Point2f rect_points[4]; 
			r.points(rect_points);
			for (int j = 0; j < 4; j++)
			{
				line(image, rect_points[j], rect_points[(j + 1) % 4], (0, 0, 0), 1, 8);
			}
			//this is for lines
		/*	fitLine(Mat(contours[i]), lin, CV_DIST_L2,0,0.01,0.01);
			lines.push_back(lin);
			int x0 = lin[2] - oldimg.cols*lin[0];        // a point on the line
			int y0 = lin[3] - oldimg.cols*lin[1];
			int x1 = x0 + 2*oldimg.cols * lin[0]; // add a vector of length 100
			int y1 = y0 + 2*oldimg.cols * lin[1]; // using the unit vector

			line(oldimg, cv::Point(x0, y0), cv::Point(x1, y1), (0,0,255), 1); // color and thickness*/
			
		}
	}
	vector<vector<Point2f>> rails(rectangles.size());
	vector<vector<Point2i>> rails_short(rectangles.size());
	//more advanced method
	for (int i = 0; i<railpoints.size(); i++)
	   for (int j = 0; j<rectangles.size();j++)
		if (BelongsTo(rectangles[j], railpoints[i]))
		{
			rails[j].push_back(railpoints[i]);
			break;
		}
	float sum_x = 0, sum_y =0;
	int k=0;
	int m=0;
	int dotsforone = 0;

	for (int i = 0; i < rails.size(); i++)
	{
		k = 1;
		double min = rails[i][0].x;
		double max = rails[i][rails[i].size() - 1].x;
		double step = (max - min) / 30;
		//redo when awake
		float first_x = rails[i][0].x + rails[i][1].x + rails[i][2].x+rails[i][3].x+ rails[i][4].x + rails[i][5].x + rails[i][6].x + rails[i][7].x;
		float first_y = rails[i][0].y + rails[i][1].y + rails[i][2].y+rails[i][3].y+ rails[i][4].x + rails[i][5].x + rails[i][6].x + rails[i][7].x;
		rails_short[i].push_back(Point2f(first_x / 4, first_y / 4));
			for (int j = 0; j < rails[i].size(); j++)
		{

		     if (rails[i][j].x <= min+step*k) 
			 {
				 sum_x += rails[i][j].x;
				 sum_y += rails[i][j].y;
				 m++;
			 }
			 else
			 {
				 if (m != 0)
				 {
					 Point2f p(sum_x / m, sum_y / m);
					
 					 if (BelongsTo(rectangles[i], p))
					 {
						 rails_short[i].push_back(Point2f(sum_x / m, sum_y / m));
					//	 image.at<uchar>(sum_y / m, sum_x / m) = 255;
				    //     imshow(result_window, image);
					//	 waitKey(0);

					}
					 m = 0;
					 sum_x = 0;
					 sum_y = 0;
				 }
					 k++;
				
			 }
		}
   }
	
//	Mat middles(image.rows,image.cols,image.type);

//	cout << "runtime = " << clock() / 1000.0 << endl;

	for (int i = 0; i < rails_short.size(); i++)
		for (int j = 1; j < rails_short[i].size(); j++)
			line(oldimg, rails_short[i][j - 1], rails_short[i][j], (255, 0, 255), 2, 8);
	imshow(image_window, oldimg); 


	imshow(result_window, image);

	
	waitKey(0);
	
	return 0;
}

