#ifndef SYSCONFIGDIALOG_H
#define SYSCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class SysConfigDialog;
}

class SysConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SysConfigDialog(QWidget *parent = 0);
    ~SysConfigDialog();

private slots:
    void on_button_OK_clicked();

private:
    Ui::SysConfigDialog *ui;
    void initForm();
};

#endif // SYSCONFIGDIALOG_H
