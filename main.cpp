#include <iostream>
#include <string>
using namespace std;

#include <opencv2\opencv.hpp>
using namespace cv;

#include "natext.h"
#include "icUI.h"

int main()
{
	Mat input = imread("test-comp.png");
	Mat mask = getMask(input);
  //Mat mask = imread("mask.bmp");
  //imwrite("mask.bmp", mask);
	Mat ret = natGenerate(input, mask, 11);
	imshow("ret", ret);
	waitKey(0);
  imwrite("result.png", ret);
}