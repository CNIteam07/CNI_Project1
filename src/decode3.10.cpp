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

#define low_gray_bound 150//�ڱ߽�
#define high_gray_bound 150//�ױ߽�

bool FindQrPoint(Mat& srcImg, vector<vector<Point>>& qrPoint);
bool IsQrPoint(vector<Point>& contour, Mat& image);
bool IsQrColorRate(vector<Point>& contour, Mat& image);
bool IsQrColorRateX(vector<Point>& contour, Mat& image);
bool IsQrColorRateY(vector<Point>& contour, Mat& image);
void AdjustQrpoint(vector<vector<Point>>& qrPoint);
int GetGrayScale(Mat& image, double row, double col);
void Decode(Mat& image, vector<vector<Point>>& qrPoint, int* Code);
bool Is_empty(Mat& image, vector<vector<Point>>& qrPoint);
int Real_Scale2(Mat& image, double row, double col);
void Decode_new(Mat& image, vector<vector<Point>>& qrPoint, int* Code, int& imnum);
int Real_bit(int x1, int x2, int x3);
int Real_Scale(Mat& image, double row, double col, double ddw, double ddh);

//��������, ɸѡ��������ά�붥��
bool FindQrPoint(Mat& srcImg, vector<vector<Point>>& qrPoint)//����srcImg�Ķ�������qrPoint
{//��ɫͼת�Ҷ�ͼ
	Mat src_gray;
	cvtColor(srcImg, src_gray, CV_BGR2GRAY);

	//��ֵ��
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();

	//���ò�����������
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
	/*void findContours(InputOutputArray image, OutputArrayOfArrays contours, OutputArray hierarchy,
	int mode, int method, Point offset = Point())*/
	//1��image:����ͼ����8 - bit�ĵ�ͨ����ֵͼ�񣬷��������ض��ᱻ����1��
	//2��contours : ���⵽����������һ��������������ÿ��Ԫ�ض���һ�����������ˣ�����������ÿ��Ԫ������һ��������vector<vector<Point> > contours;
	//3��hierarchy:���������ļ̳й�ϵ��hierarchyҲ��һ�����������Ⱥ�contours���ȣ�ÿ��Ԫ�غ�contours��Ԫ�ض�Ӧ��
	//	hierarchy[i][0], hierarchy[i][1], hierarchy[i][2], hierarchy[i][3], �ֱ���ʾ���ǵ�i������(contours[i])����һ����ǰһ���������ĵ�һ������(��һ��������)�Ͱ�����������(������)��

	//�����������ԣ�hierarchy[i][2]��ʾ��������hierachy[i][3]��ʾ������

	//4��mod: ���������ķ����������ַ�����
	//5��method : ��ʾһ�������ķ�����
	//6��offset : ��ѡ��ƫ�ƣ����Ǽ򵥵�ƽ�ƣ��ر���������ROI����֮�����á�
	Mat image(2000, 1500, CV_8UC1, Scalar(0));
	Mat image2(2000, 1500, CV_8UC1, Scalar(0));
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < contours[i].size(); ++j) {
			image.at<uchar>(contours[i].at(j)) = 255;
		}
		bool isQr = IsQrPoint(contours[i], threshold_output_copy);
		//�����ҵ���������ɫ��λ��
		if (isQr)
		{
			for (int j = 0; j < contours[i].size(); ++j) {
				image2.at<uchar>(contours[i].at(j)) = 255;
			}
			qrPoint.push_back(contours[i]);

		}
	}
	imwrite("e:/lunkuo/lunkuo.png", image);
	imwrite("e:/lunkuo/dingweidian.png", image2);
	if (qrPoint.size() < 3)
	{
		cout << "cannot find." << endl;
		return false;
	}
	AdjustQrpoint(qrPoint);
	return 1;
}

bool IsQrPoint(vector<Point>& contours, Mat& image)
{
	
	bool result = IsQrColorRate(contours, image);
	return result;
}


