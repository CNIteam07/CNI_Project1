#include"decoder.h"

/*查找轮廓, 筛选出三个二维码顶点*/
bool SearchQrcodeFunctions::FindQrPoint(Mat& srcImg, vector<vector<Point>>& qrPoint)//查找srcImg的顶点存入qrPoint
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
	for (int i = 0; i < contours.size(); i++)
	{
		bool isQr = IsQrPoint(contours[i], threshold_output_copy);
		//保存找到的三个黑色定位角
		if (isQr)
		{
			qrPoint.push_back(contours[i]);
		}
	}

	if (qrPoint.size() != 3)
		return false;
	AdjustQrpoint(qrPoint);
	return true;
}

/*找到了两个轮廓的图元，需要进一步分析是不是二维码顶点*/
bool SearchQrcodeFunctions::IsQrPoint(vector<Point>& contours, Mat& image)
{
	//横向黑白比例1:1:3:1:1
	bool result = IsQrColorRate(contours, image);
	return result;
}

/*黑白比例判断函数*/
bool SearchQrcodeFunctions::IsQrColorRate(vector<Point>& contours, Mat& image)
{
	bool x = IsQrColorRateX(contours, image);
	if (!x)
		return false;
	bool y = IsQrColorRateY(contours, image);
	return y;
}

bool SearchQrcodeFunctions::IsQrColorRateX(vector<Point>& contours, Mat& image)
{
	//用矩形拟合轮廓，防止角坐标错误
	vector<Rect> boundRect(1);
	boundRect[0] = boundingRect((Mat)contours);
	int nr = boundRect[0].y + boundRect[0].height / 2;//nr表示中间行行号
	int nc = boundRect[0].width;
	int d = nc / 7;
	if (Real_ScaleX(image, nr, boundRect[0].x, boundRect[0].x + d) == 1 &&
		Real_ScaleX(image, nr, boundRect[0].x + d, boundRect[0].x + 2 * d) == 0 &&
		Real_ScaleX(image, nr, boundRect[0].x + 2 * d, boundRect[0].x + 3 * d) == 1 &&
		Real_ScaleX(image, nr, boundRect[0].x + 3 * d, boundRect[0].x + 4 * d) == 1 &&
		Real_ScaleX(image, nr, boundRect[0].x + 4 * d, boundRect[0].x + 5 * d) == 1 &&
		Real_ScaleX(image, nr, boundRect[0].x + 5 * d, boundRect[0].x + 6 * d) == 0 &&
		Real_ScaleX(image, nr, boundRect[0].x + 6 * d, boundRect[0].x + 7 * d) == 1)
		return true;

	return false;
}

bool SearchQrcodeFunctions::IsQrColorRateY(vector<Point>& contours, Mat& image)
{
	//用矩形拟合轮廓，防止角坐标错误
	vector<Rect> boundRect(1);
	boundRect[0] = boundingRect((Mat)contours);
	int nr = boundRect[0].x + boundRect[0].width / 2;//nr表示中间列列号
	int nc = boundRect[0].height;
	int d = nc / 7;

	if (Real_ScaleY(image, nr, boundRect[0].y, boundRect[0].y + d) == 1 &&
		Real_ScaleY(image, nr, boundRect[0].y + d, boundRect[0].y + 2 * d) == 0 &&
		Real_ScaleY(image, nr, boundRect[0].y + 2 * d, boundRect[0].y + 3 * d) == 1 &&
		Real_ScaleY(image, nr, boundRect[0].y + 3 * d, boundRect[0].y + 4 * d) == 1 &&
		Real_ScaleY(image, nr, boundRect[0].y + 4 * d, boundRect[0].y + 5 * d) == 1 &&
		Real_ScaleY(image, nr, boundRect[0].y + 5 * d, boundRect[0].y + 6 * d) == 0 &&
		Real_ScaleY(image, nr, boundRect[0].y + 6 * d, boundRect[0].y + 7 * d) == 1)
		return true;

	return false;
}

/*修正3个顶点的顺序(左上pt0，右上pt1，左下pt2)*/
void SearchQrcodeFunctions::AdjustQrpoint(vector<vector<Point>>& qrPoint)
{
	int pt0, pt1, pt2;
	vector<vector<Point>> qrPoint_temp[3];
	vector<Rect> boundRect(3);
	for (int i = 0; i < 3; i++)
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
	qrPoint.assign(qrPoint_temp->begin(), qrPoint_temp->end());//把交换完的数据拷贝回qrPoint
}

