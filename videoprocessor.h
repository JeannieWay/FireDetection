#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "firevibe.h"
using namespace cv;
using namespace std;
#define MAXDETECSUM 20
class VideoProcessor : public QObject
{
    Q_OBJECT
public:
    explicit VideoProcessor(QObject *parent = 0);
    VideoProcessor(const int videoId);
    bool openVideo(const int deviceId);
    bool openVideo(const QString fileName);
    bool isOpenSucceed();
    int getFrameTime();
    bool isForeDetectOpen();
    int getDetectModel();
    void setDetectModel(int model);
    void videoRelease();

    bool getAllowdAlarm() const;
    void setAllowdAlarm(bool value);

    int getDetectSumMax() const;
    void setDetectSumMax(int value);

signals:
    void processCompleted(Mat&);
    void sendAlarm(int videoID,QString fileName);
    void processEnd(int videoID,QPixmap pixmap);

public slots:
    void processVideo(int videoID);
    void processVideoWithViBe(int videoID);
    void nextFrameTimeOut();
    void setIsForeDetect(bool value);
    bool getDebugModel();
    void setDebugModel(bool value);
private:
    VideoCapture *video;//视频源
    int videoID;
   // Mat frame;//帧数据
    BackgroundSubtractorMOG mog;//混合高斯对象
    int delayTime;//帧延时时间
    QTimer *timer;
    bool isNextFrame;//是否可以读取下一帧
    bool allowdAlarm;
    bool isOpen;//视频是否打开
    bool isForeDetect;//是否进行运动前景提取
    bool isDebug;//调试模式显示中间过程图
   // int detectSumMax;//检测帧累计最大值
   // int detectSum;//检测帧累计数，减少偶然干扰
    int oldDetectNum;//上次检测到的轮廓数
    int newDetectNum;//本次检测到的轮廓数
    //int detecModel;//颜色检测模型选择
    //------------------------------------
    int detectSumMax;//检测帧累计最大值
    int detectSum;//检测帧累计数，减少偶然干扰，初始值为不为0，为了指数增长
    double proDetectArea;//上次检测到的轮廓总面积
    double currentDetectArea;//本次检测到的轮廓总面积
    int detecModel;//颜色检测模型选择
    int frameNumber;
    int changeThreshold;//vibe算法检测无火焰切换回帧差法的阈值
    int nofireFrameCount;
    vector<vector<Point> > procontours;//前次绘制的矩形，累积多帧没有检测到火焰后才取消绘制
    Mat frame;
    Mat frameDiff;//帧差法要处理的当前帧
    Mat proFrame;
    Mat frameVibe;//vibe法要处理的当前帧
    Mat frameVibeArea;//vibe法要处理的区域
    Mat proFrameVive;//帧差中提取出背景帧，作为ViBe初始帧-帧差中检测出运动物体的前一帧
    Mat foreFrame;
    Rect vibeRect;//感兴趣区域，标注VIBE算法处理区域矩形
    FireVibe vibe;
    bool isVibe;//是否采用Vibe法
    bool isVibeInit;//Vibes是否初始化


    bool triangleDetect2(Mat &frame, vector<Point> &area);//基于正序对的尖角检测
    vector<vector<Point> > fireColorMatch(Mat &frame, Mat &detecframe, Rect &rect,int model);
    //------------------------------------
    Mat ImgPreProcess(Mat rgbimg,int model);//根据选择模型对图像进行处理
    bool FiredectetionImg(Mat &detecimg, Mat &sourceimg, int model);//检测火焰轮廓并绘制
    bool FiredetecetionFrame(Mat &resultFrame, Mat &frame, int model);//监测火焰
    void initProcessor();

};

#endif // VIEDOPROCESSOR_H
