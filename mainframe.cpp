#include "mainframe.h"
#include "ui_mainframe.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QtDebug>
#include <QThread>
#include <QFileDialog>
#include <QInputDialog>
#include "sysconfigdialog.h"
#include "sysconfiginfo.h"
#include "./src/SmtpMime"

MainFrame::MainFrame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainFrame)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/images/images/title.png"));
    this->initFrame();
    initForm();
    initVideo();
    videoType = "16";
}


MainFrame::~MainFrame()
{
    delete ui;
}

bool MainFrame::eventFilter(QObject *obj, QEvent *e)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);

    if(e->type() == QEvent::MouseButtonDblClick && mouseEvent->buttons() == Qt::LeftButton)
    {
        QLabel *labelClicked = qobject_cast<QLabel*>(obj);
        int index = getVideLabIndex(labelClicked);
        if(!isVideoMax)//没有全屏
        {
            removeLayout();
            stopAllTimer();//关闭所有定时器
            isVideoMax = true;
            videoLayList[0]->addWidget(labelClicked);           
            if(videoChannelOpenList[index] == true)
                videoTimerList[index]->start();
            labelClicked->setVisible(true);
        }
        else
        {
            videoProcessorList[index]->setDebugModel(false);
            isVideoMax = false;
            changeVideoLayout();
        }
    }
    else
    {
        if(e->type() == QEvent::MouseButtonPress)
        {
            if(obj == ui->labelFull)
            {
                screenFull();
                return true;
            }
            if(obj == ui->labelNVR)
            {
                //TODO
                return true;
            }
            if(obj == ui->labelIPC)
            {
                //TODO
                return true;
            }
            if(obj == ui->labelVideoPayBack)
            {
                videoPlayBack();
                return true;
            }
            if(obj == ui->labelConfig)
            {

                SysConfigDialog *sysconfig = new SysConfigDialog(this);
                sysconfig->exec();
                return true;
            }
            if(obj == ui->labelExit)
            {
                releseVideo();
                exit(0);
            }
            if(obj == ui->labelMinisize)
            {
                windowMinisize();
                return true;
            }

            if(mouseEvent->buttons() == Qt::RightButton)//弹出邮件菜单
            {
                currentLab = qobject_cast<QLabel *>(obj);
                rightMenu->exec(QCursor::pos());
                return true;
            }
            else
            {
                 currentLab = qobject_cast<QLabel *>(obj);
                 int index = getVideLabIndex(currentLab);
                 QString foreStr,modelStr,samplevelStr;
                 samplevelStr = tr(" 帧采样累积：%1").arg(videoProcessorList[index]->getDetectSumMax());
                 if(videoProcessorList[index]->isForeDetectOpen())
                 {
                     foreStr = tr("动态前景检测开启");
                 }
                 else
                 {
                     foreStr = tr("动态前景检测关闭");
                 }
                 switch (videoProcessorList[index]->getDetectModel())
                 {
                 case 0:
                     modelStr = tr("检测模型：RGB模型");
                     break;
                 case 1:
                     modelStr = tr("检测模型：YCC模型");
                     break;
                 case 2:
                     modelStr = tr("检测模型：HSI模型");
                     break;
                 case 3:
                     modelStr = tr("检测模型：混合模型");
                     break;
                 case 4:
                     modelStr = tr("检测模型：三重模型");
                     break;
                 default:
                     break;
                 }
                 ui->labelTitle->setText(QString(tr("智能火情检测平台 当前选中[通道%1]")).arg(index+1)+
                                         " "+foreStr + " " + modelStr + samplevelStr);
                 return true;
            }

        }
    }
    if(e->type() == QEvent::KeyPress)//空格键暂停
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if(keyEvent->key() == Qt::Key_Space)
        {
            int index = getVideLabIndex(currentLab);
            if(videoTimerList[index]->isActive())
            {
                videoTimerList[index]->stop();
            }
            else
            {
                videoTimerList[index]->start();
            }
            return true;
        }
    }
    return QObject::eventFilter(obj,e);


}

