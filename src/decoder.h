#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/types_c.h>
#include<opencv2/highgui/highgui_c.h>
#include<vector>
#include<stdlib.h>
#include<fstream>
#include<Windows.h>
#define _DST_PATH "./"
using namespace cv;
using namespace std;

#define low_gray_bound 130//黑边界
#define high_gray_bound 130//白边界

class Functions {
};
class SearchQrcodeFunctions:public Functions
{
public:
	bool FindQrPoint(Mat& srcImg, vector<vector<Point>>& qrPoint);
	inline bool IsQrPoint(vector<Point>& contour, Mat& image);
	inline bool IsQrColorRate(vector<Point>& contour, Mat& image);
	bool IsQrColorRateX(vector<Point>& contour, Mat& image);
	bool IsQrColorRateY(vector<Point>& contour, Mat& image);
	void AdjustQrpoint(vector<vector<Point>>& qrPoint);
	int Real_ScaleX(Mat& image, int line, int row_begin, int row_end);
	int Real_ScaleY(Mat& image, int row, int line_begin, int line_end);
	int GetGrayScale(Mat& image, double row, double col);
	bool Is_empty(Mat& image, vector<vector<Point>>& qrPoint);
};
class DecodingFunctions :public Functions
{
public:
	void Decode(Mat& image, vector<vector<Point>>& qrPoint, int* Code);
	void Decode_new( int* Code, int& imnum,string path, string decodefilename, string voutfilename);
	int Real_bit(int x1, int x2, int x3);
	int check(int code_check[12]);
};
class ImageOperateFunctions:public Functions
{
public:
	void Adjust_Contrast(Mat& image);
};

/*Function为保留基类，提供了公有函数*/
/*SearchQrcodeFunctions类提供了所有查找识别二维码的方法*/
/*DecodingFunctions类提供了所有解码的方法*/
/*ImageOperateFunctions类提供了所有图像处理的方法*/