bool IsQrColorRate(vector<Point>& contours, Mat& image)
{
	bool x = IsQrColorRateX(contours, image);
	if (!x)
	{
		return false;
	}
	bool y = IsQrColorRateY(contours, image);
	return y;
}
//�����ڰױ����ж�
bool IsQrColorRateX(vector<Point>& contours, Mat& image)
{
	int i;
	int count = 0;
	int x1 = 0, x2 = contours.size() / 2;
	int nr = contours[x1].y + (contours[x2].y - contours[x1].y) / 2;//nr��ʾ�м����к�
	int nc = contours[x2].x - contours[x1].x;//横向长度
	int dw = nc / 7;
	int ddw = dw / 6;
	if (ddw == 0)
		return false;
	/*if (GetGrayScale(image, nr, contours[0].x + d / 2) < low_gray_bound && GetGrayScale(image, nr, contours[0].x + 3*d / 2) > high_gray_bound&&
		GetGrayScale(image, nr, contours[0].x + 5*d / 2) < low_gray_bound &&GetGrayScale(image, nr, contours[0].x + 7*d / 2) < low_gray_bound &&
		GetGrayScale(image, nr, contours[0].x + 9*d / 2) < low_gray_bound &&GetGrayScale(image, nr, contours[0].x + 11*d / 2) > high_gray_bound&&
		GetGrayScale(image, nr, contours[0].x + 13*d / 2) < low_gray_bound)
		return true;*/
	for (i = contours[x1].y ; i < contours[x2].y ; i+=1) {
		if (!(Real_Scale2(image, i, contours[0].x + dw / 2) == 0 &&
			Real_Scale2(image, i, contours[0].x + 3 * dw / 2) == 1 &&
			Real_Scale2(image, i, contours[0].x + 5 * dw / 2) == 0 &&
			Real_Scale2(image, i, contours[0].x + 7 * dw / 2) == 0 &&
			Real_Scale2(image, i, contours[0].x + 9 * dw / 2) == 0 &&
			Real_Scale2(image, i, contours[0].x + 11 * dw / 2) == 1 &&
			Real_Scale2(image, i, contours[0].x + dw / 2) == 0))
		{
			continue;
		}
		else return true;
}
	if (i == contours[x2].y)
	{
		return false;
	}
}
//�����ڰױ����ж�
bool IsQrColorRateY(vector<Point>& contours, Mat& image)
{
	int i;
	int count=0;
	int x2 = 0, x1 = contours.size() / 2;
	double nr = contours[x2].x + (contours[x1].x - contours[x2].x) / 2;//nr��ʾ�м����к�
	double nc = contours[x1].y - contours[x2].y;//nc��ʾ�߶�
	double dh = double(nc) / 7.0;
	double ddh = dh / 6;
	if (ddh == 0)
		return false;

	/*if (GetGrayScale(image, contours[0].y + d / 2,nr) < low_gray_bound && GetGrayScale(image, contours[0].y + 3*d / 2, nr) > high_gray_bound&&
		GetGrayScale(image, contours[0].y + 5*d / 2, nr) < low_gray_bound && GetGrayScale(image, contours[0].y + 7*d / 2, nr) < low_gray_bound &&
		GetGrayScale(image, contours[0].y + 9*d / 2, nr) < low_gray_bound && GetGrayScale(image, contours[0].y + 11*d / 2, nr) > high_gray_bound&&
		GetGrayScale(image, contours[0].y + 13*d / 2, nr) < low_gray_bound)
		return true;*/
	
	for ( i = contours[x1].x ; i < contours[x2].x ; i+=1) {
		if (!(Real_Scale2(image, contours[0].y + dh / 2, i) == 0 &&
			Real_Scale2(image, contours[0].y + 3 * dh / 2, i) == 1 &&
			Real_Scale2(image, contours[0].y + 5 * dh / 2, i) == 0 &&
			Real_Scale2(image, contours[0].y + 7 * dh / 2, i) == 0 &&
			Real_Scale2(image, contours[0].y + 9 * dh / 2, i) == 0 &&
			Real_Scale2(image, contours[0].y + 11 * dh / 2, i) == 1 &&
			Real_Scale2(image, contours[0].y + 13 * dh / 2, i) == 0))

		{
			continue;
		}
		else return true;
	}
	if (i == contours[x2].x)
	{
		return false;
	}
}

//����3��������˳��(����pt0������pt1������pt2)
void AdjustQrpoint(vector<vector<Point>>& qrPoint)
{
	int pt0, pt1, pt2;
	vector<vector<Point>> qrPoint_temp[3];
	for (int i = 0; i < 3; i++)
	{
		if (qrPoint[i][0].x >= qrPoint[0][0].x && qrPoint[i][0].x >= qrPoint[1][0].x
			&& qrPoint[i][0].x >= qrPoint[2][0].x)
			pt1 = i;//x������Ϊ����
		else if (qrPoint[i][0].y >= qrPoint[0][0].y && qrPoint[i][0].y >= qrPoint[1][0].y
			&& qrPoint[i][0].y >= qrPoint[2][0].y)
			pt2 = i;//y������Ϊ����
		else
			pt0 = i;//ʣ�µ�Ϊ����
	}
	qrPoint_temp->push_back(qrPoint[pt0]);
	qrPoint_temp->push_back(qrPoint[pt1]);
	qrPoint_temp->push_back(qrPoint[pt2]);
	qrPoint.assign(qrPoint_temp->begin(), qrPoint_temp->end());//�ѽ����������ݿ�����qrPoint
}
//���ûҶ�ֵ
int GetGrayScale(Mat& image, double row, double col)
{
	uchar* data = image.ptr<uchar>(row);
	int Scale = data[(int)col];
	return Scale;
}
//����
void Decode(Mat& image, vector<vector<Point>>& qrPoint, int* Code)
{
	int codenum = 0;//��¼����λ��

	//��ɫͼת�Ҷ�ͼ
	Mat src_gray;
	cvtColor(image, src_gray, CV_BGR2GRAY);

	//��ֵ��
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();

	double temp = qrPoint[0].size();
	double white_wid = (qrPoint[0][temp / 2].x - qrPoint[0][0].x) * 2 / 21;//��10/105��
	double row0 = qrPoint[0][temp / 2].y + white_wid,
		col0 = qrPoint[0][temp / 2].x + white_wid,
		row1 = qrPoint[2][0].y - white_wid,
		col1 = qrPoint[1][0].x - white_wid;
	double dw = (col1 - col0) / 64.0,
		dh = (row1 - row0) / 64.0;
	double ddw = dw / 6,//ddw��ddh������ÿ��Сɫ�������ٶ���ȡ�㣬����ʶ����׼ȷ��
		ddh = dh / 6;

	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			Code[codenum++] = Real_Scale(threshold_output_copy, row0 + dh * (2.0 * i + 1) / 2, col0 + dw * (2.0 * j + 1) / 2, ddw, ddh);
		}
	}
}