void MainFrame::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_F1://空格键全屏
        screenFull();
        break;
    case Qt::Key_Escape://ESC退出全屏
        screenNormal();
        break;
    default:
        break;
    }

}

void MainFrame::screenFull()
{
    this->setGeometry(qApp->desktop()->geometry());
    this->layout()->setContentsMargins(0,0,0,0);
    ui->widget_main->layout()->setContentsMargins(0,0,0,0);
    ui->widget_title->setVisible(false);
    ui->treeWidget->setVisible(false);
}

void MainFrame::screenNormal()
{
    this->setGeometry(qApp->desktop()->availableGeometry());
    this->layout()->setContentsMargins(1,1,1,1);
    ui->widget_main->setContentsMargins(5,5,5,5);
    ui->widget_title->setVisible(true);
    ui->treeWidget->setVisible(true);
}

void MainFrame::sendAlarmEmail(int videoID,QString fileName)
{
    bool isAlarm = false;

    if(videoProcessorList[videoID]->getAllowdAlarm())
    {
        switch(videoID)
        {
        case 0:
            isAlarm = SysConfigInfo::isAlarmOpen_1;
            break;
        case 1:
            isAlarm = SysConfigInfo::isAlarmOpen_2;
            break;
        case 2:
            isAlarm = SysConfigInfo::isAlarmOpen_3;
            break;
        case 3:
            isAlarm = SysConfigInfo::isAlarmOpen_4;
            break;
        case 4:
            isAlarm = SysConfigInfo::isAlarmOpen_5;
            break;
        case 5:
            isAlarm = SysConfigInfo::isAlarmOpen_6;
            break;
        case 6:
            isAlarm = SysConfigInfo::isAlarmOpen_7;
            break;
        case 7:
            isAlarm = SysConfigInfo::isAlarmOpen_8;
            break;
        case 8:
            isAlarm = SysConfigInfo::isAlarmOpen_9;
            break;
        case 9:
            isAlarm = SysConfigInfo::isAlarmOpen_10;
            break;
        case 10:
            isAlarm = SysConfigInfo::isAlarmOpen_11;
            break;
        case 11:
            isAlarm = SysConfigInfo::isAlarmOpen_12;
            break;
        case 12:
            isAlarm = SysConfigInfo::isAlarmOpen_13;
            break;
        case 13:
            isAlarm = SysConfigInfo::isAlarmOpen_14;
            break;
        case 14:
            isAlarm = SysConfigInfo::isAlarmOpen_15;
            break;
        case 15:
            isAlarm = SysConfigInfo::isAlarmOpen_16;
            break;
        }
    }
    if(isAlarm)
    {
        //暂时只发送一次警报邮件
        videoProcessorList[videoID]->setAllowdAlarm(false);
        SmtpClient smtpclicent(SysConfigInfo::smtpHost,SysConfigInfo::smtpPort,
                               SysConfigInfo::isSSLChoice?SmtpClient::SslConnection : SmtpClient::TcpConnection);
        smtpclicent.setUser(SysConfigInfo::userName);
        smtpclicent.setPassword(SysConfigInfo::password);
        MimeMessage message;
        message.setSender(new EmailAddress("jxau764@163.com"));
        message.addRecipient(new EmailAddress("zhouzhongtaosoft@163.com"));
        message.setSubject(tr("检测到疑似火情"));


        MimeHtml html;
        html.setHtml(QString("<h1> Channel [%1] has Detected!<h1>"
                     "<h2> The detected iamge is below<h2>"
                     "<img src='cid:image1'/>").arg(videoID+1));
        MimeInlineFile image1(new QFile(fileName));
        image1.setContentId("image1");;
        image1.setContentType("image/jpg");

        message.addPart(&html);
        message.addPart(&image1);
        smtpclicent.connectToHost();
        if(!smtpclicent.login())
        {
            qDebug()<<tr("登录邮件服务器失败！");
        }
        if(!smtpclicent.sendMail(message))
        {
            qDebug()<<tr("发送邮件失败！");
        }
        smtpclicent.quit();
    }
}

