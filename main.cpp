#include <iostream>
#include <string>
using namespace std;

#include <opencv2\opencv.hpp>
using namespace cv;

#include "natext.h"

int main()
{
	Mat input = imread("test.png");
	input.convertTo(input, CV_8UC3);
	Mat ret = natGenerate(input, 400, 400, 5);
	//imshow("ret", ret);
	//waitKey(0);
}