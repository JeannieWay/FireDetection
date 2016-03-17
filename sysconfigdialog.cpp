#include "sysconfigdialog.h"
#include "ui_sysconfigdialog.h"
#include "sysconfiginfo.h"
#include <QMessageBox>
#include <QDebug>
SysConfigDialog::SysConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SysConfigDialog)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/images/images/setting.png"));
    initForm();
}

SysConfigDialog::~SysConfigDialog()
{
    delete ui;
}

void SysConfigDialog::on_button_OK_clicked()
{
    if(SysConfigInfo::smtpHost.isEmpty())
    {
        QMessageBox::warning(this,tr("错误"),tr("smtp服务器设置错误！"));
        return;
    }
    if(SysConfigInfo::userName.isEmpty())
    {
        QMessageBox::warning(this,tr("错误"),tr("用户名不能为空！"));
        return;
    }
    if(SysConfigInfo::password.isEmpty())
    {
        QMessageBox::warning(this,tr("错误"),tr("密码不能为空！"));
        return;
    }
    if(SysConfigInfo::recivers.isEmpty())
    {
        QMessageBox::warning(this,tr("错误"),tr("收件人不能为空！"));
        return;
    }
    if(SysConfigInfo::emalSubjec.isEmpty())
    {
        SysConfigInfo::emalSubjec = tr("检测到疑似火情");
    }
    SysConfigInfo::isAlarmOpen_1 = ui->pushButton_1->getCheck();
    SysConfigInfo::isAlarmOpen_2 = ui->pushButton_2->getCheck();
    SysConfigInfo::isAlarmOpen_3 = ui->pushButton_3->getCheck();
    SysConfigInfo::isAlarmOpen_4 = ui->pushButton_4->getCheck();
    SysConfigInfo::isAlarmOpen_5 = ui->pushButton_5->getCheck();
    SysConfigInfo::isAlarmOpen_6 = ui->pushButton_6->getCheck();
    SysConfigInfo::isAlarmOpen_7 = ui->pushButton_7->getCheck();
    SysConfigInfo::isAlarmOpen_8 = ui->pushButton_8->getCheck();
    SysConfigInfo::isAlarmOpen_9 = ui->pushButton_9->getCheck();
    SysConfigInfo::isAlarmOpen_10 = ui->pushButton_10->getCheck();
    SysConfigInfo::isAlarmOpen_11 = ui->pushButton_11->getCheck();
    SysConfigInfo::isAlarmOpen_12 = ui->pushButton_12->getCheck();
    SysConfigInfo::isAlarmOpen_13 = ui->pushButton_13->getCheck();
    SysConfigInfo::isAlarmOpen_14 = ui->pushButton_14->getCheck();
    SysConfigInfo::isAlarmOpen_15 = ui->pushButton_15->getCheck();
    SysConfigInfo::isAlarmOpen_16 = ui->pushButton_16->getCheck();
    this->close();
}

void SysConfigDialog::initForm()
{
    this->setWindowTitle(tr("系统设置"));
    ui->pushButton_1->setCheck(SysConfigInfo::isAlarmOpen_1);
    ui->pushButton_2->setCheck(SysConfigInfo::isAlarmOpen_2);
    ui->pushButton_3->setCheck(SysConfigInfo::isAlarmOpen_3);
    ui->pushButton_4->setCheck(SysConfigInfo::isAlarmOpen_4);
    ui->pushButton_5->setCheck(SysConfigInfo::isAlarmOpen_5);
    ui->pushButton_6->setCheck(SysConfigInfo::isAlarmOpen_6);
    ui->pushButton_7->setCheck(SysConfigInfo::isAlarmOpen_7);
    ui->pushButton_8->setCheck(SysConfigInfo::isAlarmOpen_8);
    ui->pushButton_9->setCheck(SysConfigInfo::isAlarmOpen_9);
    ui->pushButton_10->setCheck(SysConfigInfo::isAlarmOpen_10);
    ui->pushButton_11->setCheck(SysConfigInfo::isAlarmOpen_11);
    ui->pushButton_12->setCheck(SysConfigInfo::isAlarmOpen_12);
    ui->pushButton_13->setCheck(SysConfigInfo::isAlarmOpen_13);
    ui->pushButton_14->setCheck(SysConfigInfo::isAlarmOpen_14);
    ui->pushButton_15->setCheck(SysConfigInfo::isAlarmOpen_15);
    ui->pushButton_16->setCheck(SysConfigInfo::isAlarmOpen_16);
    ui->smtpName->setText(SysConfigInfo::smtpHost);
    ui->smtpPort->setValue(SysConfigInfo::smtpPort);
    ui->smtpSSL->setChecked(SysConfigInfo::isSSLChoice);
    ui->userName->setText(SysConfigInfo::userName);
    ui->password->setText(SysConfigInfo::password);
    ui->receivers->setText(SysConfigInfo::recivers);
    ui->subject->setText(SysConfigInfo::emalSubjec);
    connect(ui->button_cancel,SIGNAL(clicked()),this,SLOT(close()));

}
