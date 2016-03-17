#include "mainframe.h"
#include <iostream>
#include <math.h>
#include <QDebug>
#include <QTextCodec>
#include <QDesktopWidget>
#include <QApplication>
#include <QFileDialog>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "sysconfiginfo.h"
#include "./src/SmtpMime"
using namespace cv;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    MainFrame w;
    a.setApplicationName("FireDetection");
    a.setApplicationVersion("V201509");


    w.show();
    w.setGeometry(qApp->desktop()->availableGeometry());//全屏运行
    return a.exec();
}
