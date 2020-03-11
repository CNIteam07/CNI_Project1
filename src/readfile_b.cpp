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

#define low_gray_bound 130//�ڱ߽�
#define high_gray_bound 130//�ױ߽�

bool FindQrPoint(Mat& srcImg, vector<vector<Point>>& qrPoint);
bool IsQrPoint(vector<Point>& contour, Mat& image);
bool IsQrColorRate(vector<Point>& contour, Mat& image);
bool IsQrColorRateX(vector<Point>& contour, Mat& image);
bool IsQrColorRateY(vector<Point>& contour, Mat& image);
void AdjustQrpoint(vector<vector<Point>>& qrPoint);
int GetGrayScale(Mat& image, double row, double col);
void Decode(Mat& image, vector<vector<Point>>& qrPoint, int* Code);
bool Is_empty(Mat& image, vector<vector<Point>>& qrPoint);

void Decode_new(Mat& image, vector<vector<Point>>& qrPoint, int* Code, int& imnum);
int Real_bit(int x1, int x2, int x3);
int Real_Scale(Mat& image, double row, double col, double ddw, double ddh);
int Real_Scale2(Mat& image, int line, int row_begin, int row_end);
int Real_Scale3(Mat& image, int row, int line_begin, int line_end);

void Adjust_Contrast(Mat& image);

//��������, ɸѡ��������ά�붥��
bool FindQrPoint(Mat & srcImg, vector<vector<Point>> & qrPoint)//����srcImg�Ķ������qrPoint
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
	//1��image:����ͼ��8 - bit�ĵ�ͨ����ֵͼ�񣬷�������ض��ᱻ����1��
	//2��contours : ��⵽����������һ��������������ÿ��Ԫ�ض���һ����������ˣ����������ÿ��Ԫ������һ��������vector<vector<Point> > contours;
	//3��hierarchy:���������ļ̳й�ϵ��hierarchyҲ��һ�����������Ⱥ�contours��ȣ�ÿ��Ԫ�غ�contours��Ԫ�ض�Ӧ��
	//	hierarchy[i][0], hierarchy[i][1], hierarchy[i][2], hierarchy[i][3], �ֱ��ʾ���ǵ�i������(contours[i])����һ����ǰһ���������ĵ�һ������(��һ��������)�Ͱ�����������(������)��
	
	//�����������ԣ�hierarchy[i][2]��ʾ��������hierachy[i][3]��ʾ������

	//4��mod: ��������ķ����������ַ�����
	//5��method : ��ʾһ�������ķ�����
	//6��offset : ��ѡ��ƫ�ƣ����Ǽ򵥵�ƽ�ƣ��ر���������ROI����֮�����á�

	for (int i = 0; i < contours.size(); i++)
	{
		bool isQr = IsQrPoint(contours[i], threshold_output_copy);
			//�����ҵ���������ɫ��λ��
			if (isQr)
			{
				qrPoint.push_back(contours[i]);
				//cout << contours[i][0].x << " " << contours[i][0].y << endl;
			}
	}

	if (qrPoint.size() != 3)
		return false;
	AdjustQrpoint(qrPoint);
	return true;
}

//�ҵ�������������ͼԪ����Ҫ��һ�������ǲ��Ƕ�ά�붥��
bool IsQrPoint(vector<Point>& contours,Mat& image)
{
	//����ڰױ���1:1:3:1:1
	bool result = IsQrColorRate(contours,image);
	return result;
}

