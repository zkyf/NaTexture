#include <iostream>
#include <string>
using namespace std;

#include <opencv2\opencv.hpp>
using namespace cv;

#include "natext.h"
#include "icUI.h"

int main()
{
	Mat input = imread("test-comp.jpg");
	Mat mask = getMask(input);
	Mat ret = natGenerate(input, mask, 11);
	imshow("ret", ret);
	waitKey(0);
}