#include "videoprocessor.h"
#include <QDebug>
#include <QDate>
#include <QMessageBox>

VideoProcessor::VideoProcessor(QObject *parent) : QObject(parent)
{
    isOpen = false;
    isForeDetect = true;
    detectSum = 1;
    detecModel = 4;
    allowdAlarm = true;
    detectSumMax = 50;
    oldDetectNum = 0;
    newDetectNum = 0;
    isDebug = false;
}



VideoProcessor::VideoProcessor(const int videoId)
{
    isOpen = false;
    isForeDetect = true;
    detectSum = 1;
    detecModel = 4;
    isDebug = false;
    allowdAlarm = true;
    detectSumMax = 50;
    oldDetectNum = 0;
    newDetectNum = 0;
    this->videoID = videoId;
}


bool VideoProcessor::openVideo(const int deviceId)
{
    video = new VideoCapture(deviceId);//打开对应的摄像头
    isOpen = video->isOpened();
    isForeDetect = true;
    detectSum = 1;
    detecModel = 4;
    isDebug = false;
    allowdAlarm = true;
    detectSumMax = 50;
    oldDetectNum = 0;
    newDetectNum = 0;
    initProcessor();
    return isOpen;
}

bool VideoProcessor::openVideo(const QString fileName)
{
    String str = fileName.toStdString();
    video = new VideoCapture(str);
    isOpen = video->isOpened();
    isForeDetect = true;
    detectSum = 1;
    detecModel = 4;
    isDebug = false;
    allowdAlarm = true;
    oldDetectNum = 0;
    newDetectNum = 0;
    initProcessor();
    detectSumMax = MAXDETECSUM;
    return isOpen;
}

bool VideoProcessor::isOpenSucceed()
{
    return isOpen;
}

int VideoProcessor::getFrameTime()
{
    return delayTime;
}

bool VideoProcessor::isForeDetectOpen()
{
    return isForeDetect;
}

int VideoProcessor::getDetectModel()
{
    return detecModel;
}

void VideoProcessor::setDetectModel(int model)
{
    detecModel = model;
}

void VideoProcessor::videoRelease()
{
    if(isOpen)
    {
        video->release();
    }
}

void VideoProcessor::processVideo(int videoID)
{
    this->videoID = videoID;
    Mat tempFrame,rgbframe,foregroundFrame,currentFrame;
    QImage img;
    QPixmap pixmap;
    bool isFired = false;
    //读取一帧
    if(video->isOpened())
    {
        if(!video->read(currentFrame))
        {
            video->set(CV_CAP_PROP_POS_FRAMES,0);//读取结束后从头开始
            video->read(currentFrame);
        }
    }
    if(currentFrame.rows == 0)//防止读取到空
        return;
    currentFrame.copyTo(tempFrame);
    if(isForeDetect)//前景检测
    {
        mog(currentFrame,foregroundFrame,0.01);//更新背景并返回前景
        for(int i =0;i< foregroundFrame.rows;i++)
        {
              for(int j = 0;j<foregroundFrame.cols;j++)
              {
                  if(foregroundFrame.at<uchar>(i,j) == 0)//黑色
                  {
                      tempFrame.at<Vec3b>(i,j)[0] = 0;
                      tempFrame.at<Vec3b>(i,j)[1] = 0;
                      tempFrame.at<Vec3b>(i,j)[2] = 0;
                  }
                  else
                  {
                      tempFrame.at<Vec3b>(i,j)[0] = currentFrame.at<Vec3b>(i,j)[0];
                      tempFrame.at<Vec3b>(i,j)[1] = currentFrame.at<Vec3b>(i,j)[1];
                      tempFrame.at<Vec3b>(i,j)[2] = currentFrame.at<Vec3b>(i,j)[2];
                  }
              }
         }
    }


    if(isDebug)
    {
       imshow("Dynamic foreground detect result",tempFrame);
    }
     isFired = FiredectetionImg(tempFrame,currentFrame,0);
     if(currentFrame.channels() == 3 )//3通道
     {
        cvtColor(currentFrame,rgbframe,CV_BGR2RGB);
        img = QImage((const uchar*)(rgbframe.data),rgbframe.cols,rgbframe.rows,
                     rgbframe.channels()*rgbframe.cols,QImage::Format_RGB888);
     }
     else
     {
         img = QImage((const uchar*)(currentFrame.data),currentFrame.cols,currentFrame.rows,
                     currentFrame.channels()*currentFrame.cols,QImage::Format_Indexed8);
     }
     pixmap = QPixmap(QPixmap::fromImage(img));
     emit processEnd(videoID,pixmap);
     if(isFired)
     {
        QString saveFileName = QString("./logimages/channel%1-%2.jpg").arg(videoID+1).arg(QDate::currentDate().toString("yyyy.MM.dd"));
        imwrite(saveFileName.toStdString(),currentFrame);
        emit sendAlarm(this->videoID,saveFileName);
     }
}

