#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void getsdc(int sd);

    void settext(QString s);

    void resettext(QString s);

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_lineEdit_4_returnPressed();

    void on_lineEdit_3_returnPressed();

private:


};

#endif // MAINWINDOW_H