//�ڰױ����жϺ���
bool IsQrColorRate(vector<Point>& contours,Mat &image)
{
	bool x = IsQrColorRateX(contours,image);
	if (!x)
		return false;
	bool y = IsQrColorRateY(contours,image);
	return y;
}
bool IsQrColorRateX(vector<Point>& contours, Mat& image)
{
	//�þ��������������ֹ���������
	vector<Rect> boundRect(1);
	boundRect[0] = boundingRect((Mat)contours);
	int nr = boundRect[0].y + boundRect[0].height / 2;//nr��ʾ�м����к�
	int nc = boundRect[0].width;

	//int x1 = 0, x2 = contours.size() / 2;
	//int nr = contours[x1].y + (contours[x2].y - contours[x1].y) / 2;//nr��ʾ�м����к�
	//int nc = contours[x2].x - contours[x1].x;//nc��ʾ���
	int d = nc / 7;

	/*if (GetGrayScale(image, nr, contours[0].x + d / 2) < 28 && GetGrayScale(image, nr, contours[0].x + 3 * d / 2) > 207 &&
		GetGrayScale(image, nr, contours[0].x + 5 * d / 2) < 28 && GetGrayScale(image, nr, contours[0].x + 7 * d / 2) < 28 &&
		GetGrayScale(image, nr, contours[0].x + 9 * d / 2) < 28 && GetGrayScale(image, nr, contours[0].x + 11 * d / 2) > 207 &&
		GetGrayScale(image, nr, contours[0].x + 13 * d / 2) < 28)
		return true;*/
	if (Real_Scale2(image, nr, boundRect[0].x,boundRect[0].x + d) == 1 &&
		Real_Scale2(image, nr, boundRect[0].x + d, boundRect[0].x + 2*d) == 0 &&
		Real_Scale2(image, nr, boundRect[0].x + 2 * d, boundRect[0].x + 3*d) == 1 &&
		Real_Scale2(image, nr, boundRect[0].x + 3 * d, boundRect[0].x + 4*d) == 1 &&
		Real_Scale2(image, nr, boundRect[0].x + 4 * d, boundRect[0].x + 5*d) == 1 &&
		Real_Scale2(image, nr, boundRect[0].x + 5 * d, boundRect[0].x + 6*d) == 0 &&
		Real_Scale2(image, nr, boundRect[0].x + 6 * d, boundRect[0].x + 7*d) == 1 )
		return true;

	return false;
}
//����ڰױ����ж�
bool IsQrColorRateY(vector<Point>& contours, Mat& image)
{
	//�þ��������������ֹ���������
	vector<Rect> boundRect(1);
	boundRect[0] = boundingRect((Mat)contours);
	int nr = boundRect[0].x + boundRect[0].width / 2;//nr��ʾ�м����к�
	int nc = boundRect[0].height;


	//int x2 = 0, x1 = contours.size() / 2;
	//int nr = contours[x2].x + (contours[x1].x - contours[x2].x) / 2;//nr��ʾ�м����к�
	//int nc = contours[x1].y - contours[x2].y;//nc��ʾ�߶�
	int d = nc / 7;

	/*if (GetGrayScale(image, contours[0].y + d / 2, nr) < 28 && GetGrayScale(image, contours[0].y + 3 * d / 2, nr) > 207 &&
		GetGrayScale(image, contours[0].y + 5 * d / 2, nr) < 28 && GetGrayScale(image, contours[0].y + 7 * d / 2, nr) < 28 &&
		GetGrayScale(image, contours[0].y + 9 * d / 2, nr) < 28 && GetGrayScale(image, contours[0].y + 11 * d / 2, nr) > 207 &&
		GetGrayScale(image, contours[0].y + 13 * d / 2, nr) < 28)
		return true;*/
	if (Real_Scale3(image, nr, boundRect[0].y, boundRect[0].y + d) == 1 &&
		Real_Scale3(image, nr, boundRect[0].y + d, boundRect[0].y + 2 * d) == 0 &&
		Real_Scale3(image, nr, boundRect[0].y + 2 * d, boundRect[0].y + 3 * d) == 1 &&
		Real_Scale3(image, nr, boundRect[0].y + 3 * d, boundRect[0].y + 4 * d) == 1 &&
		Real_Scale3(image, nr, boundRect[0].y + 4 * d, boundRect[0].y + 5 * d) == 1 &&
		Real_Scale3(image, nr, boundRect[0].y + 5 * d, boundRect[0].y + 6 * d) == 0 &&
		Real_Scale3(image, nr, boundRect[0].y + 6 * d, boundRect[0].y + 7 * d) == 1)
		return true;

	return false;
}
//����3�������˳��(����pt0������pt1������pt2)
void AdjustQrpoint(vector<vector<Point>>& qrPoint)
{
	int pt0, pt1, pt2;
	vector<vector<Point>> qrPoint_temp[3];
	vector<Rect> boundRect(3);
	for(int i=0;i<3;i++)
		boundRect[i] = boundingRect((Mat)qrPoint[i]);
	
	for (int i = 0; i < 3; i++)
	{
		if (boundRect[i].x >= boundRect[0].x && boundRect[i].x >= boundRect[1].x
			&& boundRect[i].x >= boundRect[2].x)
			pt1 = i;//x����Ϊ����
		else if (boundRect[i].y >= boundRect[0].y && boundRect[i].y >= boundRect[1].y
			&& boundRect[i].y >= boundRect[2].y)
			pt2 = i;//y����Ϊ����
		else
			pt0 = i;//ʣ�µ�Ϊ����
	}
	qrPoint_temp->push_back(qrPoint[pt0]);
	qrPoint_temp->push_back(qrPoint[pt1]);
	qrPoint_temp->push_back(qrPoint[pt2]);
	qrPoint.assign(qrPoint_temp->begin(),qrPoint_temp->end());//�ѽ���������ݿ�����qrPoint
}
//��ûҶ�ֵ
int GetGrayScale(Mat& image, double row, double col)
{
	uchar* data = image.ptr<uchar>(row);
	int Scale = data[(int)col];
	return Scale;
}
//����
void Decode(Mat& image, vector<vector<Point>>& qrPoint,int* Code)
{
	int codenum = 0;//��¼����λ��

	//��ɫͼת�Ҷ�ͼ
	Mat src_gray;
	cvtColor(image, src_gray, CV_BGR2GRAY);

	//��ֵ��
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();

	//�þ��������������ֹ���������
	vector<Rect> boundRect(3);
	for(int i=0;i<3;i++)
		boundRect[i] = boundingRect(qrPoint[i]);
	double white_wid = boundRect[0].width*2/21;
	double row0 = boundRect[0].y + boundRect[0].height + white_wid,
		col0 = boundRect[0].x + boundRect[0].width + white_wid,
		row1 = boundRect[2].y - white_wid,
		col1 = boundRect[1].x - white_wid;


	//double temp = qrPoint[0].size();
	//double white_wid = (qrPoint[0][temp / 2].x - qrPoint[0][0].x)*2/21;//��10/105��
	//double row0 = qrPoint[0][temp / 2].y+white_wid,
	//	col0 = qrPoint[0][temp / 2].x+white_wid,
	//	row1 = qrPoint[2][0].y-white_wid,
	//	col1 = qrPoint[1][0].x-white_wid;
	double dw = (col1 - col0)/64.0,
			dh = (row1 - row0)/64.0;
	double ddw = dw / 6,//ddw��ddh������ÿ��Сɫ������ٶ��ȡ�㣬���ʶ���׼ȷ��
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
	int num=0;//�ڵ����"1"
	for (double i = row - ddw; i <= row + ddw; i += ddw)
	{
		for (double j = col - ddh; j < col + ddh; j += ddh)
		{
			if (GetGrayScale(image, i, j)< low_gray_bound)
				num++;
		}
	}
	if (num >= 5)
		return 1;
	else
		return 0;
}

