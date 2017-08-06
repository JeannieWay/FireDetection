#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QDialog>
#include <QLabel>
#include <QMenu>
#include <QTimer>
#include <QModelIndex>
#include "videoprocessor.h"
namespace Ui {
class MainFrame;
}

class MainFrame : public QDialog
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent = 0);
    ~MainFrame();

protected:
    bool eventFilter(QObject *obj, QEvent *e);
    void keyPressEvent(QKeyEvent *event);

signals:
    void startProcess(int videoID);
private slots:
    void screenFull();//全屏
    void screenNormal();//普通

    void sendAlarmEmail(int videoID, QString fileName);//发送报警邮件

    void closeCurrentVideo();//关闭当前视频通道
    void closeAllVideo();//关闭所有视频通道

    void paintFrame();//定时绘制帧处理
    void paintVideoFrame(int index,QPixmap pixmap);//接收检测结果绘制带轮廓画面

    //--ViBe
    void paintFrameWithViBe();//定时用ViBe处理绘制帧

    void showVideo_1();//切换到1画面
    void showVideo_4();
    void showVideo_9();
    void showVideo_13();
    void showVideo_16();

    void choiceIPCSource();//选择摄像头
    void choiceVideoSource();//选择通道视频源
    void foreDetectSet();//前景检测设置
    void choiceDetectModel();//选择检测模型
    void setDebugModel();

    void setSampleLevel();//设置采样等级

    int getVideLabIndex(QLabel *lab);

    void windowMinisize();

    void on_treeWidget_doubleClicked(const QModelIndex &index);

private:
    Ui::MainFrame *ui;

    QMenu *rightMenu;//右键菜单
    QLabel *currentLab;//当前鼠标停留label
    bool isVideoMax;//通道是否处于最大化
    QList<QLabel *> videoLabList;//通道显示视频label列表
    QList<QLayout *> videoLayList;//通道显示视频label的layout
    QList<QTimer *> videoTimerList;//通道对应定时器，刷新画面
    QList<VideoProcessor *> videoProcessorList;//视频处理类
    QList<bool> videoChannelOpenList;//通道是否启用
    QList<QThread *> videoThreadList;

    QString videoType;//画面展示类型

    void initFrame(); //初始化窗体
    void initForm();//初始化表格标签
    void initVideo();

    void removeLayout();//移除布局
    void stopAllTimer();
    void changeVideo_1(int index);//改变1画面布局
    void changeVideo_4(int index);
    void changeVideo_9(int index);
    void changeVideo_16(int index);

    void changeVideoLayout();//改变通道布局

    void videoPlayBack();
    void releseVideo();
    void Mat2Qimg(Mat &matImg, QImage &img);
};

#endif // MAINFRAME_H
