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
using namespace cv;
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
    void nextFrameTimeOut();
    void setIsForeDetect(bool value);
    bool getDebugModel();
    void setDebugModel(bool value);
private:
    VideoCapture *video;//视频源
    int videoID;
    Mat frame;//帧数据
    BackgroundSubtractorMOG mog;//混合高斯对象
    int delayTime;//帧延时时间
    QTimer *timer;
    bool isNextFrame;//是否可以读取下一帧
    bool allowdAlarm;
    bool isOpen;//视频是否打开
    bool isForeDetect;//是否进行运动前景提取
    bool isDebug;//调试模式显示中间过程图
    int detectSumMax;//检测帧累计最大值
    int detectSum;//检测帧累计数，减少偶然干扰
    int oldDetectNum;//上次检测到的轮廓数
    int newDetectNum;//本次检测到的轮廓数
    int detecModel;//颜色检测模型选择
    Mat ImgPreProcess(Mat rgbimg,int model);//根据选择模型对图像进行处理
    bool FiredectetionImg(Mat &detecimg, Mat &sourceimg, int model);//检测火焰轮廓并绘制
    void initProcessor();

};

#endif // VIEDOPROCESSOR_H
