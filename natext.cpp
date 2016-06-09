#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
using namespace std;

#include "natext.h"

#define randn(x) (rand()%(x))

Mat natGenerate(Mat source, int width, int height, int size)
{
	size = size / 2 * 2 + 1;
	srand((unsigned)time(0));
	int pos = (size*size - 1) / 2;
	int *dx = new int[pos];
	int *dy = new int[pos];
	int count = 0;
	for (int i = -(size - 1) / 2; i <= 0; i++)
	{
		for (int j = -(size - 1) / 2; j <= (size - 1) / 2; j++)
		{
			if (i == 0 && j == 0) break;
			dy[count] = i;
			dx[count] = j;
			count++;
		}
	}

	Mat ret(height, width, CV_8UC3, Scalar::all(0));
	Mat x_source(height, width, CV_32S, Scalar::all(0));
	Mat y_source(height, width, CV_32S, Scalar::all(0));

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int ox = randn(source.cols - size + 1) + (size - 1) / 2;
			int oy = randn(source.rows - size + 1) + (size - 1) / 2;
			ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(oy, ox)[0];
			ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(oy, ox)[1];
			ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(oy, ox)[2];
			x_source.at<int>(i, j) = ox;
			y_source.at<int>(i, j) = oy;
		}
	}
	//imshow("random ret", ret);
	//waitKey(0);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{ 
			//cout << "Ret: " << i << ", " << j << endl;
			vector<Point> can;
			for (int p = 0; p < pos; p++)
			{
				int yy = i + dy[p];
				int xx = j + dx[p];
				if (xx < 0 || xx >= width || yy < 0 || yy >= height)
				{
					continue;
				}
				int cx = x_source.at<int>(yy, xx) - dx[p];
				int cy = y_source.at<int>(yy, xx) - dy[p];
				if (cx < 0 || cx >= source.cols || cy < 0 || cy >= source.rows)
				{
					continue;
				}
				//cout << "  Canidate " << cy << ", " << cx << endl;
				can.push_back(Point(cx, cy));
			}
			double min = 1E20;
			Point minp = Point(-1, -1);
			int valid = 0;
			for (int p = 0; p < can.size(); p++)
			{
				int cpx = can[p].x;
				int cpy = can[p].y;
				//cout << "  Calculating candidate #" << p << " @ " << cpy << ", " << cpx << endl;
				int ncount = pos;
				double sum = 0;
				for (int l = 0; l < pos; l++)
				{
					int cplx = cpx + dx[l];
					int cply = cpy + dy[l];
					int xx = j + dx[l];
					int yy = i + dy[l];
					if (xx < 0 || xx >= width || yy < 0 || yy >= height || 
							cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
					{
						sum = 0;
						break;
					}
					double d = (double)(source.rows - cpy) / (1.0*source.rows);
					double r = pow(randn(1000) / 1000.0, 4);
					if (r > d && d < 0.1 && can.size()>1)
					{
						ncount = 1;
					}
					int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
					int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
					int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
					sum += (r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2);
				}
				sum /= ncount;
				//cout << "   sum = " << sum << endl;
				if (min > sum)
				{
					min = sum;
					minp = can[p];
				}
			}
			//cout << "Minp @" << minp.y << ", " << minp.x << endl;
			if (minp == Point(-1, -1))
			{
				minp.x = randn(source.cols);
				minp.y = randn(source.rows);
			}
			if (minp != Point(-1, -1))
			{
				ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(minp.y, minp.x)[0];
				ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(minp.y, minp.x)[1];
				ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(minp.y, minp.x)[2];
				x_source.at<int>(i, j) = minp.x;
				y_source.at<int>(i, j) = minp.y;
			}
			//cout << "End" << endl;
			if (j == width - 1)
			{
				imshow("Now ret", ret);
				waitKey(1);
			}
		}
	}
	
	waitKey();

	return ret;
}