//�ж϶�ά���Ƿ�Ϊ��
bool Is_empty(Mat& image, vector<vector<Point>>& qrPoint)
{
	int* Code_temp = new int[5000];
	Decode(image, qrPoint, Code_temp);
	cout << endl;
	int white_num = 0;//��¼�׵����
	for (int i = 0; i < 4096; i+=1)
	{
		if (Code_temp[i] == 0)
			white_num++;
	}
	delete []Code_temp;
	if (white_num >3000)
		return true;
	else
		return false;
}

//һ�ν�������ͼ
void Decode_new(Mat& image, vector<vector<Point>>& qrPoint, int* Code,int&imnum)
{
	int* Code1 = new int[5000];
	int* Code2 = new int[5000];
	int* Code3 = new int[5000];

	//�����������ͼ��bit��Ϣ������Code1��Code2��Code3��
	char filename1[50];
	sprintf_s(filename1, 50, "x%d.png", imnum++);
	fstream inFile1(filename1);
	if (!inFile1.eof())
		Decode(image, qrPoint, Code1);
	cout << endl << "imnum=" << imnum - 1 << endl;

	char filename2[50];
	sprintf_s(filename2, 50, "x%d.png", imnum++);
	fstream inFile2(filename2);
	if (!inFile2.eof())
		Decode(image, qrPoint, Code2);
	cout << endl << "imnum=" << imnum - 1 << endl;

	char filename3[50];
	sprintf_s(filename3, 50, "x%d.png", imnum++);
	fstream inFile3(filename3);
	if (!inFile3.eof())
		Decode(image, qrPoint, Code3);
	cout << endl << "imnum=" << imnum-1 << endl;

	//��λ�Ƚ�Code1��Code2��Code3������ʵ��Ϣ�����Code��
	for (int i = 0; i < 4096; i++)
	{
		Code[i] = Real_bit(Code1[i],Code2[i],Code3[i]);
	}
	delete []Code1;
	delete []Code2;
	delete []Code3;

	//��ԭΪ�ַ����������output.txt
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
	int temp=0;//��¼0�ĸ���
	if (x1 == 0)temp++;
	if (x2 == 0)temp++;
	if (x3 == 0)temp++;

	if (temp >= 2)
		return 0;
	else
		return 1;
}

