#ifndef FUN_H
#define FUN_H
#include<string.h>
#include <QDialog>
#include<QString>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
namespace Ui {
class fun;
}

class fun : public QDialog
{
    Q_OBJECT

public:
    explicit fun(QWidget *parent = 0);
    ~fun();
    bool makingvideo(std::string,std::string);
    void change_to_binary(std::string,char[64]);
    void draw_ROI(cv::Mat& erweima);
    QString selectFilename();
    QString selectPathname();
private slots:
    void on_pbt_cancel_clicked();

    void on_pbt_comfirm_clicked();

    void on_btn_sourse_clicked();

    void on_btn_detention_clicked();

private:
    Ui::fun *ui;
    QString path;
};

#endif // FUN_H
