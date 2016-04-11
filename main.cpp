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
/*
Mat ImgPreProcess(Mat rgbimg)
{

    Vec3b pixe,pixel;
    Vec3b pixe2;
    Scalar scalar,scalarRGB;
    int i,j;
    int myThreshold = 35;//YCbCr模型 阈值
    double RT = 115;//RGB模型 R通道阈值
    double ST = 55;//RGB 模型 饱和度阈值
    Mat tempimg(rgbimg.rows,rgbimg.cols,CV_8UC3);
    Mat hsiImg(rgbimg.rows,rgbimg.cols,CV_8UC3);
    Mat dst(rgbimg.rows,rgbimg.cols,CV_8UC1);


    cvtColor(rgbimg,hsiImg,CV_BGR2HLS);
    cvtColor(rgbimg,tempimg,CV_BGR2YCrCb);//RGB图转换为YCRCB（RGB图在opencv中存储顺序为BGR）
    scalar = mean(tempimg);//计算图像各个通道像素值均值
    scalarRGB = mean(rgbimg);
    for(i=0;i<tempimg.rows;i++)
    {
        for(j=0;j<tempimg.cols;j++)
        {


            pixe = rgbimg.at<Vec3b>(i,j);
            uchar color_B = pixe[0];
            uchar color_G = pixe[1];
            uchar color_R = pixe[2];
            uchar color_min = color_R > color_G?color_G:color_R;
            color_min = color_min>color_B?color_B:color_min;

            pixel = tempimg.at<Vec3b>(i,j);
            uchar color_Y = pixel[0];
            uchar color_Cr = pixel[1];
            uchar color_Cb = pixel[2];

            pixe2 =  hsiImg.at<Vec3b>(i,j);

            uchar color_H = pixe2[0];
            uchar color_L = pixe2[1];
            uchar color_S = pixe2[2];

            double d1 = (double)color_G/(color_R + 1.0);
            double d2 = (double)color_B/(color_R + 1.0);
            double d3 = (double)color_B/(color_G + 1.0);
            double ds = (255.0 - color_R)*RT/ST;
            double S = color_S/255.0;//将饱和度转换回 [0-1]
           // double S1 = 1-3.0*color_min/(color_R + color_G + color_B);
            //RGB约束模型 1.R>RT 2.R>=G>=B 3.S>=(255-R)*ST/RT
            if(color_R > RT && color_R >= color_G && color_G >= color_B
                    && S >= ds)
            {
                 //YCbCr 模型6个约束条件
                if(color_Y > color_Cb && color_Cr > color_Cb && color_Y > scalar.val[0] &&color_Cb
                        <scalar.val[2] && color_Cr > scalar.val[1] && color_Cr-color_Cb> myThreshold)
                {

                   // HSL模型--H>60 0.2<S<1 0.5<L<1
                    if(color_H <= 60 && color_S >= 0.2*255 && color_S <= 255 && color_L >= 0.5*255 &&color_L <= 255 )
                    {
                        pixel[0] = 255;//白色
                        pixel[1] = 255;
                        pixel[2] = 255;
                        tempimg.at<Vec3b>(i,j) = pixel;
                        continue;
                    }

                }
            }
            pixel[0] = 0;
            pixel[1] = 0;
            pixel[2] = 0;
            tempimg.at<Vec3b>(i,j) = pixel;
        }
    }
    //cvtColor(tempimg,tempimg,CV_YCrCb2BGR);
    cvtColor(tempimg,dst,CV_BGR2GRAY);
   // cvtColor(tempimg2,hsiImg,CV_HLS2BGR);
    return dst;
}

bool FireDectetion_Img(Mat &rgbimg)
{
    bool is_Fire = false;
    Mat result_img = ImgPreProcess(rgbimg);
    imshow("processed",result_img);
    //均值滤波:输入图像，输出图像，模板大小，被平滑点位置（负值为中心）
    blur(result_img,result_img,Size(5,5),Point(0,0));
    imshow("filter1",result_img);
    threshold(result_img,result_img,40,255,CV_THRESH_BINARY);//二值化
    imshow("erzhihua",result_img);
    //高斯滤波:输入图像，输出图像，模板大小（奇数），x方向差，y方向差
    GaussianBlur(result_img,result_img,Size(5,5),0,0);//高斯滤波
    imshow("filter",result_img);
    erode(result_img,result_img,Mat());//腐蚀
    dilate(result_img,result_img,Mat());//膨胀
    imshow("pengzhang",result_img);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //查找轮廓
    findContours(result_img,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
    if(hierarchy.size() > 0)
    {
        for(int idx = 0;idx>=0;idx=hierarchy[idx][0])
        {
           Scalar color( rand()&255, rand()&255, rand()&255 );
           drawContours( result_img, contours, idx, color, CV_FILLED, 8, hierarchy );

           Rect rect = boundingRect(contours.at(idx));
          // Rect ret = rect.boundingRect();
           rectangle(rgbimg,rect,Scalar(255));
        }
        is_Fire = true;
    }
    drawContours(result_img,contours,-1,Scalar(255),1);
    imshow("test",result_img);
    imshow("result",rgbimg);
    return is_Fire;
}
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    MainFrame w;
    a.setApplicationName("FireDetection");
    a.setApplicationVersion("V201509");


    w.show();
    w.setGeometry(qApp->desktop()->availableGeometry());//全屏运行

 /*   SmtpClient smtp("smtp.163.com",25);
    smtp.setUser("jxau764@163.com");
    smtp.setPassword("zwaayylqqdtoqzev");
    MimeMessage message;
    message.setSender(new EmailAddress("jxau764@163.com"));
    message.addRecipient(new EmailAddress("zhouzhongtaosoft@163.com"));
    message.setSubject(("测试一下邮件发送"));

    MimeText text;
    text.setText(("通道1检测到疑似火情"));
    message.addPart(&text);
    MimeHtml html;
    html.setHtml("<h1> Detected!<h1>"
                 "<h2> The detected iamge is below<h2>"
                 "<img src='cid:image1'/>");
    MimeInlineFile image1(new QFile("./logimages/1.jpg"));
    image1.setContentId("image1");;
    image1.setContentType("image/jpg");

    message.addPart(&html);
    message.addPart(&image1);

    smtp.connectToHost();
    if(!smtp.login())
    {
        qDebug()<<"login failed";
    }
    if(!smtp.sendMail(message))
    {
        qDebug()<<"send failed";
    }
    smtp.quit();*/

   /* QString fileName = QFileDialog::getOpenFileName(0,"选择视频文件",".","video files(*.avi *.mp4 *.rmvb)");
    String str = fileName.toStdString();
    VideoCapture video(str);
    if(!video.isOpened())
        return 1;
    Mat frame,foreground;
    BackgroundSubtractorMOG mog;
    bool stop(false);
    while (!stop) {
        if(!video.read(frame))
            break;
        //更新背景并返回
        mog(frame,foreground,0.01);
    //    threshold(foreground,foreground,128,255,cv::THRESH_BINARY_INV);
        //显示前景
        imshow("foreground",frame);
        if(waitKey(10)>=0)
            stop = true;
    }*/

  // 测试代码
 /*   VideoCapture capture_0("./firevideo/forest3.avi");
    if(!capture_0.isOpened())
    {
        return 1;
    }
    //获取帧率
    double rate = capture_0.get(CV_CAP_PROP_FPS);
    bool stop(false);
    Mat frame;
    int delay = 1000/rate;//每一帧的延迟，与视频帧率相当
    //遍历每一帧
    while(!stop)
    {
        //读取下一帧
        if(!capture_0.read(frame))
            break;
        FireDectetion_Img(frame);
        imshow("testvideo",frame);
        //延迟-也可通过按键停止
        if(waitKey(delay) >= 0)
        {
            stop = true;
        }
    }




    destroyAllWindows();*/
    return a.exec();
}