//���ڶԱȶ�
void Adjust_Contrast(Mat& image)
{
	int rows = image.rows;
	int cols = image.cols;
	int channels = image.channels();
	float alpha = 1.5;
	float beta = 10;
	Mat dst(image.size(), image.type());

	for (int row = 0; row < rows; row++) {
		const uchar* currentSrcRow = image.ptr<uchar>(row);
		uchar* currentDstRow = dst.ptr<uchar>(row);
		for (int col = 0; col < cols * channels; col++) {
			currentDstRow[col] = saturate_cast<uchar>(alpha * currentSrcRow[col] + beta);
		}
	}
	/*dst.copyTo(image);*/
	
	/*namedWindow("output image", CV_WINDOW_NORMAL);
	imshow("output image", dst);*/
}

int Real_Scale2(Mat& image, int line, int row_begin, int row_end)
{
	int width = row_end - row_begin;
	int num = 0;
	for (int i = row_begin; i <= row_end; i++)
	{
		if (GetGrayScale(image, line, i) <= low_gray_bound)
			num++;
	}
	if (num > width / 2)
		return 1;
	else
		return 0;
}
int Real_Scale3(Mat& image, int row, int line_begin, int line_end)
{
	int height = line_end - line_begin;
	int num = 0;
	for (int i = line_begin; i <= line_end; i++)
	{
		if (GetGrayScale(image, i, row) <= low_gray_bound)
			num++;
	}
	if (num > height / 2)
		return 1;
	else
		return 0;
}

int main()
{
	char filename[50];//������ʱ�洢�ļ���
	int codenum = 0;
	int imnum = 1;//��¼�����ͼƬ����
	int Begin_flag = 0;//0��ʾǰͬ���룬1��ʾ������

//WinExec("./ffmpeg.exe -i output.mp4 -r 30 x%d.png", SW_SHOW);

	//while (1)
	//{
	//	sprintf_s(filename, 50, "x%d.png", imnum++);
	//	//sprintf_s(filename, 50, "test6.png");
	//	fstream inFile(filename);
	//	if (inFile.good())
	//	{
	//		Mat image = imread(filename);
	//		int rows = image.rows;
	//		int cols = image.cols;
	//		int channels = image.channels();
	//		float alpha = 1.3;
	//		float beta = 10;
	//		Mat dst(image.size(), image.type());

	//		for (int row = 0; row < rows; row++) {
	//			const uchar* currentSrcRow = image.ptr<uchar>(row);
	//			uchar* currentDstRow = dst.ptr<uchar>(row);
	//			for (int col = 0; col < cols * channels; col++) {
	//				currentDstRow[col] = saturate_cast<uchar>(alpha * currentSrcRow[col] + beta);
	//			}
	//		}
	//		dst.copyTo(image);

	//		imwrite(filename, image);
	//	}
	//	else break;
	//}
	//
	while (1)
	{
		int* Code = new int[5000];
		sprintf_s(filename, 50, "x%d.png", imnum++);
		//sprintf_s(filename, 50, "x23.png");
		fstream inFile(filename);
		if (inFile.good())
		{
			Mat img = imread(filename);
			//Adjust_Contrast(img);
			/*namedWindow("output image", CV_WINDOW_AUTOSIZE);
			imshow("output image", img);*/

			vector<vector<Point>> qrPoint;
		//	cout << "x" << imnum - 1 << ".png=" << FindQrPoint(img, qrPoint) << endl;
			cout << endl << "imnum=" << imnum-1 << endl;
			if (FindQrPoint(img, qrPoint))
			{
				cout << "x" << imnum - 1 << ".png_empty=" << Is_empty(img, qrPoint) << endl;
				if (Is_empty(img, qrPoint))//�ж�λ����Ϊ��
				{
					if (Begin_flag == 0)//�ǿ�ʼ��
						continue;//����һ��	
					else//�ǽ�����
						return 0;//�˳�
				}
				else//�ж�λ���Ҳ���
				{
					Begin_flag = 1;//��������
					imnum--;
					Decode_new(img, qrPoint, Code, imnum);
				}
			}
				
		}
		else
			break;

		delete []Code;
	}

	waitKey(0);
	return 0;
}