void MainFrame::closeCurrentVideo()
{
    int index = getVideLabIndex(currentLab);
    if(videoTimerList[index] != NULL)
        videoTimerList[index]->stop();
    videoChannelOpenList[index] = false;
    videoLabList[index]->setText(tr("通道%1").arg(index+1));
}

void MainFrame::closeAllVideo()
{
    stopAllTimer();
    for(int i = 0;i<16;i++)
    {
        videoChannelOpenList[i] = false;
        videoLabList[i]->setText(tr("通道%1").arg(i+1));
    }
}

void MainFrame::paintFrame()
{
    QTimer *timer = (QTimer *)sender();
    int index;
    if(timer == videoTimerList[0])
    {
        index = 0;
    }
    else if (timer == videoTimerList[1])
    {
        index = 1;
    }
    else if (timer == videoTimerList[2])
    {
        index = 2;
    }
    else if (timer == videoTimerList[3])
    {
        index = 3;
    }
    else if (timer == videoTimerList[4])
    {
        index = 4;
    }
    else if (timer == videoTimerList[5])
    {
        index = 5;
    }
    else if (timer == videoTimerList[6])
    {
        index = 6;
    }
    else if (timer == videoTimerList[7])
    {
        index = 7;
    }
    else if (timer == videoTimerList[8])
    {
        index = 8;
    }
    else if (timer == videoTimerList[9])
    {
        index = 9;
    }
    else if (timer == videoTimerList[10])
    {
        index = 10;
    }
    else if (timer == videoTimerList[11])
    {
        index = 11;
    }
    else if (timer == videoTimerList[12])
    {
        index = 12;
    }
    else if (timer == videoTimerList[13])
    {
        index = 13;
    }
    else if (timer == videoTimerList[14])
    {
        index = 14;
    }
    else if (timer == videoTimerList[15])
    {
        index = 15;
    }
    else if (timer == videoTimerList[16])
    {
        index = 16;
    }
    else
    {
        return;
    }
    videoProcessorList[index]->processVideo(index);
}

void MainFrame::paintVideoFrame(int index, QPixmap pixmap)
{
    videoLabList[index]->setPixmap(pixmap);
}

void MainFrame::paintFrameWithViBe()
{
    QTimer *timer = (QTimer *)sender();
    int index;
    if(timer == videoTimerList[0])
    {
        index = 0;
    }
    else if (timer == videoTimerList[1])
    {
        index = 1;
    }
    else if (timer == videoTimerList[2])
    {
        index = 2;
    }
    else if (timer == videoTimerList[3])
    {
        index = 3;
    }
    else if (timer == videoTimerList[4])
    {
        index = 4;
    }
    else if (timer == videoTimerList[5])
    {
        index = 5;
    }
    else if (timer == videoTimerList[6])
    {
        index = 6;
    }
    else if (timer == videoTimerList[7])
    {
        index = 7;
    }
    else if (timer == videoTimerList[8])
    {
        index = 8;
    }
    else if (timer == videoTimerList[9])
    {
        index = 9;
    }
    else if (timer == videoTimerList[10])
    {
        index = 10;
    }
    else if (timer == videoTimerList[11])
    {
        index = 11;
    }
    else if (timer == videoTimerList[12])
    {
        index = 12;
    }
    else if (timer == videoTimerList[13])
    {
        index = 13;
    }
    else if (timer == videoTimerList[14])
    {
        index = 14;
    }
    else if (timer == videoTimerList[15])
    {
        index = 15;
    }
    else if (timer == videoTimerList[16])
    {
        index = 16;
    }
    else
    {
        return;
    }
    videoProcessorList[index]->processVideoWithViBe(index);
}

