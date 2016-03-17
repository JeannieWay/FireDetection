#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QPushButton>

class SwitchButton : public QPushButton
{
    Q_OBJECT

public:
    explicit SwitchButton(QWidget * parent = 0);
    bool getCheck()const;
    void setCheck(bool isChecked);

public slots:
    void changeOnoff();

private:
    bool isChecked;
    QString styleOn;
    QString styleOff;
};

#endif // SWITCHBUTTON_H
