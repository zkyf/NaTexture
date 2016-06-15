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

Mat natGenerate(Mat source, Mat mask, int size)
{
	const int delta = size*2;

	size = size / 2 * 2 + 1;
	int height = source.rows;
	int width = source.cols;
	srand((unsigned)time(0));

	Mat ret = source.clone();
	Mat x_source(height, width, CV_32S, Scalar::all(0));
	Mat y_source(height, width, CV_32S, Scalar::all(0));
	Mat valid = mask.clone();
  Mat region;
  Point start;
	int left = width, right = 0, top = height, bottom = 0;
	for (int i = 0; i < source.rows; i++)
	{
		for (int j = 0; j < source.cols; j++)
		{
			if (mask.at<uchar>(i, j) == 0)
			{
				x_source.at<int>(i, j) = j;
				y_source.at<int>(i, j) = i;
			}
			else
			{
        x_source.at<int>(i, j) = -1E20;
        y_source.at<int>(i, j) = -1E20;
				ret.at<Vec3b>(i, j)[0] = 0;
				ret.at<Vec3b>(i, j)[1] = 0;
				ret.at<Vec3b>(i, j)[2] = 0;
        if (i < top)
        {
          top = i;
          start = Point(j, i);
        }
				if (i>bottom) bottom = i;
				if (j < left) left = j;
				if (j>right) right = j;
			}
		}
	}
  region = ret.clone();
	//imshow("random ret", ret);
	//waitKey(0);
	for (int i = top; i <= bottom; i++)
	{
		for (int j = left; j <= right; j++)
		{
			if (mask.at<uchar>(i, j) > 0)
			{
				//cout << "Ret: " << i << ", " << j << endl;
				vector<Point> can;
				int pos = 0;
				int out = 0;
				for (int dy = -size / 2; dy <= size / 2; dy++)
				{
					for (int dx = -size / 2; dx <= size / 2; dx++)
					{
						int yy = i + dy;
						int xx = j + dx;
						if (xx < 0 || xx >= width || yy < 0 || yy >= height)
						{
							continue;
						}
						pos++;
						if (valid.at<uchar>(yy, xx) > 0)
						{
							out++;
							//cout << "Invalid @" << xx << ", " << yy << endl;
							continue;
						}
						int cx = x_source.at<int>(yy, xx) - dx;
						int cy = y_source.at<int>(yy, xx) - dy;
						if (cx < 0 || cx >= width || cy < 0 || cy >= height)
						{
							continue;
						}
						if (valid.at<uchar>(cy, cx) > 0)
						{
							//cout << "Out mask @" << xx << ", " << yy;
							//cout << ", ori from @" << cx << ", " << cy << endl;
							out++;
							continue;
						}
						//cout << "  Canidate " << cy << ", " << cx << endl;
						can.push_back(Point(cx, cy));
					}
				}
				double min = 1E20;
				Point minp = Point(-1, -1);
				cout << "can.size() = " << can.size() << endl;
				//Mat retL(size, size, CV_8UC3, Scalar::all(255));
				//Mat oriL(size, size, CV_8UC3, Scalar::all(255));
				//retL.at<Vec3b>(size / 2, size / 2)[0] = 0;
				//retL.at<Vec3b>(size / 2, size / 2)[1] = 0;
				//retL.at<Vec3b>(size / 2, size / 2)[2] = 255;
				//oriL.at<Vec3b>(size / 2, size / 2)[0] = 255;
				//oriL.at<Vec3b>(size / 2, size / 2)[1] = 0;
				//oriL.at<Vec3b>(size / 2, size / 2)[2] = 0;
				int count = 0;
				int nolessf = 0;
				int nocountf = 0;
				for (int p = 0; p < can.size(); p++)
				{
					//cout << "  Calculating candidate #" << p << " @ " << cpy << ", " << cpx << endl;
					double ncount = 0;
					double sum = 0;
					int cpx = can[p].x;
					int cpy = can[p].y;
					if (mask.at<uchar>(cpy, cpx) > 0) continue;
					for (int dy = -size / 2; dy <= size / 2; dy++)
					{
						for (int dx = -size / 2; dx <= size / 2; dx++)
						{
							int cplx = cpx + dx;
							int cply = cpy + dy;
							int xx = j + dx;
							int yy = i + dy;
							if (cplx < 0 || cplx >= width || cply < 0 || cply >= height)
							{
								continue;
							}
							if (xx < 0 || xx >= width || yy < 0 || yy >= height)
							{
								continue;
							}
							if (valid.at<uchar>(cply, cplx) > 0) continue;
							if (valid.at<uchar>(yy, xx) > 0) continue;
							int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = ret.at<Vec3b>(cply, cplx)[0];
							int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = ret.at<Vec3b>(cply, cplx)[1];
							int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = ret.at<Vec3b>(cply, cplx)[2];
							double d = sqrt(dx * dx / (size * size / 4)
															+ dy * dy / (size * size / 4));
							double ratio = 1 - natGaussian(d, 0.4);
							sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
							ncount += ratio;
						}
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
					else
					{
						if (min <= sum) nolessf++;
						else nocountf++;
					}
				}
				region.at<Vec3b>(i, j)[0] = 255;
				region.at<Vec3b>(i, j)[1] = 0;
				region.at<Vec3b>(i, j)[2] = 0;
				//minp = Point(-1, -1);
				if (minp == Point(-1, -1) || count < size || min >= 200)
				{
					cout << "No sum @" << j << ", " << i << endl;
					//cout << "pos  = " << pos << ", out = " << out << endl;
					//cout << "nolessf = " << nolessf << ", nocountf = " << nocountf << endl;
					//system("pause");
					min = 1E20;
					for (int oy = i - delta; oy <= i + delta; oy++)
					{
						if (oy >= height) break;
						for (int ox = j - delta; ox <= j + delta; ox++)
						{
							if (ox >= width) break;
							if (valid.at<uchar>(oy, ox) > 0) continue;
							int cpx = x_source.at<int>(oy, ox);
							int cpy = y_source.at<int>(oy, ox);
							if (cpx < 0 || cpx >= width || cpy < 0 || cpy >= height)
							{
								continue;
							}
							if (mask.at<uchar>(cpy, cpx) > 0) continue;
							double ncount = 0;
							double sum = 0;
							for (int dy = -size / 2; dy <= size / 2; dy++)
							{
								for (int dx = -size / 2; dx <= size / 2; dx++)
								{
									int cplx = cpx + dx;
									int cply = cpy + dy;
									int xx = j + dx;
									int yy = i + dy;
									if (cplx < 0 || cplx >= width || cply < 0 || cply >= height)
									{
										continue;
									}
									if (xx < 0 || xx >= width || yy < 0 || yy >= height)
									{
										continue;
									}
									if (valid.at<uchar>(cply, cplx) > 0) continue;
									if (valid.at<uchar>(yy, xx) > 0) continue;
									int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = ret.at<Vec3b>(cply, cplx)[0];
									int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = ret.at<Vec3b>(cply, cplx)[1];
									int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = ret.at<Vec3b>(cply, cplx)[2];
									double d = sqrt(dx * dx / (size * size / 4) + dy * dy / (size * size / 4));
									double ratio = 1 - natGaussian(d, 0.5);
									sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
									ncount += ratio;
								}
							}
							if (ncount>0)
							{
								sum /= ncount;
							}
							//cout << "   sum = " << sum << endl;
							if (min > sum && ncount > 0)
							{
								min = sum;
								minp = Point(cpx, cpy);
							}
						}
					}
					region.at<Vec3b>(i, j)[0] = 0;
					region.at<Vec3b>(i, j)[1] = 255;
					region.at<Vec3b>(i, j)[2] = 0;
				}
				if (minp == Point(-1, -1))
				{
					cout << "No sum again @" << j << ", " << i << endl;
					//system("pause");
					min = 1E20;
					for (int cpy = 0; cpy < height; cpy++)
					{
						for (int cpx = 0; cpx < width; cpx++)
						{
							if (cpx >= width) break;
							if (mask.at<uchar>(cpy, cpx) > 0) continue;
							double ncount = 0;
							double sum = 0;
							for (int dy = -size / 2; dy <= size / 2; dy++)
							{
								for (int dx = -size / 2; dx <= size / 2; dx++)
								{
									int cplx = cpx + dx;
									int cply = cpy + dy;
									int xx = j + dx;
									int yy = i + dy;
									if (cplx < 0 || cplx >= width || cply < 0 || cply >= height)
									{
										continue;
									}
									if (xx < 0 || xx >= width || yy < 0 || yy >= height)
									{
										continue;
									}
									if (mask.at<uchar>(cply, cplx) > 0) continue;
									if (valid.at<uchar>(yy, xx) > 0) continue;
									int r1 = ret.at<Vec3b>(yy, xx)[0], r2 = source.at<Vec3b>(cply, cplx)[0];
									int g1 = ret.at<Vec3b>(yy, xx)[1], g2 = source.at<Vec3b>(cply, cplx)[1];
									int b1 = ret.at<Vec3b>(yy, xx)[2], b2 = source.at<Vec3b>(cply, cplx)[2];
									double d = sqrt(dx * dx / (size * size / 4) + dy * dy / (size * size / 4));
									double ratio = 1 - natGaussian(d, 0.5);
									sum += ((r1 - r2)*(r1 - r2) + (g1 - g2)*(g1 - g2) + (b1 - b2)*(b1 - b2)) * ratio;
									ncount += ratio;
								}
							}
							if (ncount>0)
							{
								sum /= ncount;
							}
							//cout << "   sum = " << sum << endl;
							if (min > sum && ncount > 0 && mask.at<uchar>(cpy, cpx) == 0)
							{
								min = sum;
								minp = Point(cpx, cpy);
							}
						}
					}
					region.at<Vec3b>(i, j)[0] = 255;
					region.at<Vec3b>(i, j)[1] = 0;
					region.at<Vec3b>(i, j)[2] = 255;
				}
				Mat toshow = source.clone();
				if (minp != Point(-1, -1))
				{
					ret.at<Vec3b>(i, j)[0] = source.at<Vec3b>(minp.y, minp.x)[0];
					ret.at<Vec3b>(i, j)[1] = source.at<Vec3b>(minp.y, minp.x)[1];
					ret.at<Vec3b>(i, j)[2] = source.at<Vec3b>(minp.y, minp.x)[2];
					x_source.at<int>(i, j) = x_source.at<int>(minp);
					y_source.at<int>(i, j) = y_source.at<int>(minp);
					valid.at<uchar>(i, j) = 0;
					toshow.at<Vec3b>(minp.y, minp.x)[0] = 0;
					toshow.at<Vec3b>(minp.y, minp.x)[1] = 0;
					toshow.at<Vec3b>(minp.y, minp.x)[2] = 255;
				}
				else
				{
					ret.at<Vec3b>(i, j)[0] = 0;
					ret.at<Vec3b>(i, j)[1] = 0;
					ret.at<Vec3b>(i, j)[2] = 255;
					cout << "No point @ " << i << ", " << j << endl;
					system("pause");
				}
				//cout << "End" << endl;
				if (1)
				{
					//for (int l = 0; l < pos; l++)
					//{
					//	int cplx = minp.x + dx[l];
					//	int cply = minp.y + dy[l];
					//	int xx = j + dx[l];
					//	int yy = i + dy[l];
					//	if (xx < 0 || xx >= source.cols || yy < 0 || yy >= height)
					//	{
					//		retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = 0;
					//		retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = 0;
					//		retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = 0;
					//		continue;
					//	}

					//	int r1 = ret.at<Vec3b>(yy, xx)[0];
					//	int g1 = ret.at<Vec3b>(yy, xx)[1];
					//	int b1 = ret.at<Vec3b>(yy, xx)[2];

					//	retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = r1;
					//	retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = g1;
					//	retL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = b1;
					//}

					//for (int l = 0; l < pos; l++)
					//{
					//	int cplx = minp.x + dx[l];
					//	int cply = minp.y + dy[l];
					//	int xx = j + dx[l];
					//	int yy = i + dy[l];
					//	if (cplx < 0 || cplx >= source.cols || cply < 0 || cply >= source.rows)
					//	{
					//		oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = 0;
					//		oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = 0;
					//		oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = 0;
					//		continue;
					//	}

					//	int r2 = source.at<Vec3b>(cply, cplx)[0];
					//	int g2 = source.at<Vec3b>(cply, cplx)[1];
					//	int b2 = source.at<Vec3b>(cply, cplx)[2];

					//	oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[0] = r2;
					//	oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[1] = g2;
					//	oriL.at<Vec3b>(dy[l] + size / 2, dx[l] + size / 2)[2] = b2;
					//}

					//Mat ret2s;
					//resize(ret, ret2s, ret.size() * 3, 0.0, 0.0, CV_INTER_NN);
					//resize(toshow, toshow, toshow.size() * 3, 0.0, 0.0, CV_INTER_NN);
					//resize(retL, retL, retL.size() * 25, 0.0, 0.0, CV_INTER_NN);
					//resize(oriL, oriL, oriL.size() * 25, 0.0, 0.0, CV_INTER_NN);
					imshow("Now ret", ret);
					//cout << (int)mask.at<uchar>(minp) << endl;
					imshow("From", toshow);
					imshow("Region", region);
					//imshow("retL", retL);
					//imshow("oriL", oriL);
					waitKey(3);
				}
			}
		}
	}

	int dirx[4] = { 0, 1, 0, -1 };
	int diry[4] = { 1, 0, -1, 0 };
	int lastdir = 0;
	//for (int i = top, j = left; top<=bottom && left <= right; )
 // {



	//	switch (lastdir)
	//	{
	//		case 0: // down
	//			if (i == bottom)
	//			{
	//				left++;
	//				lastdir = 1;
	//			}
	//			break;
	//		case 1: // right
	//			if (j == right)
	//			{
	//				bottom--;
	//				lastdir = 2;
	//			}
	//			break;
	//		case 2: // up
	//			if (i == top)
	//			{
	//				right--;
	//				lastdir = 3;
	//			}
	//			break;
	//		case 3: // left
	//			if (j <= left)
	//			{
	//				top++;
	//				lastdir = 0;
	//			}
	//			break;
	//	}
	//	j += dirx[lastdir];
	//	i += diry[lastdir];
 // }
	return ret;
}