#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2\features2d.hpp"

#include <iostream>
#include <stdio.h>
#include <ctime>

#define SIGMA 3
#define KERNEL 11
#define TRESH_MIN 100
#define TRESH_MAX 500
#define ALPHA 2
#define BETA -1
#define GAMMA 0
#define WHITE 200

using namespace std;
using namespace cv;

/// Global Variables


/// Function Headers

/** @function main */
int main(int argc, char** argv)
{
	

	//img = imread(argv[1], 1
	//templ = imread(argv[2], 1);
	Mat img; Mat templ; Mat result;
	char* image_window = "Source Image";
	char* result_window = "Result window";
	string path = "D:\\testimages\\new\\ipm_cu.jpg";
	string templpath = "D:\\testimages\\new\\shablon_2.jpg";
	img = imread(path);
	templ = imread(templpath);

		
	Mat oldimg = img;
	int ch1;
	/// Create windows
	namedWindow(image_window, CV_WINDOW_AUTOSIZE);
//	namedWindow(result_window, CV_WINDOW_AUTOSIZE);
	
	matchTemplate(img, templ, img, TM_CCOEFF); //the most suitable method
	normalize(img, img, 0, 1, NORM_MINMAX, -1, Mat());
	double minVal, maxVal;
	minMaxLoc(img, &minVal, &maxVal);  //find  minimum  and  maximum  intensities
	Mat  draw, image;
	img.convertTo(draw, CV_8U, 255.0 / (maxVal - minVal), -255.0*minVal/(maxVal-minVal));
	
	Canny(draw, draw, 100,500);
	GaussianBlur(draw, draw, cv::Size(KERNEL, KERNEL), SIGMA); //the blur kernel is of an odd size!!
	GaussianBlur(draw, image, cv::Size(KERNEL, KERNEL), SIGMA); //unsharp mask
	addWeighted(draw, ALPHA, image, BETA, 0, image);

	
	vector <Point2d> railpoints; //maybe. just maybe i should create a class for this

	threshold(image, image, 60, 255, cv::THRESH_BINARY);

	copyMakeBorder(image, image, templ.rows / 2, templ.rows / 2, templ.cols / 2, templ.cols / 2, BORDER_CONSTANT, 0);
	//the result is slightly smaller than the image and we need the original coordinaltes so we add a border
	
	//test output of whatever the hell we found at the steps above on the original image
	/*for (int i = 1; i < oldimg.rows; i++)
	for (int j = 1; j < oldimg.cols; j++)
	if (image.at<uchar>(i,j) == 255)
	{
	railpoints.push_back(Point(i, j));
	oldimg.at<Vec3b>(i,j) = (0,0,255); //REDO
	}*/

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<Vec4f> lines;
	Vec4f lin;

	for (int i = 0; i < contours.size(); i++)
	{
     	if (contourArea(contours[i]) > 300)
		{
			fitLine(Mat(contours[i]), lin, CV_DIST_L2,0,0.01,0.01);
			lines.push_back(lin);
			int x0 = lin[2] - oldimg.cols*lin[0];        // a point on the line
			int y0 = lin[3] - oldimg.cols*lin[1];
			int x1 = x0 + 2*oldimg.cols * lin[0]; // add a vector of length 100
			int y1 = y0 + 2*oldimg.cols * lin[1]; // using the unit vector

										 // draw the line

			line(oldimg, cv::Point(x0, y0), cv::Point(x1, y1), (0,0,255), 1); // color and thickness
		
		}
	}

	
//	cout << "runtime = " << clock() / 1000.0 << endl;
	imshow(image_window, oldimg);
//	imshow(result_window, image);

	
	waitKey(0);
	
	return 0;
}

