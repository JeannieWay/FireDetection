#ifndef EMAILSENDER_H
#define EMAILSENDER_H
#include <QString>
#include <QObject>
//class QString;

class SysConfigInfo :public QObject
{
public:
    SysConfigInfo();
    static QString smtpHost;
    static int smtpPort;
    static bool isSSLChoice;
    static QString userName;
    static QString password;
    static QString recivers;
    static QString emalSubjec;
    static int sendTime;
    static bool isAlarmOpen_1;
    static bool isAlarmOpen_2;
    static bool isAlarmOpen_3;
    static bool isAlarmOpen_4;
    static bool isAlarmOpen_5;
    static bool isAlarmOpen_6;
    static bool isAlarmOpen_7;
    static bool isAlarmOpen_8;
    static bool isAlarmOpen_9;
    static bool isAlarmOpen_10;
    static bool isAlarmOpen_11;
    static bool isAlarmOpen_12;
    static bool isAlarmOpen_13;
    static bool isAlarmOpen_14;
    static bool isAlarmOpen_15;
    static bool isAlarmOpen_16;
};

#endif // EMAILSENDER_H