void MainFrame::showVideo_1()
{
    removeLayout();
    stopAllTimer();
    videoType = "1";
    isVideoMax = true;
    int index = 0;

    QAction *action = (QAction*)sender();
    QString name = action->text();
    qDebug()<<name;
    if(name == tr("通道1"))
        index = 0;
    else if(name == tr("通道2"))
        index = 1;
    else if(name == tr("通道3"))
        index = 2;
    else if(name == tr("通道4"))
        index = 3;
    else if(name == tr("通道5"))
        index = 4;
    else if(name == tr("通道6"))
        index = 5;
    else if(name == tr("通道7"))
        index = 6;
    else if(name == tr("通道8"))
        index = 7;
    else if(name == tr("通道9"))
        index = 8;
    else if(name == tr("通道10"))
        index = 9;
    else if(name == tr("通道11"))
        index = 10;
    else if(name == tr("通道12"))
        index = 11;
    else if(name == tr("通道13"))
        index = 12;
    else if(name == tr("通道14"))
        index = 13;
    else if(name == tr("通道15"))
        index = 14;
    else if(name == tr("通道16"))
        index = 15;
    changeVideo_1(index);
}

void MainFrame::showVideo_4()
{
    removeLayout();
    stopAllTimer();
    isVideoMax = false;
    int index = 0;

    QAction *action = (QAction*)sender();
    QString name = action->text();
    if(name == tr("通道1~4"))
    {
        index = 0;
        videoType = "1_4";
    }
    else if(name == tr("通道5~8"))
    {
        index = 4;
        videoType = "5_8";
    }
    else if (name == tr("通道9~12"))
    {
        index = 8;
        videoType = "9_12";
    }
    else if (name == tr("通道13~16"))
    {
        index = 12;
        videoType = "13_16";
    }
    changeVideo_4(index);
}

void MainFrame::showVideo_9()
{
    removeLayout();
    stopAllTimer();
    isVideoMax = false;
    int index = 0;

    QAction *action = (QAction*)sender();
    QString name = action->text();
    if(name == tr("通道1~9"))
    {
        index = 0;
        videoType = "1_9";
    }
    else if (name == tr("通道8~16"))
    {
        index = 7;
        videoType = "8_16";
    }
    changeVideo_9(index);
}

void MainFrame::showVideo_13()
{

}

void MainFrame::showVideo_16()
{
    removeLayout();
    stopAllTimer();
    videoType = "16";
    isVideoMax = false;
    int index = 0;
    changeVideo_16(index);
}

void MainFrame::choiceIPCSource()
{
    int index = getVideLabIndex(currentLab);
    videoProcessorList[index]->openVideo(0);//打开默认摄像头
    videoChannelOpenList[index] = true;
    videoTimerList[index]->start(20);
    //connect(videoTimerList[index],SIGNAL(timeout()),this,SLOT(paintFrame()));
    connect(videoTimerList[index],SIGNAL(timeout()),this,SLOT(paintFrame()));
}

void MainFrame::choiceVideoSource()
{
    int index = getVideLabIndex(currentLab);
    QString fileName = QFileDialog::getOpenFileName(this,tr("选择视频文件"),".",tr("video files(*.avi *.mp4 *.rmvb)"));
    if(fileName.isEmpty())//没有选择文件
    {
        return;
    }
    videoProcessorList[index]->openVideo(fileName);
    videoChannelOpenList[index] = true;
    videoTimerList[index]->start(videoProcessorList[index]->getFrameTime());
  //  connect(videoTimerList[index],SIGNAL(timeout()),this,SLOT(paintFrame()));
    connect(videoTimerList[index],SIGNAL(timeout()),this,SLOT(paintFrameWithViBe()));
}

