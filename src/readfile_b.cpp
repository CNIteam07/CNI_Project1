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

using namespace cv;
using namespace std;

bool FindQrPoint(Mat& srcImg, vector<vector<Point>>& qrPoint);
bool IsQrPoint(vector<Point>& contour, Mat& image);
bool IsQrColorRate(vector<Point>& contour, Mat& image);
bool IsQrColorRateX(vector<Point>& contour, Mat& image);
bool IsQrColorRateY(vector<Point>& contour, Mat& image);
void AdjustQrpoint(vector<vector<Point>>& qrPoint);
int GetGrayScale(Mat& image, int row, int col);

void Decode(Mat& image, vector<vector<Point>>& qrPoint);

int *Code=new int[1000000];
int codenum = 0;//记录解码位数


//查找轮廓, 筛选出三个二维码顶点
bool FindQrPoint(Mat & srcImg, vector<vector<Point>> & qrPoint)//查找srcImg的顶点存入qrPoint
{//彩色图转灰度图
	Mat src_gray;
	cvtColor(srcImg, src_gray, CV_BGR2GRAY);

	//二值化
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();
	
	//调用查找轮廓函数
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
	/*void findContours(InputOutputArray image, OutputArrayOfArrays contours, OutputArray hierarchy, 
	int mode, int method, Point offset = Point())*/
	//1、image:输入图像。8 - bit的单通道二值图像，非零的像素都会被当作1。
	//2、contours : 检测到的轮廓。是一个向量，向量的每个元素都是一个轮廓。因此，这个向量的每个元素仍是一个向量。vector<vector<Point> > contours;
	//3、hierarchy:各个轮廓的继承关系。hierarchy也是一个向量，长度和contours相等，每个元素和contours的元素对应。
	//	hierarchy[i][0], hierarchy[i][1], hierarchy[i][2], hierarchy[i][3], 分别表示的是第i条轮廓(contours[i])的下一条，前一条，包含的第一条轮廓(第一条子轮廓)和包含他的轮廓(父轮廓)。
	
	//更正：经测试，hierarchy[i][2]表示子轮廓，hierachy[i][3]表示父轮廓

	//4、mod: 检测轮廓的方法。有四种方法。
	//5、method : 表示一条轮廓的方法。
	//6、offset : 可选的偏移，就是简单的平移，特别是在做了ROI步骤之后有用。


	////通过黑色定位角作为父轮廓，有两个子轮廓的特点，筛选出三个定位角
	//int parentIdx = -1;
	//int ic = 0;
	
	for (int i = 0; i < contours.size(); i++)
	{
		//if (hierarchy[i][2] != -1 && ic == 0)
		//{
		//	parentIdx = i;
		//	ic++;
		//}
		//else if (hierarchy[i][2] != -1)
		//{
		//	ic++;
		//}
		//else if (hierarchy[i][2] == -1)
		//{
		//	ic = 0;
		//	parentIdx = -1;
		//}

		////有两个子轮廓才是二维码的顶点
		//if (ic >= 2)
		//{
			//bool isQr = IsQrPoint(contours[parentIdx],threshold_output_copy);
		bool isQr = IsQrPoint(contours[i], threshold_output_copy);
			//保存找到的三个黑色定位角
			if (isQr)
			{
				//qrPoint.push_back(contours[parentIdx]);
				qrPoint.push_back(contours[i]);
				//ic = 0;
				//parentIdx = -1;
			}
		//}
	}

	if (qrPoint.size() < 3)
		return false;
	AdjustQrpoint(qrPoint);
	return true;
}

//找到了两个轮廓的图元，需要进一步分析是不是二维码顶点
bool IsQrPoint(vector<Point>& contours,Mat& image)
{
	//横向黑白比例1:1:3:1:1
	bool result = IsQrColorRate(contours,image);
	return result;
}

//黑白比例判断函数
bool IsQrColorRate(vector<Point>& contours,Mat &image)
{
	bool x = IsQrColorRateX(contours,image);
	if (!x)
		return false;
	bool y = IsQrColorRateY(contours,image);
	return y;
}
//横向黑白比例判断
bool IsQrColorRateX(vector<Point>& contours,Mat &image)
{
	int x1 = 0, x2 = contours.size() / 2;
	int nr = contours[x1].y+(contours[x2].y-contours[x1].y) / 2;//nr表示中间行行号
	int nc = contours[x2].x-contours[x1].x;//nc表示宽度
	int d = nc / 7;

	if (GetGrayScale(image, nr, contours[0].x + d / 2) < 28 && GetGrayScale(image, nr, contours[0].x + 3*d / 2) >207 &&
		GetGrayScale(image, nr, contours[0].x + 5*d / 2) < 28 &&GetGrayScale(image, nr, contours[0].x + 7*d / 2) < 28 && 
		GetGrayScale(image, nr, contours[0].x + 9*d / 2) < 28 &&GetGrayScale(image, nr, contours[0].x + 11*d / 2) >207 && 
		GetGrayScale(image, nr, contours[0].x + 13*d / 2) < 28)
		return true;

	return false;
}
//纵向黑白比例判断
bool IsQrColorRateY(vector<Point> &contours, Mat& image)
{

	int x2 = 0, x1 = contours.size() / 2;
	int nr = contours[x2].x + (contours[x1].x - contours[x2].x) / 2;//nr表示中间列列号
	int nc = contours[x1].y - contours[x2].y;//nc表示高度
	int d = nc / 7;

	if (GetGrayScale(image, contours[0].y + d / 2,nr) < 28 && GetGrayScale(image, contours[0].y + 3*d / 2, nr) > 207 &&
		GetGrayScale(image, contours[0].y + 5*d / 2, nr) < 28 && GetGrayScale(image, contours[0].y + 7*d / 2, nr) < 28 &&
		GetGrayScale(image, contours[0].y + 9*d / 2, nr) < 28 && GetGrayScale(image, contours[0].y + 11*d / 2, nr) > 207 &&
		GetGrayScale(image, contours[0].y + 13*d / 2, nr) < 28)
		return true;

	return false;
}

