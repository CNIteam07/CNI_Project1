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

#define low_gray_bound 130//黑边界
#define high_gray_bound 130//白边界

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

	for (int i = 0; i < contours.size(); i++)
	{
		bool isQr = IsQrPoint(contours[i], threshold_output_copy);
			//保存找到的三个黑色定位角
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
bool IsQrColorRateX(vector<Point>& contours, Mat& image)
{
	//用矩形拟合轮廓，防止角坐标错误
	vector<Rect> boundRect(1);
	boundRect[0] = boundingRect((Mat)contours);
	int nr = boundRect[0].y + boundRect[0].height / 2;//nr表示中间行行号
	int nc = boundRect[0].width;

	//int x1 = 0, x2 = contours.size() / 2;
	//int nr = contours[x1].y + (contours[x2].y - contours[x1].y) / 2;//nr表示中间行行号
	//int nc = contours[x2].x - contours[x1].x;//nc表示宽度
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
//纵向黑白比例判断
bool IsQrColorRateY(vector<Point>& contours, Mat& image)
{
	//用矩形拟合轮廓，防止角坐标错误
	vector<Rect> boundRect(1);
	boundRect[0] = boundingRect((Mat)contours);
	int nr = boundRect[0].x + boundRect[0].width / 2;//nr表示中间列列号
	int nc = boundRect[0].height;


	//int x2 = 0, x1 = contours.size() / 2;
	//int nr = contours[x2].x + (contours[x1].x - contours[x2].x) / 2;//nr表示中间列列号
	//int nc = contours[x1].y - contours[x2].y;//nc表示高度
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
//修正3个顶点的顺序(左上pt0，右上pt1，左下pt2)
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
			pt1 = i;//x最大的为右上
		else if (boundRect[i].y >= boundRect[0].y && boundRect[i].y >= boundRect[1].y
			&& boundRect[i].y >= boundRect[2].y)
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
int GetGrayScale(Mat& image, double row, double col)
{
	uchar* data = image.ptr<uchar>(row);
	int Scale = data[(int)col];
	return Scale;
}
//解码
void Decode(Mat& image, vector<vector<Point>>& qrPoint,int* Code)
{
	int codenum = 0;//记录解码位数

	//彩色图转灰度图
	Mat src_gray;
	cvtColor(image, src_gray, CV_BGR2GRAY);

	//二值化
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();

	//用矩形拟合轮廓，防止角坐标错误
	vector<Rect> boundRect(3);
	for(int i=0;i<3;i++)
		boundRect[i] = boundingRect(qrPoint[i]);
	double white_wid = boundRect[0].width*2/21;
	double row0 = boundRect[0].y + boundRect[0].height + white_wid,
		col0 = boundRect[0].x + boundRect[0].width + white_wid,
		row1 = boundRect[2].y - white_wid,
		col1 = boundRect[1].x - white_wid;


	//double temp = qrPoint[0].size();
	//double white_wid = (qrPoint[0][temp / 2].x - qrPoint[0][0].x)*2/21;//“10/105”
	//double row0 = qrPoint[0][temp / 2].y+white_wid,
	//	col0 = qrPoint[0][temp / 2].x+white_wid,
	//	row1 = qrPoint[2][0].y-white_wid,
	//	col1 = qrPoint[1][0].x-white_wid;
	double dw = (col1 - col0)/64.0,
			dh = (row1 - row0)/64.0;
	double ddw = dw / 6,//ddw，ddh用于在每个小色块儿里再多次取点，提高识别的准确度
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
	int num=0;//黑点个数"1"
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

//判断二维码是否为空
bool Is_empty(Mat& image, vector<vector<Point>>& qrPoint)
{
	int* Code_temp = new int[5000];
	Decode(image, qrPoint, Code_temp);
	cout << endl;
	int white_num = 0;//记录白点个数
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

//一次解码三张图
void Decode_new(Mat& image, vector<vector<Point>>& qrPoint, int* Code,int&imnum)
{
	int* Code1 = new int[5000];
	int* Code2 = new int[5000];
	int* Code3 = new int[5000];

	//获得连续三张图的bit信息，存在Code1，Code2，Code3中
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

	//按位比较Code1，Code2，Code3，将真实信息存放在Code中
	for (int i = 0; i < 4096; i++)
	{
		Code[i] = Real_bit(Code1[i],Code2[i],Code3[i]);
	}
	delete []Code1;
	delete []Code2;
	delete []Code3;

	//还原为字符串，输出到output.txt
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
//（按位）判断一个bit真实信息
int Real_bit(int x1, int x2, int x3)
{
	int temp=0;//记录0的个数
	if (x1 == 0)temp++;
	if (x2 == 0)temp++;
	if (x3 == 0)temp++;

	if (temp >= 2)
		return 0;
	else
		return 1;
}

//调节对比度
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
	char filename[50];//用于临时存储文件名
	int codenum = 0;
	int imnum = 1;//记录解码的图片张数
	int Begin_flag = 0;//0表示前同步码，1表示结束码

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
				if (Is_empty(img, qrPoint))//有定位码且为空
				{
					if (Begin_flag == 0)//是开始码
						continue;//读下一张	
					else//是结束码
						return 0;//退出
				}
				else//有定位码且不空
				{
					Begin_flag = 1;//结束码标记
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
