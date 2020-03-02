#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<fun.h>
#include<dialog.h>
#include<about.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    fun m;
    m.show();
    m.exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    Dialog m;
    m.show();
    m.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    About m;
    m.show();
    m.exec();
}