int Real_Scale(Mat& image, double row, double col, double ddw, double ddh)
{
	int num = 0;
	for (double i = row - ddw; i <= row + ddw; i += ddw)
	{
		for (double j = col - ddh; j < col + ddh; j += ddh)
		{
			if (GetGrayScale(image, i, j) < low_gray_bound)
				num++;
		}
	}
	if (num >= 5)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
int Real_Scale2(Mat& image, double row, double col)
{
	int num = 0;
	int i = row;
	int j = col;
    if (GetGrayScale(image, i, j) < low_gray_bound)
	num++;
		
	if (num ==1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
//�ж϶�ά���Ƿ�Ϊ��
bool Is_empty(Mat& image, vector<vector<Point>>& qrPoint)
{
	int* Code_temp = new int[5000];
	Decode(image, qrPoint, Code_temp);
	int white_num = 0;//��¼�׵�����
	for (int i = 0; i < 4096; i += 10)
	{
		if (Code_temp[i] == 1)
			white_num++;
	}
	delete[]Code_temp;
	if (white_num > 390)
	{
		cout << "is empty"<<endl;
		return true;
	}
	else
		return false;
}

//һ�ν�������ͼ
void Decode_new(Mat& image, vector<vector<Point>>& qrPoint, int* Code, int& imnum)
{
	int* Code1 = new int[5000];
	int* Code2 = new int[5000];
	int* Code3 = new int[5000];

	//������������ͼ��bit��Ϣ������Code1��Code2��Code3��
	char filename1[15];
	sprintf_s(filename1, 15, "x%d.png", imnum++);
	fstream inFile1(filename1);
	if (!inFile1.eof())
		Decode(image, qrPoint, Code1);

	char filename2[15];
	sprintf_s(filename2, 15, "x%d.png", imnum++);
	fstream inFile2(filename2);
	if (!inFile2.eof())
		Decode(image, qrPoint, Code2);

	imnum++;
	char filename3[15];
	sprintf_s(filename3, 15, "x%d.png", imnum++);
	fstream inFile3(filename3);
	if (!inFile3.eof())
		Decode(image, qrPoint, Code3);

	//��λ�Ƚ�Code1��Code2��Code3������ʵ��Ϣ������Code��
	for (int i = 0; i < 4096; i++)
	{
		Code[i] = Real_bit(Code1[i], Code2[i], Code3[i]);
	}
	delete[]Code1;
	delete[]Code2;
	delete[]Code3;

	//��ԭΪ�ַ�����������output.txt
	int temp = 0;
	char temp_char;
	for (int i = 0; i < 4096; i += 8)
	{
		for (int j = 7; j >= 0; j--)
			temp = temp * 2 + Code[i + j];
		temp_char = (char)temp;
		cout << temp_char;
		temp = 0;
	}

	return;
}
//����λ���ж�һ��bit��ʵ��Ϣ
int Real_bit(int x1, int x2, int x3)
{
	int temp = 0;//��¼0�ĸ���
	if (x1 == 0)temp++;
	if (x2 == 0)temp++;
	if (x3 == 0)temp++;

	if (temp <= 2)
		return 0;
	else
		return 1;
}


int main()
{
	char filename[50];//������ʱ�洢�ļ���
	int codenum = 0;
	int imnum = 1;//��¼������ͼƬ����
	int Begin_flag = 0;//0��ʾǰͬ���룬1��ʾ������
	int count1 = 0;
	int count2 = 0;
//WinExec("./ffmpeg.exe -i output.mp4 -r 30 x%d.png", SW_SHOW);

	while (1)
	{
		int* Code = new int[5000];
		sprintf_s(filename, 50, "e:/hi/x%d.png", imnum++);
		fstream inFile(filename);
		if (inFile.good())
		{
			Mat img = imread(filename);
			vector<vector<Point>> qrPoint;
			if (FindQrPoint(img, qrPoint))
			{
			
				if (Is_empty(img, qrPoint))//�ж�λ����Ϊ��
				{

					if (Begin_flag == 0)//�ǿ�ʼ��
					{
						
						continue;
					}//����һ��	
					else//�ǽ�����
						return 0;//�˳�
				}
				else//�ж�λ���Ҳ���
				{
					Begin_flag = 1;//����������
					imnum-=2;
					cout << imnum << endl;
					Decode_new(img, qrPoint, Code, imnum);
				}
			}

		}
		else
			break;

		delete[]Code;
	}

	waitKey(0);
	return 0;
}