//修正3个顶点的顺序(左上pt0，右上pt1，左下pt2)
void AdjustQrpoint(vector<vector<Point>>& qrPoint)
{
	int pt0, pt1, pt2;
	vector<vector<Point>> qrPoint_temp[3];
	for (int i = 0; i < 3; i++)
	{
		if (qrPoint[i][0].x >= qrPoint[0][0].x && qrPoint[i][0].x >= qrPoint[1][0].x
			&& qrPoint[i][0].x >= qrPoint[2][0].x)
			pt1 = i;//x最大的为右上
		else if (qrPoint[i][0].y >= qrPoint[0][0].y && qrPoint[i][0].y >= qrPoint[1][0].y
			&& qrPoint[i][0].y >= qrPoint[2][0].y)
			pt2 = i;//y最大的为左下
		else
			pt0 = i;//剩下的为左上
	}
	qrPoint_temp->push_back(qrPoint[pt0]);
	qrPoint_temp->push_back(qrPoint[pt1]);
	qrPoint_temp->push_back(qrPoint[pt2]);
	qrPoint.assign(qrPoint_temp->begin(),qrPoint_temp->end());//把交换完的数据拷贝回qrPoint
}
//获得灰度值
int GetGrayScale(Mat& image, int row, int col)
{
	uchar* data = image.ptr<uchar>(row);
	int Scale = data[col];
	return Scale;
}
//解码
void Decode(Mat& image, vector<vector<Point>>& qrPoint)
{
	//彩色图转灰度图
	Mat src_gray;
	cvtColor(image, src_gray, CV_BGR2GRAY);

	//二值化
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();

	int temp = qrPoint[0].size();
	int white_wid = (qrPoint[0][temp / 2].x - qrPoint[0][0].x)*2/21;//“10/105”
	int row0 = qrPoint[0][temp / 2].y+white_wid,
		col0 = qrPoint[0][temp / 2].x+white_wid,
		row1 = qrPoint[2][0].y-white_wid,
		col1 = qrPoint[1][0].x-white_wid;
	double dw = (col1 - col0)/64.0,
			dh = (row1 - row0)/64.0;
	
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			if (GetGrayScale(threshold_output_copy, row0 + (int)(dh * (2 * i + 1)/2), col0 + (int)(dw * (2 * j + 1)/2)) < 80)//判定为黑
				Code[codenum] = 1;
			if (GetGrayScale(threshold_output_copy, row0 + (int)(dh * (2 * i + 1)/2), col0 + (int)(dw * (2 * j + 1)/2)) > 160)//判定为白
				Code[codenum] = 0;
			codenum++;
		}
	}
}


int main()
{
	char filename[15];//用于临时存储文件名
	codenum = 0;
	int imnum = 1;//记录解码的图片张数

	/*
	WinExec("./ffmpeg.exe -i output.mp4 x%d.png", SW_SHOW);
	WinExec(lpCmdLine, SW_SHOW);
	*/

	while (1)
	{
		sprintf_s(filename, 15, "x%d.png", imnum);
		fstream inFile(filename);
		if (inFile.good())
		{
			Mat img = imread(filename);

			vector<vector<Point>> qrPoint;
			if (FindQrPoint(img, qrPoint))
				Decode(img, qrPoint);
		}
		else
			break;
		imnum++;
	}

	int d = 0;//辅助输出换行
	for (int i = 0; i < codenum; i++)
	{
		cout << Code[i];
		d++;
		if (d == 8)
		{
			d -= 8;
			cout << endl;
		}
	}

	cout << endl;
	int temp = 0;
	char temp_char;

	for (int i = 0; i < codenum; i += 8)
	{
		for (int j = 7; j >= 0; j--)
			temp = temp * 2 + Code[i + j];
		temp_char = (char)temp;
		cout << temp_char;
		temp = 0;
	}

	delete Code;

	waitKey(0);
	return 0;
}