/*获得灰度值*/
int SearchQrcodeFunctions::GetGrayScale(Mat& image, double row, double col)
{
	uchar* data = image.ptr<uchar>(row);
	int Scale = data[(int)col];
	return Scale;
}

int SearchQrcodeFunctions::Real_ScaleX(Mat& image, int line, int row_begin, int row_end)
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

int SearchQrcodeFunctions::Real_ScaleY(Mat& image, int row, int line_begin, int line_end)
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

/*判断二维码是否为空*/
bool SearchQrcodeFunctions::Is_empty(Mat& image, vector<vector<Point>>& qrPoint)
{
	DecodingFunctions Decodefunction;
	int* Code_temp = new int[5000];
	Decodefunction.Decode(image, qrPoint, Code_temp);
	int num = 0;//记录黑点个数
	for (int i = 0; i < 2400; i++)
	{
		if (Code_temp[i] == 1)
			num++;
	}
	delete[]Code_temp;

	if (num < 1)
		return true;
	else
		return false;
}

/*解码*/
void DecodingFunctions:: Decode(Mat& image, vector<vector<Point>>& qrPoint, int* Code)
{
	SearchQrcodeFunctions SearchQrcodeFunction;
	int codenum = 0;//记录解码位数
	int* Code_temp = new int[5000];//临时存储带有纠错码的信息

	//彩色图转灰度图
	Mat src_gray;
	cvtColor(image, src_gray, CV_BGR2GRAY);

	//二值化
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat threshold_output_copy = threshold_output.clone();

	//用矩形拟合轮廓，防止角坐标错误
	vector<Rect> boundRect(3);
	for (int i = 0; i < 3; i++)
		boundRect[i] = boundingRect(qrPoint[i]);
	double white_wid = boundRect[0].width * 2.0 / 21;	//10/105;
	double row0 = static_cast<double>(boundRect[0].y) + static_cast<double>(boundRect[0].height) + white_wid,
		col0 = static_cast<double>(boundRect[0].x) + static_cast<double>(boundRect[0].width) + white_wid,						//3.19强制类型转换以避免算数溢出
		row1 = boundRect[2].y - white_wid,
		col1 = boundRect[1].x - white_wid;

	double dw = (col1 - col0) / 60.0,
		dh = (row1 - row0) / 60.0;
	double ddw = dw / 9,//ddw，ddh用于在每个小色块儿里再多次取点，提高识别的准确度
		ddh = dh / 9;

	for (int i = 0; i < 60; i++)
	{
		for (int j = 0; j < 60; j++)
		{
			if (SearchQrcodeFunction.GetGrayScale(threshold_output_copy, row0 + dh * (2.0 * i + 1) / 2, col0 + dw * (2.0 * j + 1) / 2) < low_gray_bound)
				Code_temp[codenum++] = 1;
			else
				Code_temp[codenum++] = 0;
		}
	}
	for (int i = 0; i < 3600; i += 12)
	{
		int code_check[12];
		for (int j = 0; j < 12; j++)//截取12位放入code_check[12]中
		{
			code_check[j] = Code_temp[i + j];
		}
		int bit = check(code_check);
		if (bit != -1)
			Code_temp[i + bit] = (Code_temp[i + bit] + 1) % 2;//纠正Code_temp
	}

	//将纠错后的信息流存入Code
	for (int i = 0; i < 3600; i += 12)
	{
		for (int j = 0; j < 8; j++)
		{
			Code[i * 2 / 3 + j] = Code_temp[i + j];
		}
	}
	delete[]Code_temp;
}

