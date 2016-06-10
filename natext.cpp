#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <math.h>
using namespace std;

#include "natext.h"

#define randn(x) (rand()%(x))

double natGaussian(double x, double sigma = 1.0, double mu = 0.0)
{
	return exp(-(x - mu)*(x - mu) / (2 * sigma*sigma)) / (sigma*sqrt(2 * M_PI));
}

Mat natGenerate(Mat source, int width, int height, int size)
{
	const int delta = 5;

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

	for (int i = 0; i < pos; i++)
	{
		cout << dy[i] << ", " << dx[i] << endl;
	}

	Mat ret(height, width, CV_8UC3, Scalar::all(0));
	Mat x_source(height, width, CV_32S, Scalar::all(0));
	Mat y_source(height, width, CV_32S, Scalar::all(0));

	for (int i = 0; i < source.rows - delta; i++)
	{
		for (int j = 0; j < source.cols - delta; j++)
		{
			ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(i, j)[0];
			ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(i, j)[1];
			ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(i, j)[2];
			x_source.at<int>(i, j) = j;
			y_source.at<int>(i, j) = i;
		}
	}
	//imshow("random ret", ret);
	//waitKey(0);

	for (int i = source.rows - delta; i < height; i++)
	{
		for (int j = 0; j < source.cols - delta; j++)
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
				//cout << "  cy: " << cy << ", cx: " << cx << endl;
				if (cx < 0 || cx >= source.cols || cy < 0 || cy >= source.rows)
				{
					continue;
				}
				//cout << "  Canidate " << cy << ", " << cx << endl;
				can.push_back(Point(cx, cy));
			}
			double min = 1E20;
			Point minp = Point(-1, -1);
			//cout << "can.size() = " << can.size() << endl;
			Mat retL(size, size, CV_8UC3, Scalar::all(255));
			Mat oriL(size, size, CV_8UC3, Scalar::all(255));
			retL.at<Vec3b>(size / 2, size / 2)[0] = 0;
			retL.at<Vec3b>(size / 2, size / 2)[1] = 0;
			retL.at<Vec3b>(size / 2, size / 2)[2] = 255;
			oriL.at<Vec3b>(size / 2, size / 2)[0] = 255;
			oriL.at<Vec3b>(size / 2, size / 2)[1] = 0;
			oriL.at<Vec3b>(size / 2, size / 2)[2] = 0;
			int count = 0;
			for (int p = 0; p < can.size(); p++)
			{
				//cout << "  Calculating candidate #" << p << " @ " << cpy << ", " << cpx << endl;
				double ncount = 0;
				double sum = 0;
				int cpx = can[p].x;
				int cpy = can[p].y;
				for (int l = 0; l < pos; l++)
				{
					int cplx = cpx + dx[l];
					int cply = cpy + dy[l];
					int xx = j + dx[l];
					int yy = i + dy[l];
					if (cplx < 0 || cplx >= source.cols - 1 || cply < 0 || cply >= source.rows - 1)
					{
						break;
					}
					if (xx < 0 || xx >= source.cols || yy < 0 || yy >= height)
					{
						continue;
					}
					int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
					int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
					int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
					double d = sqrt(dx[l] * dx[l] / (size * size / 4)
													+ dy[l] * dy[l] / (size * size / 4));
					double ratio = 1 - natGaussian(d, 0.4);
					sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
					ncount += ratio;
				}
				if (ncount>0)
				{
					sum /= ncount;
					count++;
				}
				//cout << "   sum = " << sum << endl;
				if (min > sum && ncount>0)
				{
					min = sum;
					minp = Point(cpx, cpy);
				}
			}

			if (minp == Point(-1, -1) || count < size)
			{
				cout << "No sum" << endl;
				for (int cpy = delta; cpy < source.rows - delta; cpy++)
				{
					for (int cpx = delta; cpx < source.rows - delta; cpx++)
					{
						double ncount = 0;
						double sum = 0;
						for (int l = 0; l < pos; l++)
						{
							int cplx = cpx + dx[l];
							int cply = cpy + dy[l];
							int xx = j + dx[l];
							int yy = i + dy[l];
							if (cplx < delta || cplx >= source.cols - delta || cply < delta || cply >= source.rows - delta)
							{
								break;
							}
							if (xx < 0 || xx >= source.cols || yy < 0 || yy >= height)
							{
								continue;
							}
							int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
							int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
							int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
							double d = sqrt(dx[l] * dx[l] / (size * size / 4) + dy[l] * dy[l] / (size * size / 4));
							double ratio = 1 - natGaussian(d, 0.5);
							sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
							ncount += ratio;
						}
						if (ncount>0)
						{
							sum /= ncount;
						}
						//cout << "   sum = " << sum << endl;
						if (min > sum && ncount>0)
						{
							min = sum;
							minp = Point(cpx, cpy);
						}
					}
				}
			}
			Mat toshow = source.clone();
			if (minp != Point(-1, -1))
			{
				ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(minp.y, minp.x)[0];
				ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(minp.y, minp.x)[1];
				ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(minp.y, minp.x)[2];
				x_source.at<int>(i, j) = minp.x;
				y_source.at<int>(i, j) = minp.y;
				toshow.at<Vec3b>(minp.y, minp.x)[0] = 0;
				toshow.at<Vec3b>(minp.y, minp.x)[1] = 0;
				toshow.at<Vec3b>(minp.y, minp.x)[2] = 255;
			}
			else
			{
				ret.at<Vec3b>(i, j)[0] = 0;
				ret.at<Vec3b>(i, j)[1] = 0;
				ret.at<Vec3b>(i, j)[2] = 255;
			}
			//cout << "End" << endl;
			if (1)
			{
				for (int l = 0; l < pos; l++)
				{
					int cplx = minp.x + dx[l];
					int cply = minp.y + dy[l];
					int xx = j + dx[l];
					int yy = i + dy[l];
					if (xx < 0 || xx >= source.cols || yy < 0 || yy >= height)
					{
						retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = 0;
						retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = 0;
						retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = 0;
						continue;
					}

					int r1 = ret.at<Vec3b>(yy, xx)[0];
					int g1 = ret.at<Vec3b>(yy, xx)[1];
					int b1 = ret.at<Vec3b>(yy, xx)[2];

					retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = r1;
					retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = g1;
					retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = b1;
				}

				for (int l = 0; l < pos; l++)
				{
					int cplx = minp.x + dx[l];
					int cply = minp.y + dy[l];
					int xx = j + dx[l];
					int yy = i + dy[l];
					if (cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
					{
						oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = 0;
						oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = 0;
						oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = 0;
						continue;
					}

					int r2 = source.at<Vec3b>(cply, cplx)[0];
					int g2 = source.at<Vec3b>(cply, cplx)[1];
					int b2 = source.at<Vec3b>(cply, cplx)[2];

					oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = r2;
					oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = g2;
					oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = b2;
				}

				Mat ret2s;
				resize(ret, ret2s, ret.size() * 3, 0.0, 0.0, CV_INTER_NN);
				resize(toshow, toshow, toshow.size() * 3, 0.0, 0.0, CV_INTER_NN);
				resize(retL, retL, retL.size() * 25, 0.0, 0.0, CV_INTER_NN);
				resize(oriL, oriL, oriL.size() * 25, 0.0, 0.0, CV_INTER_NN);
				imshow("Now ret", ret2s);
				//imshow("From", toshow);
				//imshow("retL", retL);
				//imshow("oriL", oriL);
				waitKey(1);
			}
		}
	}

	ret = ret.t();
	x_source = x_source.t();
	y_source = y_source.t();
	int t = height;
	height = width;
	width = t;

	for (int i = source.cols - delta; i < height; i++)
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
				int cx = x_source.at<int>(yy, xx) - dy[p];
				int cy = y_source.at<int>(yy, xx) - dx[p];
				//cout << "  cy: " << cy << ", cx: " << cx << endl;
				if (cx < 0 || cx >= source.cols || cy < 0 || cy >= source.rows)
				{
					continue;
				}
				//cout << "  Canidate " << cy << ", " << cx << endl;
				can.push_back(Point(cx, cy));
			}
			double min = 1E20;
			Point minp = Point(-1, -1);
			//cout << "can.size() = " << can.size() << endl;
			Mat retL(size, size, CV_8UC3, Scalar::all(255));
			Mat oriL(size, size, CV_8UC3, Scalar::all(255));
			retL.at<Vec3b>(size / 2, size / 2)[0] = 0;
			retL.at<Vec3b>(size / 2, size / 2)[1] = 0;
			retL.at<Vec3b>(size / 2, size / 2)[2] = 255;
			oriL.at<Vec3b>(size / 2, size / 2)[0] = 255;
			oriL.at<Vec3b>(size / 2, size / 2)[1] = 0;
			oriL.at<Vec3b>(size / 2, size / 2)[2] = 0;

			int count = 0;
			for (int p = 0; p < can.size(); p++)
			{
				//cout << "  Calculating candidate #" << p << " @ " << cpy << ", " << cpx << endl;
				double ncount = 0;
				double sum = 0;
				int cpx = can[p].x;
				int cpy = can[p].y;
				for (int l = 0; l < pos; l++)
				{
					int cplx = cpx + dy[l];
					int cply = cpy + dx[l];
					int xx = j + dx[l];
					int yy = i + dy[l];
					if (cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
					{
						break;
					}
					if (xx < 0 || xx >= width || yy < 0 || yy >= height)
					{
						continue;
					}
					int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
					int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
					int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
					double d = sqrt(dx[l] * dx[l] / (size * size / 4) + dy[l] * dy[l] / (size * size / 4));
					double ratio = 1 - natGaussian(d, 0.5);
					sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
					ncount += ratio;
				}
				if (ncount>0)
				{
					sum /= ncount;
					count++;
				}
				//cout << "   sum = " << sum << endl;
				if (min > sum && ncount>0)
				{
					min = sum;
					minp = Point(cpx, cpy);
				}
			}

			if (minp == Point(-1, -1) || count < size)
			{
				for (int cpy = delta; cpy < source.rows - delta; cpy++)
				{
					for (int cpx = delta; cpx < source.cols - delta; cpx++)
					{
						double ncount = 0;
						double sum = 0;
						for (int l = 0; l < pos; l++)
						{
							int cplx = cpx + dy[l];
							int cply = cpy + dx[l];
							int xx = j + dx[l];
							int yy = i + dy[l];
							if (cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
							{
								break;
							}
							if (xx < 0 || xx >= width || yy < 0 || yy >= height)
							{
								continue;
							}
							int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
							int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
							int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
							double d = sqrt(dx[l] * dx[l] / (size * size / 4) + dy[l] * dy[l] / (size * size / 4));
							double ratio = 1 - natGaussian(d, 0.5);
							sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
							ncount += ratio;
						}
						if (ncount>0)
						{
							sum /= ncount;
						}
						//cout << "   sum = " << sum << endl;
						if (min > sum && ncount>0)
						{
							min = sum;
							minp = Point(cpx, cpy);
						}
					}
				}
			}
			Mat toshow = source.clone();
			if (minp != Point(-1, -1))
			{
				ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(minp.y, minp.x)[0];
				ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(minp.y, minp.x)[1];
				ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(minp.y, minp.x)[2];
				x_source.at<int>(i, j) = minp.x;
				y_source.at<int>(i, j) = minp.y;
				toshow.at<Vec3b>(minp.y, minp.x)[0] = 0;
				toshow.at<Vec3b>(minp.y, minp.x)[1] = 0;
				toshow.at<Vec3b>(minp.y, minp.x)[2] = 255;
			}
			else
			{
				ret.at<Vec3b>(i, j)[0] = 0;
				ret.at<Vec3b>(i, j)[1] = 0;
				ret.at<Vec3b>(i, j)[2] = 255;
			}
			//cout << "End" << endl;
			if (1)
			{
				Mat ret2s;
				resize(ret, ret2s, ret.size() * 3, 0.0, 0.0, CV_INTER_NN);
				imshow("Now ret", ret2s.t());
				waitKey(1);
			}
		}
	}

	
	flip(ret, ret, 0);
	flip(x_source, x_source, 0);
	flip(y_source, y_source, 0);

	for (int i = height - size; i < height; i++)
	{
		for (int j = source.cols; j < width; j++)
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
				int cx = x_source.at<int>(yy, xx) + dy[p];
				int cy = y_source.at<int>(yy, xx) - dx[p];
				//cout << "  cy: " << cy << ", cx: " << cx << endl;
				if (cx < 0 || cx >= source.cols || cy < 0 || cy >= source.rows)
				{
					continue;
				}
				//cout << "  Canidate " << cy << ", " << cx << endl;
				can.push_back(Point(cx, cy));
			}
			double min = 1E20;
			Point minp = Point(-1, -1);
			//cout << "can.size() = " << can.size() << endl;
			Mat retL(size, size, CV_8UC3, Scalar::all(255));
			Mat oriL(size, size, CV_8UC3, Scalar::all(255));
			retL.at<Vec3b>(size / 2, size / 2)[0] = 0;
			retL.at<Vec3b>(size / 2, size / 2)[1] = 0;
			retL.at<Vec3b>(size / 2, size / 2)[2] = 255;
			oriL.at<Vec3b>(size / 2, size / 2)[0] = 255;
			oriL.at<Vec3b>(size / 2, size / 2)[1] = 0;
			oriL.at<Vec3b>(size / 2, size / 2)[2] = 0;

			int count = 0;
			for (int p = 0; p < can.size(); p++)
			{
				//cout << "  Calculating candidate #" << p << " @ " << cpy << ", " << cpx << endl;
				double ncount = 0;
				double sum = 0;
				int cpx = can[p].x;
				int cpy = can[p].y;
				for (int l = 0; l < pos; l++)
				{
					int cplx = cpx - dy[l];
					int cply = cpy + dx[l];
					int xx = j + dx[l];
					int yy = i + dy[l];
					if (cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
					{
						break;
					}
					if (xx < 0 || xx >= width || yy < 0 || yy >= height)
					{
						continue;
					}
					int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
					int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
					int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
					double d = sqrt(dx[l] * dx[l] / (size * size / 4) + dy[l] * dy[l] / (size * size / 4));
					double ratio = 1 - natGaussian(d, 0.5);
					sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
					ncount += ratio;
				}
				if (ncount>0)
				{
					sum /= ncount;
					count++;
				}
				//cout << "   sum = " << sum << endl;
				if (min > sum && ncount>0)
				{
					min = sum;
					minp = Point(cpx, cpy);
				}
			}

			if (minp == Point(-1, -1) || count < size)
			{
				for (int cpy = delta; cpy < source.rows - delta; cpy++)
				{
					for (int cpx = delta; cpx < source.cols - delta; cpx++)
					{
						double ncount = 0;
						double sum = 0;
						for (int l = 0; l < pos; l++)
						{
							int cplx = cpx - dy[l];
							int cply = cpy + dx[l];
							int xx = j + dx[l];
							int yy = i + dy[l];
							if (cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
							{
								break;
							}
							if (xx < 0 || xx >= width || yy < 0 || yy >= height)
							{
								continue;
							}
							int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
							int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
							int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
							double d = sqrt(dx[l] * dx[l] / (size * size / 4) + dy[l] * dy[l] / (size * size / 4));
							double ratio = 1 - natGaussian(d, 0.5);
							sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
							ncount += ratio;
						}
						if (ncount>0)
						{
							sum /= ncount;
						}
						//cout << "   sum = " << sum << endl;
						if (min > sum && ncount>0)
						{
							min = sum;
							minp = Point(cpx, cpy);
						}
					}
				}
			}
			Mat toshow = source.clone();
			if (minp != Point(-1, -1))
			{
				ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(minp.y, minp.x)[0];
				ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(minp.y, minp.x)[1];
				ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(minp.y, minp.x)[2];
				x_source.at<int>(i, j) = minp.x;
				y_source.at<int>(i, j) = minp.y;
				toshow.at<Vec3b>(minp.y, minp.x)[0] = 0;
				toshow.at<Vec3b>(minp.y, minp.x)[1] = 0;
				toshow.at<Vec3b>(minp.y, minp.x)[2] = 255;
			}
			else
			{
				ret.at<Vec3b>(i, j)[0] = 0;
				ret.at<Vec3b>(i, j)[1] = 0;
				ret.at<Vec3b>(i, j)[2] = 255;
			}
			//cout << "End" << endl;
			if (1)
			{
				Mat ret2s;
				flip(ret, ret2s, 0);
				resize(ret2s, ret2s, ret.size() * 3, 0.0, 0.0, CV_INTER_NN);
				imshow("Now ret", ret2s.t());
				waitKey(1);
			}
		}
	}


	waitKey();

	return ret;
}