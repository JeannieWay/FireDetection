#include "switchbutton.h"

SwitchButton::SwitchButton(QWidget *parent):QPushButton(parent)
{
    setCursor(QCursor(Qt::PointingHandCursor));
    isChecked = false;
    //background-image
    styleOn = "border-image: url(:/images/images/switch_on.png); border: 0px;";
    styleOff = "border-image: url(:/images/images/switch_off.png); border: 0px;";
    setFixedSize(70,35);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet(styleOff);
    connect(this,SIGNAL(clicked()),this,SLOT(changeOnoff()));
}

bool SwitchButton::getCheck() const
{
    return isChecked;
}

void SwitchButton::setCheck(bool isChecked)
{
    if(this->isChecked != isChecked)
    {
        this->isChecked = !isChecked;
        changeOnoff();
    }
}

void SwitchButton::changeOnoff()
{
    if(isChecked)
    {
        setStyleSheet(styleOff);
        isChecked = false;
    }
    else
    {
        setStyleSheet(styleOn);
        isChecked = true;
    }
}

