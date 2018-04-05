#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

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
	/// Load image and template
	//img = imread(argv[1], 1
	//templ = imread(argv[2], 1);
	Mat img; Mat templ; Mat result;
	char* image_window = "Source Image";
	char* result_window = "Result window";

	int match_method;
	int max_Trackbar = 5;

	string path = "D:\\testimages\\new\\ipm_str.jpg";
	string templpath = "D:\\testimages\\new\\shablon_2.jpg";
	img = imread(path);
	templ = imread(templpath);

	Mat oldimg = img;
	int ch1;
	/// Create windows
	namedWindow(image_window, CV_WINDOW_AUTOSIZE);
	namedWindow(result_window, CV_WINDOW_AUTOSIZE);

	matchTemplate(img, templ, img, TM_CCOEFF); //the most suitable method
	normalize(img, img, 0, 1, NORM_MINMAX, -1, Mat());
	double minVal, maxVal;
	minMaxLoc(img, &minVal, &maxVal);  //find  minimum  and  maximum  intensities
	Mat  draw, image;
	img.convertTo(draw, CV_8U, 255.0 / (maxVal - minVal), -255.0*minVal/(maxVal-minVal));
	
	Canny(draw, draw, 100, 500);
	GaussianBlur(draw, draw, cv::Size(KERNEL, KERNEL), SIGMA); //the blur kernel is of an odd size!!
	GaussianBlur(draw, image, cv::Size(KERNEL, KERNEL), SIGMA); //unsharp mask
	addWeighted(draw, ALPHA, image, BETA, 0, image);

	
	vector <Point2d> railpoints; //maybe. just maybe i should create a class for this

	threshold(image, image, 50, 255, cv::THRESH_BINARY);

	copyMakeBorder(image, image, templ.rows / 2, templ.rows / 2, templ.cols / 2, templ.cols / 2, BORDER_CONSTANT, 0);
	//the result is slightly smaller than the image and we need the original coordinaltes so we add a border

	//TODO: destroy small blobs with findContours i guess

	//this is basically for test purposes so far
	for (int i = 1; i < oldimg.rows; i++)
		for (int j = 1; j < oldimg.cols; j++)
			if (image.at<uchar>(i,j) == 255)
			{
				railpoints.push_back(Point(i, j));
				oldimg.at<Vec3b>(i,j) = (0,0,255); //REDO 
			}
	imshow(image_window, oldimg);
	imshow(result_window, image);

	
	waitKey(0);
	
	return 0;
}