void VideoProcessor::nextFrameTimeOut()
{

}

void VideoProcessor::setIsForeDetect(bool value)
{
    isForeDetect = value;
}

bool VideoProcessor::getDebugModel()
{
    return isDebug;
}

void VideoProcessor::setDebugModel(bool value)
{
    isDebug = value;
}
int VideoProcessor::getDetectSumMax() const
{
    return detectSumMax;
}

void VideoProcessor::setDetectSumMax(int value)
{
    detectSumMax = value;
}

bool VideoProcessor::getAllowdAlarm() const
{
    return allowdAlarm;
}

void VideoProcessor::setAllowdAlarm(bool value)
{
    allowdAlarm = value;
}


Mat VideoProcessor::ImgPreProcess(Mat rgbimg, int model)
{
    Vec3b pixe,pixel;
    Vec3b pixe2;
    Scalar scalar,scalarRGB;
    int i,j;
    bool isMeetRGB=false,isMeetYCC=false,isMeetHSI=false;//三种颜色模型满足条件
    bool isDetected = false;//检测到符合火焰点
    int myThreshold = 35;//YCbCr模型 阈值
    double RT = 115;//RGB模型 R通道阈值
    double ST = 55;//RGB 模型 饱和度阈值
    Mat tempimg(rgbimg.rows,rgbimg.cols,CV_8UC3);
    Mat hsiImg(rgbimg.rows,rgbimg.cols,CV_8UC3);
    Mat dst(rgbimg.rows,rgbimg.cols,CV_8UC1);


    cvtColor(rgbimg,hsiImg,CV_BGR2HLS);//RGB转换为HSI图形
    cvtColor(rgbimg,tempimg,CV_BGR2YCrCb);//RGB图转换为YCRCB（RGB图在opencv中存储顺序为BGR）
    scalar = mean(tempimg);//计算图像各个通道像素值均值
    scalarRGB = mean(rgbimg);
    for(i=0;i<rgbimg.rows;i++)
    {
        for(j=0;j<rgbimg.cols;j++)
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

           /* double d1 = (double)color_G/(color_R + 1.0);
            double d2 = (double)color_B/(color_R + 1.0);
            double d3 = (double)color_B/(color_G + 1.0);*/
            double ds = (255.0 - color_R)*RT/ST;
            double S = color_S/255.0;//将饱和度转换回 [0-1]
           // double S1 = 1-3.0*color_min/(color_R + color_G + color_B);
            //RGB约束模型 1.R>RT 2.R>=G>=B 3.S>=(255-R)*ST/RT
            if(color_R > RT && color_R >= color_G && color_G >= color_B
                    && S >= ds)
            {
                isMeetRGB = true;
            }
             //YCbCr 模型6个约束条件
            if(color_Y > color_Cb && color_Cr > color_Cb && color_Y > scalar.val[0] &&color_Cb
                    <scalar.val[2] && color_Cr > scalar.val[1] && color_Cr-color_Cb> myThreshold)
            {
                isMeetYCC = true;
            }
            // HSL模型--H>60 0.2<S<1 0.5<L<1
            if(color_H <= 60 && color_S >= 0.2*255 && color_S < 255 && color_L >= 0.5*255 &&color_L < 255 )
            {
                isMeetHSI = true;
            }
            switch(model)
            {
            case 0:
                if(isMeetRGB)
                {
                    isMeetRGB = false;
                    isDetected = true;
                }
                break;
            case 1:
                if(isMeetYCC)
                {
                    isMeetYCC = false;
                    isDetected = true;
                }
                break;
            case 2:
                if(isMeetHSI)
                {
                    isMeetHSI = false;
                    isDetected = true;
                }
                break;
            case 3:
                if(isMeetYCC && isMeetHSI)
                {
                    isMeetYCC = false;
                    isMeetHSI = false;
                    isDetected = true;
                }
                break;
            case 4:
                if(isMeetRGB && isMeetYCC && isMeetHSI)
                {
                    isMeetRGB = false;
                    isMeetYCC = false;
                    isMeetHSI = false;
                    isDetected = true;
                }
                break;
            }
            if(isDetected)
            {
                isDetected = false;
                pixel[0] = 255;//白色
                pixel[1] = 255;
                pixel[2] = 255;
                tempimg.at<Vec3b>(i,j) = pixel;
                continue;
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

bool VideoProcessor::FiredectetionImg(Mat &detecimg, Mat &sourceimg, int model)
{
    bool is_Fire = false;
    Mat result_img = ImgPreProcess(detecimg,model);
    if(isDebug)
    {
       imshow("After color model processes",result_img);
    }
    else
    {
        destroyAllWindows();
    }
    //均值滤波:输入图像，输出图像，模板大小，被平滑点位置（负值为中心）
    blur(result_img,result_img,Size(5,5),Point(0,0));
    threshold(result_img,result_img,40,255,CV_THRESH_BINARY);//二值化
   // threshold(result_img,result_img,40,255,CV_THRESH_OTSU);
    //高斯滤波:输入图像，输出图像，模板大小（奇数），x方向差，y方向差
    GaussianBlur(result_img,result_img,Size(5,5),0,0);//高斯滤波
    erode(result_img,result_img,Mat());//腐蚀
    dilate(result_img,result_img,Mat());//膨胀
    if(isDebug)
    {
        imshow("Morphological processed",result_img);
    }
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //查找轮廓
    findContours(result_img,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
   // imshow("findContors",result_img);
    newDetectNum = hierarchy.size();
    if(newDetectNum > oldDetectNum)
    {
       detectSum += 2;//增长步长比下降快-检测快 消失慢
       if(detectSum > detectSumMax)
       {
             is_Fire = true;
             for(int idx = 0;idx>=0;idx=hierarchy[idx][0])//绘制矩形圈出火焰
             {
                Scalar color(0,0,255);
                Rect rect = boundingRect(contours.at(idx));
                rectangle(sourceimg,rect,color,2);//在源图像上绘制轮廓矩形
             }
       }
    }
    else
    {
        if(newDetectNum > 0)
        {
            detectSum++;
            if(detectSum > detectSumMax)
            {
                  is_Fire = true;
                //  emit sendAlarm(videoID);
                  for(int idx = 0;idx>=0;idx=hierarchy[idx][0])//绘制矩形圈出火焰
                  {
                     Scalar color(0,0,255);
                     Rect rect = boundingRect(contours.at(idx));
                     rectangle(sourceimg,rect,color,2);//在源图像上绘制轮廓矩形
                  }
            }
        }
        else
        {
            detectSum--;
        }
    }
    oldDetectNum = newDetectNum;
    return is_Fire;
}

void VideoProcessor::initProcessor()
{
    if(isOpen)//视频打开成功
    {
        double rate = video->get(CV_CAP_PROP_FPS);//获得帧率
        delayTime = 1000/rate;//帧之间时间间隔
    }
}