void MainFrame::foreDetectSet()
{
    QAction *action = (QAction*)sender();
    QString txt = action->text();
    int index = getVideLabIndex(currentLab);
    if(txt == tr("开启动态前景检测"))
    {
       videoProcessorList[index]->setIsForeDetect(true);
    }
    else
    {
        videoProcessorList[index]->setIsForeDetect(false);
    }
}

void MainFrame::choiceDetectModel()
{
    QAction *action = (QAction*)sender();
    QString txt = action->text();
    int index = getVideLabIndex(currentLab);
    int model = 4;//默认三重模型
    if(txt == tr("RGB模型"))
    {
        model = 0;
    }
    else if (txt == tr("YCC模型"))
    {
        model = 1;
    }
    else if (txt == tr("HSI模型"))
    {
        model = 2;
    }
    else if (txt == tr("混合模型"))
    {
        model = 3;
    }
    else if (txt == tr("三重模型"))
    {
        model = 4;
    }
    videoProcessorList[index]->setDetectModel(model);
}

void MainFrame::setDebugModel()
{
    int index = getVideLabIndex(currentLab);
    QAction *action = (QAction*)sender();
    QString txt = action->text();
    if(txt == tr("开启调试模式"))
    {
        videoProcessorList[index]->setDebugModel(true);
    }
    else
    {
        videoProcessorList[index]->setDebugModel(false);
    }
}

void MainFrame::setSampleLevel()
{
    bool ok;
    int sum,index;
    sum = QInputDialog::getInt(this,tr("帧采样等级"),tr("请输入帧采样累积数"),20,0,500,1,&ok);
    if(ok)
    {
        index = getVideLabIndex(currentLab);
        videoProcessorList[index]->setDetectSumMax(sum);
    }
}

int MainFrame::getVideLabIndex(QLabel *lab)
{
    if(lab == ui->label_1)
        return 0;
    if(lab == ui->label_2)
        return 1;
    if(lab == ui->label_3)
        return 2;
    if(lab == ui->label_4)
        return 3;
    if(lab == ui->label_5)
        return 4;
    if(lab == ui->label_6)
        return 5;
    if(lab == ui->label_7)
        return 6;
    if(lab == ui->label_8)
        return 7;
    if(lab == ui->label_9)
        return 8;
    if(lab == ui->label_10)
        return 9;
    if(lab == ui->label_11)
        return 10;
    if(lab == ui->label_12)
        return 11;
    if(lab == ui->label_13)
        return 12;
    if(lab == ui->label_14)
        return 13;
    if(lab == ui->label_15)
        return 14;
    if(lab == ui->label_16)
        return 15;
    return 0;
}

void MainFrame::initFrame()
{
    this->setStyleSheet("QGroupBox#gboxMain{border-width:0px;}");
    this->setProperty("Form",true);
    //设置窗体标题栏隐藏
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinMaxButtonsHint);

}

void MainFrame::initForm()
{
    ui->labelFull->installEventFilter(this);//添加事件过滤器
    ui->labelFull->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelNVR->installEventFilter(this);
    ui->labelNVR->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelIPC->installEventFilter(this);
    ui->labelIPC->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelVideoPayBack->installEventFilter(this);
    ui->labelVideoPayBack->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelConfig->installEventFilter(this);
    ui->labelConfig->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelExit->installEventFilter(this);
    ui->labelExit->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelMinisize->installEventFilter(this);
    ui->labelMinisize->setCursor(QCursor(Qt::PointingHandCursor));
    ui->labelTitle->setText(tr("智能火情监测平台"));
 //   ui->treeWidget->header()->setVisible(false);
    ui->treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->treeWidget->setFocusPolicy(Qt::NoFocus);
    ui->treeWidget->expandAll();//展开所有节点

}

