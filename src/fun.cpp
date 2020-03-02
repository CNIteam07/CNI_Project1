#include "fun.h"
#include "ui_fun.h"
#include<QString>
#include<QDebug>
#include<QMessageBox>
#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<vector>
#include<fstream>
#include<Windows.h>
#include<stdlib.h>
#include<stdio.h>
#include<Qmenu>
#include<QFileDialog>
#include<QProgressDialog>
#include<opencv2/imgproc/imgproc.hpp>
#include<cstdlib>
#include<opencv2/opencv.hpp>
#define string_length 10

#define MAXSIZE 50000

using namespace cv;
using namespace std;
fun::fun(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fun)
{
    ui->setupUi(this);
}

fun::~fun()
{
    delete ui;
}

void fun::on_pbt_cancel_clicked()
{
    this->close();
}
bool fun::makingvideo(string yuan_file,string mudi_file)
{
    using namespace cv;
    void change_to_binary(string,char[64]);
    void draw_ROI(Mat&);
    ifstream read_file(yuan_file);//打开.txt文件
    if(!read_file)
    {
        QMessageBox::critical(this,"错误","打开文件失败！请确认。"); //打开失败报错反馈
        return false;
    }
    Mat my_qrcode(570, 570, CV_8UC3, Scalar(255, 255, 255));
        int locate=125;
        char binary[64];
        for (int i = 0; i < 64; i++)
            binary[i] = '0';
        int line = 0;
        int index = 1;
        Point left_upper;
        Point upper_right;
        //起始帧
        draw_ROI(my_qrcode);
        imwrite(mudi_file+"/0.bmp",my_qrcode);
        while (!read_file.eof())
        {//roopstart
            string tmp;
            getline(read_file, tmp);
            //绘制三个定位点
            draw_ROI(my_qrcode);
            //填充bit信息
            for (unsigned int i = 0; i <= tmp.length() - 1; i += 8)
            {//stringstate
                string temp_read;//读出的字符
                temp_read.append(tmp, i, 8);
                change_to_binary(temp_read, binary);
                for (int i = 0; i < 64; i++)
                {
                    left_upper = Point(locate + i * 5, locate + line * 5);
                    upper_right = Point(locate + i * 5 + 5, locate + line * 5 + 5);
                    /*rectangle(src, start, center, Scalar(0, 0, 0), FILLED);*/
                    if (binary[i] == '1')rectangle(my_qrcode, left_upper, upper_right, Scalar(0, 0, 0), FILLED);//黑表示1
                    if (binary[i] == '0')rectangle(my_qrcode, left_upper, upper_right, Scalar(255, 255, 255), FILLED);//白表示0
                }
                line++;
                if (line % 64 == 0)
                {
                    char file_name[10];
                    sprintf(file_name, "%d", index);
                    imwrite(mudi_file + "/" + file_name+ ".bmp", my_qrcode);
                    my_qrcode = Mat(570, 570, CV_8UC3, Scalar(255, 255, 255));
                    draw_ROI(my_qrcode);
                    index++;
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
                sprintf(file_name, "%d", index);
                imwrite(mudi_file + "/" + file_name + ".bmp", my_qrcode);
                index++;
            }
        }//roopend
        const char* stdpath=mudi_file.c_str();
        char* ffmpegpath="./ffmpeg.exe"; //ios c++可以考虑c_str()方法
        char* jpg="/%d.bmp";
        char* mp4="/output.mp4";
        char cmdcomand[100];
        sprintf(cmdcomand,"%s -f image2 -r 10 -i %s%s -vcodec mpeg4 %s%s",ffmpegpath,stdpath,jpg,stdpath,mp4);
        WinExec(cmdcomand,SW_SHOW);
        /* const char* result="./ffmpeg.exe -f image2 -r 10 -i "+stdpathname+"/%d.jpg -vcodec mpeg4 "+stdpathname+"/output.mp4";*/
        /* system(result);//调命令行完成ffmpeg从图像convert视频 */
        return true;
    }
void fun::on_pbt_comfirm_clicked()
{
    QString filename=this->ui->led_filename->text();
    QString filepath=this->ui->led_path->text();
    QString file_name_type=filename.right(3);
    QString path_name_type=filepath.right(3);
    QDir dir(filepath);
    QMessageBox msgBox;
    string stdfilename=filename.toStdString();
    string stdfilepath=filepath.toStdString();
    int ret;
    if(filename=="")//文件路径为空
    {
        QMessageBox::warning(this,"警告","您未设置源文件路径，请重新设置。");
        return;
    }
    if(filepath=="")//文件夹路径为空
    {
        QMessageBox::warning(this,"警告","您未设置目标文件夹路径，请重新设置。");
        return;
    }
    if(filename.contains(QRegExp("[\\x4e00-\\x9fa5]+")))//中文识别
    {
        QMessageBox::warning(this,"警告","当前程序不支持中文目录。请重新设置。");
        this->ui->led_filename->setText("");
        return;
    }
    if(filepath.contains(QRegExp("[\\x4e00-\\x9fa5]+")))//中文识别
    {
        QMessageBox::warning(this,"警告","当前程序不支持中文目录。请重新设置。");
        this->ui->led_path->setText("");
        return;
    }
    if(!dir.exists())//文件夹路径不存在是否创建
    {
        ret=QMessageBox::question(this,"警告","文件路径不存在，是否要创建该文件夹？","确认","取消");
        if(ret==0)
        {
            dir.mkpath(filepath);
        }
        else return;
    }
    /*msgBox.setText("信息已经确认.");
        msgBox.setInformativeText("是否依据默认路径进行下一步？");
        msgBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret=msgBox.exec();*/
    if(file_name_type!="txt"&&file_name_type!="dat")//该模块用于检查文件所属类型
    {
        QMessageBox::warning(this,"Attention!","不支持的文件类型.\n请尝试输入文件扩展名.");
        return;
    }
    ret=QMessageBox::question(this,"信息已确认.","是否依据该路径进行下一步？","确认","取消");
    if(ret==0)
    {
        bool judge=makingvideo(stdfilename,stdfilepath);
        if(judge)
        {
            for(int i=0;i<101;++i)
            {
                this->ui->progressBar->setValue(i);
            }
            QMessageBox::information(this,"成功.","视频被保存在指定目录下.");
            this->ui->progressBar->setValue(0);
        }
    }
    else return;
}

void change_to_binary(string in,char binary_arrry[64]){

    int t = 0;
        for (unsigned int i = 0; i<in.length(); i++)
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

void draw_ROI(Mat& erweima)
{
    //绘制三个定位点
        Point center = Point(10, 10);
        Point start = Point(115, 115);
        rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
        center = Point(25, 25);
        start = Point(100, 100);
        rectangle(erweima, start, center, Scalar(255, 255, 255), FILLED);
        center = Point(40, 40);
        start = Point(85, 85);
        rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
        center = Point(455, 10);
        start = Point(560, 115);
        rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
        center = Point(470, 25);
        start = Point(545, 100);
        rectangle(erweima, start, center, Scalar(255, 255, 255), FILLED);
        center = Point(485, 40);
        start = Point(530, 85);
        rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
        center = Point(10, 455);
        start = Point(115, 560);
        rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
        center = Point(25, 470);
        start = Point(100, 545);
        rectangle(erweima, start, center, Scalar(255, 255, 255), FILLED);
        center = Point(40, 485);
        start = Point(85, 530);
        rectangle(erweima, start, center, Scalar(0, 0, 0), FILLED);
}

void fun::on_btn_sourse_clicked()
{
    path=selectFilename();
    if(path!=nullptr)
    {
        this->ui->led_filename->setText(path);
    }
}

void fun::on_btn_detention_clicked()
{
    path=selectPathname();
    if(path!=nullptr)
    {
        this->ui->led_path->setText(path);
    }
}
QString fun::selectFilename()
{
    QFileDialog *fileDialog = new QFileDialog(this);//创建一个QFileDialog对象，构造函数中的参数可以有所添加。
    fileDialog->setWindowTitle(tr("打开文件"));//设置文件保存对话框的标题
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器

    fileDialog->setNameFilter(tr("All(*.dat *.txt)"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    fileDialog->setFileMode(QFileDialog::ExistingFiles);//这个地方可以设置选择文件夹，我这里写的是选择文件，可以看源码
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //还有另一种形式QFileDialog::List，这个只是把文件的文件名以列表的形式显示出来
    // fileDialog->setGeometry(10,30,300,200);//设置文件对话框的显示位置

    if(fileDialog->exec() == QDialog::Accepted)
    {
        path = fileDialog->selectedFiles()[0];//得到用户选择的第一个文件名
        fileDialog->close();
        return path;
    }
    else
        return 0;
}
QString fun::selectPathname()
{
    QFileDialog *fileDialog = new QFileDialog(this);//创建一个QFileDialog对象，构造函数中的参数可以有所添加。
    fileDialog->setWindowTitle(tr("打开文件夹"));//设置文件保存对话框的标题
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    path=fileDialog->getExistingDirectory();//设置选择文件夹
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //还有另一种形式QFileDialog::List，这个只是把文件的文件名以列表的形式显示出来
    // fileDialog->setGeometry(10,30,300,200);//设置文件对话框的显示位置
    return path;
}