/*校验*/
int DecodingFunctions::check(int code_check[12])
{
	int Gx[5] = { 1,0,0,1,1 };//Gx=10011
	int copy_code[12];
	for (int i = 0; i <= 11; i++)copy_code[i] = code_check[i];
	for (int i = 0; i <= 7; i++)
	{
		if (copy_code[i] == 1)
		{
			for (int k = 0; k <= 4; k++)
			{
				if (copy_code[i + k] != Gx[k])copy_code[i + k] = 1;
				else copy_code[i + k] = 0;
			}
		}
	}
	int result_crc = 0;
	for (int i = 8; i <= 11; i++)
	{
		result_crc = result_crc * 2 + copy_code[i];
	}
	if (result_crc == 0)return -1;

	switch (result_crc)
	{
	case 14:return 0;
	case 7:return 1;
	case 10:return 2;
	case 5:return 3;
	case 11:return 4;
	case 12:return 5;
	case 6:return 6;
	case 3:return 7;
	case 8:return 8;
	case 4:return 9;
	case 2:return 10;
	case 1:return 11;
	default:
		return -1;	//由于4位可纠错16位，信息8+4=12位，正确1位，还多出来3位，暂时不作处理
		break;
	}
}

/*一次解码三张图*/
void DecodingFunctions::Decode_new(int* Code, int& imnum,string path,string decodefilename,string voutfilename)
{
	SearchQrcodeFunctions SearchQrcodeFunction;
	/*全部初始化为0*/
	int* Code1 = new int[5000]();
	int* Code2 = new int[5000]();
	int* Code3 = new int[5000]();
	Mat img;
	char filename[50];
	//获得连续三张图的bit信息，存在Code1，Code2，Code3中
	sprintf_s(filename,50, "/x%d.png", imnum);
	string image_path = path + filename;
	fstream inFile1(image_path);
	if (!inFile1.eof())
	{
		img = imread(image_path);
		vector<vector<Point>> qrPoint1;
		SearchQrcodeFunction.FindQrPoint(img, qrPoint1);
		Decode(img, qrPoint1, Code1);
	}
	imnum++;
	sprintf_s(filename,50, "/x%d.png", imnum);
	image_path = path+filename;
	fstream inFile2(image_path);
	if (!inFile2.eof())
	{
		img = imread(image_path);
		vector<vector<Point>> qrPoint2;
		SearchQrcodeFunction.FindQrPoint(img, qrPoint2);
		Decode(img, qrPoint2, Code2);
	}
	imnum++;
	sprintf_s(filename, 50,"/x%d.png", imnum);
	image_path = path + filename;
	fstream inFile3(image_path);
	if (!inFile3.eof())
	{
		img = imread(image_path);
		vector<vector<Point>> qrPoint3;
		SearchQrcodeFunction.FindQrPoint(img, qrPoint3);
		Decode(img, qrPoint3, Code3);
	}
	imnum++;
	//按位比较Code1，Code2，Code3，将真实信息存放在Code中
	for (int i = 0; i < 2400; i++)
	{
		Code[i] = Real_bit(Code1[i], Code2[i], Code3[i]);
	}
	delete[]Code1;
	delete[]Code2;
	delete[]Code3;

	//还原为字符串，输出decode.bin,vout.bin
	vector<unsigned char> data;
	vector<unsigned char> datav;
	int temp = 0;
	unsigned char temp_char;
	for (int i = 0; i < 2400; i += 8)
	{
		for (int j = 0; j < 8; j++)
			temp = temp * 2 + Code[i + j];
		temp_char = (unsigned char)temp;
		data.push_back(temp_char);
		datav.push_back(0xFF);
		temp = 0;
	}
	//将vector转unsigned char数组并fwrite
	unsigned char DATA[300];
	unsigned char DATAV[300];
	memcpy(DATA, &data[0], data.size() * sizeof(unsigned char));
	memcpy(DATAV, &datav[0], datav.size() * sizeof(unsigned char));
	char filename1[50];
	char filename2[50];
	sprintf_s(filename1, "%s", decodefilename.c_str());
	sprintf_s(filename2, "%s", voutfilename.c_str());

	FILE* fp1 = fopen(filename1, "ab+");
	FILE* fp2 = fopen(filename2, "ab+");

	fwrite(DATA, sizeof(unsigned char), 300, fp1);
	fwrite(DATAV, sizeof(unsigned char), 300, fp2);
	
	return;
}

/*（按位）判断一个bit真实信息*/
int DecodingFunctions::Real_bit(int x1, int x2, int x3)
{
	int temp = 0;//记录0的个数
	if (x1 == 0)temp++;
	if (x2 == 0)temp++;
	if (x3 == 0)temp++;

	if (temp >= 2)
		return 0;
	else
		return 1;
}

/*调节对比度*/
void ImageOperateFunctions:: Adjust_Contrast(Mat& image)
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
}