void MainFrame::initVideo()
{
    currentLab = NULL;
    isVideoMax = false;

    videoLabList.append(ui->label_1);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_2);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_3);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_4);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_5);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_6);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_7);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_8);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_9);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_10);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_11);    
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_12);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_13);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_14);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_15);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));

    videoLabList.append(ui->label_16);
    videoTimerList.append(new QTimer(this));
    videoProcessorList.append(new VideoProcessor());
    videoThreadList.append(new QThread(this));
    for(int i=0;i<16;i++)
    {
        videoLabList[i]->setScaledContents(true);
        videoProcessorList[i]->moveToThread(videoThreadList[i]);
        videoChannelOpenList.append(false);
        connect(videoThreadList[i],SIGNAL(finished()),videoProcessorList[i],SLOT(deleteLater()));
        videoThreadList[i]->start();
        connect(videoProcessorList[i],SIGNAL(processEnd(int,QPixmap)),this,SLOT(paintVideoFrame(int,QPixmap)));
        connect(videoProcessorList[i],SIGNAL(sendAlarm(int,QString)),this,SLOT(sendAlarmEmail(int,QString)));
    }

    videoLayList.append(ui->horizontalLayout_1);
    videoLayList.append(ui->horizontalLayout_2);
    videoLayList.append(ui->horizontalLayout_3);
    videoLayList.append(ui->horizontalLayout_4);

    for(int i=0;i<16;i++)
    {
        videoLabList[i]->installEventFilter(this);
        videoLabList[i]->setText(QString(tr("通道%1")).arg(i+1));
    }

    rightMenu = new QMenu(this);
    rightMenu->setStyleSheet("font: 12pt \"微软雅黑\";");

    QMenu *menuCurrent = rightMenu->addMenu(QString(tr("当前通道设置")));
    menuCurrent->addAction(tr("IPC选择"),this,SLOT(choiceIPCSource()));
    menuCurrent->addAction(tr("视频回放"),this,SLOT(choiceVideoSource()));

    QMenu *foreDetecMenu = menuCurrent->addMenu(tr("前景检测"));
    foreDetecMenu->addAction(tr("开启动态前景检测"),this,SLOT(foreDetectSet()));
    foreDetecMenu->addAction(tr("关闭动态前景检测"),this,SLOT(foreDetectSet()));

    QMenu *detecMenu = menuCurrent->addMenu(tr("监测模型"));
    detecMenu->addAction(tr("RGB模型"),this,SLOT(choiceDetectModel()));
    detecMenu->addAction(tr("YCC模型"),this,SLOT(choiceDetectModel()));
    detecMenu->addAction(tr("HSI模型"),this,SLOT(choiceDetectModel()));
    detecMenu->addAction(tr("混合模型"),this,SLOT(choiceDetectModel()));
    detecMenu->addAction(tr("三重模型"),this,SLOT(choiceDetectModel()));
    QMenu *debugMenu = menuCurrent->addMenu(tr("调试模式"));
    debugMenu->addAction(tr("开启调试模式"),this,SLOT(setDebugModel()));
    debugMenu->addAction(tr("关闭调试模式"),this,SLOT(setDebugModel())); 
    menuCurrent->addAction(tr("采样等级"),this,SLOT(setSampleLevel()));

    rightMenu->addAction(QString(tr("关闭当前通道")),this,SLOT(closeCurrentVideo()));
    rightMenu->addAction(tr("关闭所有通道"),this,SLOT(closeAllVideo()));
    rightMenu->addSeparator();

    QMenu *menu1 = rightMenu->addMenu(tr("切换到1画面"));
    menu1->addAction(tr("通道1"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道2"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道3"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道4"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道5"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道6"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道7"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道8"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道9"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道10"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道11"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道12"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道13"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道14"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道15"),this,SLOT(showVideo_1()));
    menu1->addAction(tr("通道16"),this,SLOT(showVideo_1()));

    QMenu *menu4 = rightMenu->addMenu(tr("切换到4画面"));
    menu4->addAction(tr("通道1~4"),this,SLOT(showVideo_4()));
    menu4->addAction(tr("通道5~8"),this,SLOT(showVideo_4()));
    menu4->addAction(tr("通道9~12"),this,SLOT(showVideo_4()));
    menu4->addAction(tr("通道13~16"),this,SLOT(showVideo_4()));

    QMenu *menu9 = rightMenu->addMenu(tr("切换到9画面"));
    menu9->addAction(tr("通道1~9"),this,SLOT(showVideo_9()));
    menu9->addAction(tr("通道8~16"),this,SLOT(showVideo_9()));

    rightMenu->addAction(tr("切换到16画面"),this,SLOT(showVideo_16()));





}

