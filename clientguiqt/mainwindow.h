#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread.h>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

void init();

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Ui::MainWindow *ui;

private slots:
    void on_pushButton_clicked();
    void settext(QString s);
    void getsdc(int sd);
    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_lineEdit_4_returnPressed();

private:


};

#endif // MAINWINDOW_H
