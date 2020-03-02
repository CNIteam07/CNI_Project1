#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<cstdlib>
#include<fstream>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui_c.h>
#define string_length 10

using namespace std;
using namespace cv;

void draw_ROI(Mat& erweima);

void change_to_binary(string in,char binary_arrry[64])//将字母转换为二进制
{
    int t = 0;
    for (int i = 0; i<in.length(); i++)
    {
        int temp_ascii = (int)in[i];
        for (int index = 0; temp_ascii != 0; index++)
        {
            binary_arrry[index+t*8] += temp_ascii % 2;
            temp_ascii /= 2;
        }
        t++;
    }
	
	return;
}
void creat_photos(string yuan_file, string mudi_file)//打开文件，生成图片
{//funstart
	ifstream read_file(yuan_file);//打开.txt文件
	Mat erweima(570, 570, CV_8UC3, Scalar(255, 255, 255));
    int locate=125;//��ȡ����������ֵ
    char binary[64];
    for (int i = 0; i < 64; i++)
        binary[i] = '0';
    int line = 0;
    int zhang_index = 0;//�жϼ���ͼƬ
    int how_many_char = 0;//�жϼ�����
    Point zuoshang;//���ϵĵ�
    Point youxia;//���µĵ�
    while (!read_file.eof())
    {//roopstart
        string a;
        getline(read_file, a);

        //绘制三个定位点
        draw_ROI(erweima);

        //填充bit信息
        for (int i = 0; i <= a.length() - 1; i += 8)
        {//stringstate
            string temp_read;//读出的字符
            temp_read.append(a, i, 8);
            change_to_binary(temp_read, binary);

            for (int i = 0; i < 64; i++)
            {
                zuoshang = Point(locate + i * 5, locate + line * 5);
                youxia = Point(locate + i * 5 + 5, locate + line * 5 + 5);
                /*rectangle(src, start, center, Scalar(0, 0, 0), FILLED);*/
                if (binary[i] == '1')rectangle(erweima, zuoshang, youxia, Scalar(0, 0, 0), FILLED);//黑表示1
                if (binary[i] == '0')rectangle(erweima, zuoshang, youxia, Scalar(255, 255, 255), FILLED);//白表示0
            }
            line++;
            if (line % 64 == 0) 
            {
                char file_name[10];
                sprintf_s(file_name, "%d", zhang_index);
                imwrite(mudi_file + "//" + file_name+ ".bmp", erweima);
                erweima = Mat(570, 570, CV_8UC3, Scalar(255, 255, 255));
                draw_ROI(erweima);
                zhang_index++;
                line = 0;
            }
            for (int i = 0; i < 64; i++)
            {
                binary[i] = '0';
            }
        }//stringend
        if (line % 64 != 0)
        {
            char file_name[10];
            sprintf_s(file_name, "%d", zhang_index);
            imwrite(mudi_file + "//" + file_name + ".bmp", erweima);
            zhang_index++;
        }
    }//roopend
          
}//funend

void draw_ROI(Mat& erweima)
{
    //绘制三个定位点
    Point center = Point(10, 10);//��һ����λ�㶨�忪ʼ

    Point start = Point(115, 115);

    rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);

    center = Point(25, 25);

    start = Point(100, 100);

    rectangle(erweima, start, center, Scalar(255, 255, 255), FILLED);
    center = Point(40, 40);

    start = Point(85, 85);

    rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);//��һ����λ������


    center = Point(455, 10);//�ڶ�����λ�㿪ʼ

    start = Point(560, 115);

    rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
    center = Point(470, 25);

    start = Point(545, 100);

    rectangle(erweima, start, center, Scalar(255, 255, 255), FILLED);
    center = Point(485, 40);

    start = Point(530, 85);

    rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);//�ڶ�����λ������
    center = Point(10, 455);//��������λ�㿪ʼ

    start = Point(115, 560);

    rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
    center = Point(25, 470);

    start = Point(100, 545);

    rectangle(erweima, start, center, Scalar(255, 255, 255), FILLED);
    center = Point(40, 485);

    start = Point(85, 530);

    rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);//��������λ������
}

int main()
{
	creat_photos("C:\\Users\\Bcopton\\source\\repos\\Project1\\shuru.txt", "C:\\Users\\Bcopton\\source\\repos\\Project1");
	waitKey(0);
	return 0;
}