#include "videoprocessor.h"
#include <QDebug>
#include <QDate>
#include <QMessageBox>

VideoProcessor::VideoProcessor(QObject *parent) : QObject(parent)
{
    isOpen = false;
    isForeDetect = true;
    detecModel = 4;
    allowdAlarm = true;
    oldDetectNum = 0;
    newDetectNum = 0;
    isDebug = false;
    detectSumMax = 20;//检测帧累计最大值
    detectSum = 1;//检测帧累计数，减少偶然干扰，初始值为不为0，为了指数增长
    proDetectArea = 0;//上次检测到的轮廓总面积
    currentDetectArea = 0;//本次检测到的轮廓总面积
    frameNumber = 1;
    changeThreshold = 10;//vibe算法检测无火焰切换回帧差法的阈值
    nofireFrameCount = 0;
    isVibe = false;//是否采用Vibe法
    isVibeInit = false;//Vibes是否初始化
}



VideoProcessor::VideoProcessor(const int videoId)
{
    isOpen = false;
    isForeDetect = true;
    detecModel = 4;
    isDebug = false;
    allowdAlarm = true;
    oldDetectNum = 0;
    newDetectNum = 0;
    detectSumMax = 20;//检测帧累计最大值
    detectSum = 1;//检测帧累计数，减少偶然干扰，初始值为不为0，为了指数增长
    proDetectArea = 0;//上次检测到的轮廓总面积
    currentDetectArea = 0;//本次检测到的轮廓总面积
    frameNumber = 1;
    changeThreshold = 10;//vibe算法检测无火焰切换回帧差法的阈值
    nofireFrameCount = 0;
    this->videoID = videoId;
    isVibe = false;//是否采用Vibe法
    isVibeInit = false;//Vibes是否初始化

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

void VideoProcessor::processVideoWithViBe(int videoID)
{
    this->videoID = videoID;
    QImage img;
    QPixmap pixmap;
    bool isFired = false;
    Mat rgbframe;
    //读取一帧
    if(video->isOpened())
    {
        if(!video->read(frame))
        {
            return;
          //  video->set(CV_CAP_PROP_POS_FRAMES,0);//读取结束后从头开始
          //  video->read(currentFrame);
        }
    }
    if(frame.rows == 0)//防止读取到空
        return;
    if(!isVibe) // 采用帧差法先提取大致区域
    {
        cvtColor(frame,frameDiff,CV_BGR2GRAY);
        if (frameNumber == 1)
        {
            proFrame = frameDiff.clone();
            proFrameVive = frame.clone();
        }
        if ((frameNumber % 100) == 0)
        {
            cout << "Frame number = " << frameNumber << endl;
        }
        //更新背景并返回前景
        absdiff(proFrame, frameDiff, foreFrame);
        //二值化
        threshold(foreFrame, foreFrame, 20, 255, CV_THRESH_BINARY);
        //中值滤波
        medianBlur(foreFrame, foreFrame, 3); /* 3x3 median filtering */
        proFrame = frameDiff.clone();
        //获取轮廓
        vector<vector<Point> > contours;//保存轮廓用的向量
        vector<Point> Points;//所有轮廓的点集，用来创建包含所有轮廓的最小矩形
        Mat temimg = foreFrame.clone();
        findContours(temimg,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
        for(int i = 0;i< contours.size();i++)
        {
            Points.insert(Points.end(),contours[i].begin(),contours[i].end());
        }
        if(Points.size() != 0)
        {
            vibeRect = boundingRect(Points);
            frameVibeArea = proFrameVive(vibeRect);//截取目标区域
            //调用vibe算法
            isVibe = true;
            if(!isVibeInit)
            {
                cout<<"Change to Vive"<<endl;
                cvtColor(frameVibeArea, frameVibe, CV_BGR2YCrCb);

                vibe.init(frameVibe, FireVibe::Config::getRGBConfig());
                isVibeInit = true;
            }
        }
        proFrameVive = frame.clone();
    }
    else//使用Vibe算法来检测目标区域
    {
        frameVibeArea = frame(vibeRect);//截取目标
        cvtColor(frameVibeArea, frameVibe, CV_BGR2YCrCb);
        Mat tempforeframe(frame.rows,frame.cols,CV_8UC1,Scalar(0));//纯黑临时帧,叠加目标区域
        frameVibeArea = tempforeframe(vibeRect);//截取目标
        //更新背景并返回前景
        vibe.update(frameVibe, frameVibeArea);
        medianBlur(frameVibeArea, frameVibeArea, 3); /* 3x3 中值滤波 */
        foreFrame = tempforeframe.clone();//显示vibe检测的结果
        vector<vector<Point> > currentContours;
        currentContours = fireColorMatch(frame,foreFrame,vibeRect,0);
        for(int i=0; i<currentContours.size(); i++)
        {
            currentDetectArea += fabs(contourArea(currentContours[i]));
        }
        if(currentDetectArea > proDetectArea)
        {
            if(detectSum < detectSumMax*2)
                detectSum *= 2;//急速增长
            nofireFrameCount = 0;

        }
        else
        {
            if(currentDetectArea > 0)
            {
                if(detectSum < detectSumMax*2)
                    detectSum ++;//增长比下降快
                nofireFrameCount = 0;
            }
            else
            {
                if(detectSum > 1)//初始值为1
                {
                    detectSum--;
                    nofireFrameCount = 0;
                }
                if(detectSum == 1)
                    nofireFrameCount++;
            }
        }
        proDetectArea = currentDetectArea;
        currentDetectArea = 0;
        if(detectSum > detectSumMax)
        {
            isFired = true;
            //绘制轮廓
            if(currentContours.size() > 0)
            {

                vector<Point> Points;
                vector<vector<Point> > littleArea;
                for(int i = 0;i< currentContours.size();i++)
                {
                    //为了显示效果，大于一定面积的区域进行合并
                    if(currentContours[i].size() > 10 )
                        Points.insert(Points.end(),currentContours[i].begin(),currentContours[i].end());
                    else
                        littleArea.push_back(currentContours[i]);
                }
                if(Points.size() > 0)
                {
                    Scalar color(0,0,255);
                    Rect rect = boundingRect(Points);
                    rectangle(frame,rect,color,2);//在源图像上绘制轮廓矩形
                }
                for(int i=0;i<littleArea.size();i++)
                {
                    Scalar color(0,0,255);
                    Rect rect = boundingRect(littleArea.at(i));
                    rectangle(frame,rect,color,2);//在源图像上绘制小轮廓矩形
                }
                procontours = currentContours;
            }
            else
            {
                //绘制一次上一次检测到的轮廓-显示效果
                if(procontours.size() >0)//绘制上一次检测到的轮廓
                {
                    vector<Point> Points;
                    vector<vector<Point> > littleArea;
                    for(int i = 0;i< procontours.size();i++)
                    {
                        //为了显示效果，大于一定面积的区域进行合并
                        if(procontours[i].size() > 10 )
                            Points.insert(Points.end(),procontours[i].begin(),procontours[i].end());
                        else
                            littleArea.push_back(procontours[i]);
                    }
                    if(Points.size() > 0)
                    {
                        Scalar color(0,0,255);
                        Rect rect = boundingRect(Points);
                        rectangle(frame,rect,color,2);//在源图像上绘制轮廓矩形
                    }

                    for(int i=0;i<littleArea.size();i++)
                    {
                        Scalar color(0,0,255);
                        Rect rect = boundingRect(littleArea.at(i));
                        rectangle(frame,rect,color,2);//在源图像上绘制小轮廓矩形
                    }
                }
                procontours = currentContours;
            }
        }
        else
        {
            if(procontours.size() >0)//绘制上一次检测到的轮廓
            {
                vector<Point> Points;
                vector<vector<Point> > littleArea;
                for(int i = 0;i< procontours.size();i++)
                {
                    //为了显示效果，大于一定面积的区域进行合并
                    if(procontours[i].size() > 10 )
                        Points.insert(Points.end(),procontours[i].begin(),procontours[i].end());
                    else
                        littleArea.push_back(procontours[i]);
                }
                if(Points.size() > 0)
                {
                    Scalar color(0,0,255);
                    Rect rect = boundingRect(Points);
                    rectangle(frame,rect,color,2);//在源图像上绘制轮廓矩形
                }

                for(int i=0;i<littleArea.size();i++)
                {
                    Scalar color(0,0,255);
                    Rect rect = boundingRect(littleArea.at(i));
                    rectangle(frame,rect,color,2);//在源图像上绘制小轮廓矩形
                }
            }
        }

        if(nofireFrameCount > changeThreshold)//没有检测到火焰，切换回帧差
        {
            isFired = false;
           cout<<"Change to frame diff"<<endl;
           procontours.clear();
           nofireFrameCount = 0;
           isVibe = false;
           isVibeInit = false;
           cvtColor(frame,frameDiff,CV_BGR2GRAY);
           proFrame = frameDiff.clone();
        }
    }



    ++frameNumber;
    if(frame.channels() == 3 )//3通道
    {
       cvtColor(frame,rgbframe,CV_BGR2RGB);
       img = QImage((const uchar*)(rgbframe.data),rgbframe.cols,rgbframe.rows,
                    rgbframe.channels()*rgbframe.cols,QImage::Format_RGB888);
    }
    else
    {
        img = QImage((const uchar*)(frame.data),frame.cols,frame.rows,
                    frame.channels()*frame.cols,QImage::Format_Indexed8);
    }
    pixmap = QPixmap(QPixmap::fromImage(img));
    emit processEnd(videoID,pixmap);
    if(isFired)
    {
       QString saveFileName = QString("./logimages/channel%1-%2.jpg").arg(videoID+1).arg(QDate::currentDate().toString("yyyy.MM.dd"));
       imwrite(saveFileName.toStdString(),frame);
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
        
            double ds = (255.0 - color_R)*RT/ST;
            double S = color_S/255.0;//将饱和度转换回 [0-1]
           
            //RGB约束模型 1.R>RT 2.R>=G>=B 3.S>=(255-R)*ST/RT
            if(color_R > RT && color_R > color_G && color_G > color_B
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
    cvtColor(tempimg,dst,CV_BGR2GRAY);
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
    newDetectNum = hierarchy.size();
    if(newDetectNum > oldDetectNum)
    {
       if(detectSum < detectSumMax*2)
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
            if(detectSum < detectSumMax*2)
            detectSum ++;
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
            if(detectSum > 0)
            detectSum--;
        }
    }
    oldDetectNum = newDetectNum;
    return is_Fire;
}

bool VideoProcessor::FiredetecetionFrame(Mat &resultFrame, Mat &frame, int model)
{
     bool is_Fire = false;
     int detectSumMax = 20;//检测帧累计最大值
     int detectSum = 1;//检测帧累计数，减少偶然干扰，初始值为不为0，为了指数增长
     double proDetectArea = 0;//上次检测到的轮廓总面积
     double currentDetectArea = 0;//本次检测到的轮廓总面积
     int detecModel = model;//颜色检测模型选择
     int frameNumber = 1;
     int changeThreshold = 10;//vibe算法检测无火焰切换回帧差法的阈值
     int nofireFrameCount = 0;
     vector<vector<Point> > procontours;//前次绘制的矩形，累积多帧没有检测到火焰后才取消绘制
     Mat frameDiff;//帧差法要处理的当前帧
     Mat proFrame;
     Mat frameVibe;//vibe法要处理的当前帧
     Mat frameVibeArea;//vibe法要处理的区域
     Mat proFrameVive;//帧差中提取出背景帧，作为ViBe初始帧-帧差中检测出运动物体的前一帧
     Mat foreFrame;
     Rect vibeRect;//感兴趣区域，标注VIBE算法处理区域矩形
    // FireVibe vibe;
     bool isVibe = false;//是否采用Vibe法
     bool isVibeInit = false;//Vibes是否初始化
}

void VideoProcessor::initProcessor()
{
    if(isOpen)//视频打开成功
    {
        double rate = video->get(CV_CAP_PROP_FPS);//获得帧率
        delayTime = 1000/rate;//帧之间时间间隔
    }
}




//--------------------
void MyMerge(vector<int> &array, int l,int mid,int r, int &count)//归并排序合并
{
    int i,j,k,n1,n2;
    n1 = mid - l +1;
    n2 = r - mid;
    vector<int> tempL;
    vector<int> tempR;
    for(i = 0;i<n1;i++)
    {
        tempL.push_back(array[l+i]);
    }
    for(i=0;i<n2;i++)
    {
        tempR.push_back(array[mid+i]);
    }
    i = 0;
    j = 0;
    for(k=l;k<r && i<n1 && j<n2;k++)
    {
        if(tempL[i]<tempR[j])//相等情况不算正序
        {
            array[k] = tempL[i++];
            count += n2-j;
        }
        else
        {
            if(tempL[i] > tempL[j])
            {
                count--;
            }
            //count--;
            array[k] = tempR[j++];


        }
    }
    while(i<n1)
    {
        array[k++] = tempL[i++];
    }
    while(j<n2)
    {
        array[k++] = tempR[j++];
    }
}

void MergeSort(vector<int> &array, int l, int r, int &count)//归并排序求正序对
{
    int mid;
    if(l == r)
        return;
    if(r > array.size())
        return;
    mid = l+((r-l)>>1);
    MergeSort(array,l,mid,count);
    MergeSort(array,mid+1,r,count);
    MyMerge(array,l,mid,r,count);
}
//基于正序对的火焰尖角检测
bool VideoProcessor::triangleDetect2(Mat &frame, std::vector<Point> &area)
{
    Rect rect = boundingRect(area);
    int currentPointCount=0;//当前行的前景点数（白色点）
    double matchNum = 0;//正序行数-上一行点数小于下一行
    double matchThreshold = 0.6;//正序数检测阈值
    Point startPoint = rect.tl();//左上角
    Point endPoint = rect.br();//右下角
    int totalRows = rect.height;//总行数
    int totalNum = totalRows*(totalRows-1)/2;//排列组合公式 CN25
    int seqNum = 0;//正序对个数
    vector<int> pointCounts;//保存每行的前景像素点数
    if(totalNum <= 1)
    {
        cout<<"rows<1"<<endl;
        return false;
    }

    for(int row=startPoint.y; row<=endPoint.y; row++)//遍历列
    {
        currentPointCount = 0;
        for(int col=startPoint.x; col<=endPoint.x; col++)
        {

            if(frame.at<uchar>(row,col) == 255)//用at方式访问时，参数先y坐标再x坐标
                currentPointCount++;
        }
        pointCounts.push_back(currentPointCount);
    }
    MergeSort(pointCounts, 0, totalRows, seqNum);//求出正序对数
    matchNum = (double)seqNum/(double)totalNum;
    if(matchNum >= matchThreshold)
        return true;
    else
        return false;

}

//火焰颜色模型匹配
vector<vector<Point> > VideoProcessor::fireColorMatch(Mat &frame, Mat &detecframe, Rect &rect, int model)
{
    Point startPoint = rect.tl();//起始检测点
    Vec3b pixe,pixel;
    Vec3b pixe2;
    Scalar scalar,scalarRGB;
    int i,j;
    bool isMeetRGB=false,isMeetYCC=false,isMeetHSI=false;//三种颜色模型满足条件
    bool isDetected = false;//检测到符合火焰点
    int myThreshold = 35;//YCbCr模型 阈值
    double RT = 115;//RGB模型 R通道阈值
    double ST = 55;//RGB 模型 饱和度阈值
    Mat imgYcrcb;
    Mat imgHSI;
    Mat detecImg = detecframe.clone();
    vector<vector<Point> > contours;
    cvtColor(frame,imgHSI,CV_BGR2HLS);//RGB转换为HSI图形
    cvtColor(frame,imgYcrcb,CV_BGR2YCrCb);//RGB图转换为YCRCB（RGB图在opencv中存储顺序为BGR）
    scalar = mean(imgYcrcb);//计算图像各个通道像素值均值
    scalarRGB = mean(imgHSI);
    for(i=startPoint.y; i<startPoint.y+rect.height;i++)
    {
        for(j=startPoint.x; j<startPoint.x+rect.width;j++)
        {
            if(detecImg.at<uchar>(i,j) == 0)//黑色，背景，跳过
                continue;
            pixe = frame.at<Vec3b>(i,j);
            uchar color_B = pixe[0];
            uchar color_G = pixe[1];
            uchar color_R = pixe[2];
            uchar color_min = color_R > color_G?color_G:color_R;
            color_min = color_min>color_B?color_B:color_min;

            pixel = imgYcrcb.at<Vec3b>(i,j);
            uchar color_Y = pixel[0];
            uchar color_Cr = pixel[1];
            uchar color_Cb = pixel[2];

            pixe2 =  imgHSI.at<Vec3b>(i,j);

            uchar color_H = pixe2[0];
            uchar color_L = pixe2[1];
            uchar color_S = pixe2[2];
            RT = 135;
            ST = 45;
            double ds = (255.0 - color_R)*RT/ST;
            double S = color_S/255.0;//将饱和度转换回 [0-1]
            //RGB约束模型 1.R>RT 2.R>=G>=B 3.S>=(255-R)*ST/RT
            if(color_R > RT && color_R > color_G && color_G > color_B
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
            }
            else
            {
                detecImg.at<uchar>(i,j) = 0;//不符合点置为背景
            }

        }
    }
 //   imshow("colordetected",detecImg);
    Mat element5(8,8,CV_8U,Scalar(1));
    dilate(detecImg,detecImg,element5);//膨胀
    erode(detecImg,detecImg,element5);//腐蚀，闭运算
 //   imshow("dilate",detecImg);
    Mat temimg = detecImg.clone();
    findContours(temimg,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    vector<vector<Point> > result;
    Mat temtriangleimg = detecImg.clone();//显示排除尖角后的图像
    for(int i=0;i<contours.size();i++)
    {
        //进行尖角检测，排除不符合火焰尖角的区域
        if(triangleDetect2(temtriangleimg,contours[i]))
        {
            result.push_back(contours[i]);
        }
        else
        {
            //将不符合尖角区域填充为背景
           Rect rect = boundingRect(contours[i]);
           Mat imgRoi = temtriangleimg(rect);
           Mat tempMsk(imgRoi.rows, imgRoi.cols,CV_8UC1,Scalar(0));
           tempMsk.copyTo(imgRoi);
           /*for(int j=0; j<contours[i].size();j++)
               temtriangleimg.at<uchar>(contours[i][j]) = 0;//不符合尖角设为背景*/
        }

    }
//    imshow("aftertriangle",temtriangleimg);
    return result;
}