void MainFrame::removeLayout()
{
    //将layout上的组件去掉
    for(int i=0;i<4;i++)
    {
        videoLayList[0]->removeWidget(videoLabList[i]);
        videoLabList[i]->setVisible(false);
    }
    for(int i=4;i<8;i++)
    {
        videoLayList[1]->removeWidget(videoLabList[i]);
        videoLabList[i]->setVisible(false);
    }
    for(int i=8;i<12;i++)
    {
        videoLayList[2]->removeWidget(videoLabList[i]);
        videoLabList[i]->setVisible(false);
    }
    for(int i=12;i<16;i++)
    {
        videoLayList[3]->removeWidget(videoLabList[i]);
        videoLabList[i]->setVisible(false);
    }
}

void MainFrame::stopAllTimer()
{
    for(int i=0;i<16;i++)//关闭所有通道的定时器
    {
        if(videoThreadList[i] != NULL)
        {
            videoTimerList[i]->stop();
        }
    }
}

void MainFrame::changeVideo_1(int index)
{
    videoLayList[0]->addWidget(videoLabList[index]);
    videoLabList[index]->setVisible(true);
   //重启对应通道的定时器
    if(videoChannelOpenList[index] == true)
        videoTimerList[index]->start();
}

void MainFrame::changeVideo_4(int index)
{
    //每个layout放置两个label 2*2
    for(int i = index; i<index+2;i++)
    {
        videoLayList[0]->addWidget(videoLabList[i]);
        videoLabList[i]->setVisible(true);
        if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
    for(int i = index+2;i<index+4;i++)
    {
       videoLayList[1]->addWidget(videoLabList[i]);
       videoLabList[i]->setVisible(true);
       if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
}

void MainFrame::changeVideo_9(int index)
{
    //3*3
    for(int i = index;i<index+3;i++)
    {
        videoLayList[0]->addWidget(videoLabList[i]);
        videoLabList[i]->setVisible(true);
        if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
    for(int i = index+3;i<index+6;i++)
    {
       videoLayList[1]->addWidget(videoLabList[i]);
       videoLabList[i]->setVisible(true);
       if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
    for(int i = index+6;i<index+9;i++)
    {
       videoLayList[2]->addWidget(videoLabList[i]);
       videoLabList[i]->setVisible(true);
       if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }

}

void MainFrame::changeVideo_16(int index)
{
    for(int i = index;i<index+4;i++)
    {
        videoLayList[0]->addWidget(videoLabList[i]);
        videoLabList[i]->setVisible(true);
        if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
    for(int i = index+4;i<index+8;i++)
    {
       videoLayList[1]->addWidget(videoLabList[i]);
       videoLabList[i]->setVisible(true);
       if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
    for(int i = index+8;i<index+12;i++)
    {
       videoLayList[2]->addWidget(videoLabList[i]);
       videoLabList[i]->setVisible(true);
       if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
    for(int i = index+12;i<index+16;i++)
    {
       videoLayList[3]->addWidget(videoLabList[i]);
       videoLabList[i]->setVisible(true);
       if(videoChannelOpenList[i] == true)
            videoTimerList[i]->start();
    }
}

void MainFrame::changeVideoLayout()
{
    if(videoType == "1_4")
    {
        removeLayout();
        changeVideo_4(0);
    }
    else if (videoType == "5_8")
    {
        removeLayout();
        changeVideo_4(4);
    }
    else if(videoType == "9_12")
    {
        removeLayout();
        changeVideo_4(8);
    }
    else if (videoType == "13_16")
    {
        removeLayout();
        changeVideo_4(12);
    }
    else if (videoType == "1_9")
    {
        removeLayout();
        changeVideo_9(0);
    }
    else if (videoType == "8_16")
    {
        removeLayout();
        changeVideo_9(7);
    }
    else if (videoType == "16")
    {
        removeLayout();
        changeVideo_16(0);
    }
}

void MainFrame::videoPlayBack()
{
    videoProcessorList[0]->openVideo(0);
    videoTimerList[0]->start(20);
    connect(videoTimerList[0],SIGNAL(timeout()),this,SLOT(paintFrame()));

    videoProcessorList[1]->openVideo("./firevideo/fireVideo_1.mp4");

    videoProcessorList[2]->openVideo("./firevideo/fireVideo_2.mp4");

    videoProcessorList[3]->openVideo("./firevideo/fireVideo_3.avi");

    videoProcessorList[4]->openVideo("./firevideo/fireVideo_4.avi");

    videoProcessorList[5]->openVideo("./firevideo/fireVideo_5.avi");

    videoProcessorList[6]->openVideo("./firevideo/fireVideo_6.avi");
    videoProcessorList[6]->setIsForeDetect(false);

    videoProcessorList[7]->openVideo("./firevideo/fireVideo_7.avi");


    videoProcessorList[8]->openVideo("./firevideo/fireVideo_8.avi");
    videoProcessorList[8]->setIsForeDetect(false);

    videoProcessorList[9]->openVideo("./firevideo/fireVideo_9.avi");
    videoProcessorList[9]->setIsForeDetect(false);

    videoProcessorList[10]->openVideo("./firevideo/fireVideo_10.avi");
    videoProcessorList[10]->setIsForeDetect(false);

    videoProcessorList[11]->openVideo("./firevideo/fireVideo_11.avi");

    videoProcessorList[12]->openVideo("./firevideo/fireVideo_12.avi");

    videoProcessorList[13]->openVideo("./firevideo/fireVideo_13.avi");

    videoProcessorList[14]->openVideo("./firevideo/fireVideo_14.avi");

    videoProcessorList[15]->openVideo("./firevideo/fireVideo_15.avi");
    videoProcessorList[15]->setIsForeDetect(false);

    for(int i = 1;i<16;i++)
    {
        if(!videoProcessorList[i]->isOpenSucceed())//打开没有成功
        {
            continue;
        }
        videoChannelOpenList[i] = true;
        videoTimerList[i]->start(videoProcessorList[i]->getFrameTime());
        connect(videoTimerList[i],SIGNAL(timeout()),this,SLOT(paintFrame()));
    }

}

void MainFrame::releseVideo()
{
    for(int i=0;i<16;i++)
    {

        if(videoProcessorList[i]->isOpenSucceed())
        {
            videoProcessorList[i]->videoRelease();
        }
    }
}

void MainFrame::Mat2Qimg(Mat &matImg, QImage &img)
{
    Mat rgbframe;
    if(matImg.channels() == 3 )//3通道
    {
        cvtColor(matImg,rgbframe,CV_BGR2RGB);
        img = QImage((const uchar*)(rgbframe.data),rgbframe.cols,rgbframe.rows,
                     rgbframe.channels()*rgbframe.cols,QImage::Format_RGB888);
    }
    else
    {
        img = QImage((const uchar*)(matImg.data),matImg.cols,matImg.rows,
                     matImg.channels()*matImg.cols,QImage::Format_Indexed8);
    }
}

void MainFrame::windowMinisize()
{
    this->showMinimized();
}

void MainFrame::on_treeWidget_doubleClicked(const QModelIndex &index)
{
    QMessageBox::information(this,tr("设备信息"),tr("当前设备离线！"));
}
