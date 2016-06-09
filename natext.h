#include <opencv2\opencv.hpp>
using namespace cv;

#include <iostream>
#include <fstream>
using namespace std;

#ifndef _SYNTHESIZING_NATURAL_TEXTURES
#define _SYNTHESIZING_NATURAL_TEXTURES

Mat natGenerate(Mat source, int width, int height, int size = 3);

#endif