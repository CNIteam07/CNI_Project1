
#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<cstdlib>
#include<fstream>
#include<opencv2/opencv.hpp>
#include<Windows.h>
using namespace cv;
using namespace std;

class Paint {
public:
	void draw_ROI(Mat& erweima);
};
class BaseFunctions
{
public:
	void change_to_binary(char in, char binary_arrry[60], int& index);
	void ConvertToVideo(string stdpath);
};
class Encode :public Paint,public BaseFunctions {
public:
	bool EncodeMyQrCode(string yuan_file, string limit);
};