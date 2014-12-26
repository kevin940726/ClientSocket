#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "thread.h"
#include "recving.h"
#include "class.h"

#include <iostream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <QRegExp>
#include <QThread>
#include <QtDebug>
#include <QListWidgetItem>
#include <QMessageBox>

#define BLEN 1024

int sd, sds, sdc = 0;
std::string name;
u_short portno;
QStringList list;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getsdc(int sd){
    sdc = sd;
}

void MainWindow::settext(QString s){
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText("<<" + s);
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::resettext(QString s){
    ui->textBrowser->setText(s);
}

void init(){
    if ((sd = connectsock("localhost", 6900)) < 0){
        errwarning("Faild to connect to server.");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    std::string buf = ui->lineEdit_4->text().toUtf8().constData();
    buf = buf + "\n";
    if (send(sdc, buf.c_str(), strlen(buf.c_str()), 0) < 0) qDebug() << "send";
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText(QString::fromStdString(buf));
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->lineEdit_4->setText("");
}

void MainWindow::on_pushButton_2_clicked()
{
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);

    std::string req = ui->lineEdit_3->text().toStdString();
    if (::send(sd, req.c_str(), strlen(req.c_str()), 0) < 0)
        errexit("Failed to send to server.");
    if (recv(sd, bptr, buflen, 0) < 0)
        errexit("Failed to receive the response.");

    QRegExp reg("REGISTER#(.*)");
    int pos = reg.indexIn(QString::fromStdString(req));
    if (pos > -1) {
        ui->label->setText(QString(buf));
        name = reg.cap(1).toStdString();
        this->setWindowTitle(QString::fromStdString(name));
        qDebug() << QString::fromStdString(name);
        std::string log = "#(.*)#(.*)";
        log = name + log;
        QRegExp logre(QString::fromStdString(log));
        memset(buf, 0, BLEN);
        if (send(sd, "List", 4, 0) < 0)
            errexit("Failed to send the request");
        if (recv(sd, bptr, buflen, 0) < 0)
            errexit("Failed to receive the response.");
        pos = logre.indexIn(QString(buf));
        if (pos > -1){
            qDebug() << logre.cap(1) << "#" << logre.cap(2);
            portno = logre.cap(2).toUShort();
            sds = passivesock(logre.cap(2).toUShort()+1, SOMAXCONN);
            Thread *thread1 = new Thread(sds);
            thread1->start();
            connect(thread1, SIGNAL(appendtext(QString)), this, SLOT(settext(QString)));
            connect(thread1, SIGNAL(sdcSignal(int)), this, SLOT(getsdc(int)));
            connect(thread1, SIGNAL(resettext(QString)), this, SLOT(resettext(QString)));
        }
    }
    else{
        qDebug() << buf;
        std::stringstream ss;
        ss << name << "#" << portno;
        std::string logex = ss.str();
        if (req == logex || req == "List"){
            list = QString(buf).split("\n");
            ui->label->setText(list.first());
            list.removeFirst();
            list.removeLast();
            ui->listWidget->clear();
            ui->listWidget->addItems(list);
        }
        else    ui->label->setText(QString(buf));
    }
    ui->lineEdit_3->setText("");
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QRegExp member("(.*)#(.*)#(.*)");
    int pos = member.indexIn(item->text());
    if (pos > -1){
        qDebug() << member.cap(3).toUShort();
        sdc = connectsock("localhost", member.cap(3).toUShort()+1);
        if (sdc < 0) errexit("Failed to connect.");
        ui->textBrowser->setText("connected.\n");
        recving *recving1 = new recving(sdc);
        recving1->start();
        connect(recving1, SIGNAL(appendtext(QString)), this, SLOT(settext(QString)));
        connect(recving1, SIGNAL(resettext(QString)), this, SLOT(resettext(QString)));
    }
}

void MainWindow::on_lineEdit_4_returnPressed()
{
    on_pushButton_3_clicked();
}

void MainWindow::on_lineEdit_3_returnPressed()
{
    on_pushButton_2_clicked();
}
