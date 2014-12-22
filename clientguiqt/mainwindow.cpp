#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "thread.h"
#include "recving.h"

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

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#define BLEN 1200

u_short portbase = 0; // port base, for non-root servers
int sd, sds, sdc = 0;
std::string name;
u_short portno;
QStringList list;

pthread_t tid1; // the thread identifie

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

void errexit(std::string format){
    QMessageBox error;
    error.critical(0, "error", QString::fromStdString(format));
    exit(1);
}

int connectsock(const char *host, u_short service, const char *transport){
    struct hostent *phe;
    struct protoent *ppe;
    struct sockaddr_in sin; //an Internet endpoint address

    int sd; //socket descriptor
    memset(&sin, 0, sizeof(sin)); //clean up
    sin.sin_family = AF_INET; //address family: Internet
    sin.sin_port = htons(service);

    // Map host name to IP address, allowing for dotted decimal
    if ( (phe = gethostbyname(host)) )
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr=inet_addr(host)) == INADDR_NONE)
        errexit("can’t get host entry");

    // Map transport protocol name to protocol number
    if ( (ppe = getprotobyname(transport)) == 0)
        errexit("can’t get protocol entry");

    //allocate socket
    if ((sd = socket(AF_INET, SOCK_STREAM, ppe->p_proto)) < 0)
        errexit("Failed to allocate to server.");

    //connect the socket
    if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Faild to connect to server");

    return sd;
}

int passivesock(u_short service, int qlen){
    struct protoent *ppe; // pointer to protocol information entry
    struct sockaddr_in sin; //an Internet endpoint address
    int s; //socket descriptor
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(service);

    if ( (ppe = getprotobyname("TCP")) == 0)
        errexit("getprotobyname");

    if ((s = socket(AF_INET, SOCK_STREAM, ppe->p_proto)) < 0)
        errexit("socket");

    if (::bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("bind");

    if (listen(s, qlen) < 0)
        errexit("listen");

    return s;
}

void MainWindow::settext(QString s){
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText(">>" + s);
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::getsdc(int sd){
    sdc = sd;
}

void init(){
    if ((sd = connectsock("localhost", 6900, "TCP")) < 0){
        errexit("Faild to connect to server.");
    }
}

void MainWindow::on_pushButton_clicked()
{
    char req[BLEN];
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(req, 0, BLEN);
    memset(buf, 0, BLEN);

    /*const char* host = ui->lineEdit->text().toUtf8().constData();
    u_short port = ui->lineEdit_2->text().toUShort();
    sd = connectsock(host, port, "TCP");*/
    std::string name = ui->lineEdit_3->text().toStdString();
    std::string reqs = "REGISTER#" + name;
    if (send(sd, reqs.c_str(), strlen(reqs.c_str()), 0) < 0)
        errexit("Failed to send the request");
    if (recv(sd, bptr, buflen, 0) < 0)
        errexit("Failed to receive the response.");
    ui->label->setText(QString(buf));

    std::string log = "#(.*)#(.*)";
    log = name + log;
    QRegExp logre(QString::fromStdString(log));
    memset(buf, 0, BLEN);
    if (send(sd, "List", 4, 0) < 0)
        errexit("Failed to send the request");
    if (recv(sd, bptr, buflen, 0) < 0)
        errexit("Failed to receive the response.");
    ui->label->setText(QString(buf));
    int pos = logre.indexIn(QString(buf));
    if (pos > -1){
        qDebug() << logre.cap(1) << "#" << logre.cap(2);
        ui->label->setText(ui->label->text()+logre.cap(1)+"#"+logre.cap(2));
        sds = passivesock(logre.cap(2).toUShort(), SOMAXCONN);
    }
    list = QString(buf).split("\n");
    list.removeFirst();
    list.removeLast();
    qDebug() << list;
    //QStringListModel model;
    //model.setStringList(list);
    //ui->listView->setModel(&model);
    //ui->listView->show();

    Thread *thread1 = new Thread(sds);
    thread1->start();
    connect(thread1, SIGNAL(appendtext(QString)), this, SLOT(settext(QString)));
    connect(thread1, SIGNAL(sdcSignal(int)), this, SLOT(getsdc(int)));
}

void MainWindow::on_pushButton_3_clicked()
{
    std::string buf = ui->lineEdit_4->text().toUtf8().constData();
    buf = "<<" + buf + "\n";
    if (::send(sdc, buf.c_str(), strlen(buf.c_str()), 0) < 0) qDebug() << "send";
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
        //ui->label->setText(QString(buf));
        pos = logre.indexIn(QString(buf));
        if (pos > -1){
            qDebug() << logre.cap(1) << "#" << logre.cap(2);
            portno = logre.cap(2).toUShort();
            //ui->label->setText(ui->label->text()+logre.cap(1)+"#"+logre.cap(2));
            sds = passivesock(logre.cap(2).toUShort()+1, SOMAXCONN);
            Thread *thread1 = new Thread(sds);
            thread1->start();
            connect(thread1, SIGNAL(appendtext(QString)), this, SLOT(settext(QString)));
            connect(thread1, SIGNAL(sdcSignal(int)), this, SLOT(getsdc(int)));
        }
    }
    else{
        qDebug() << buf;
        std::stringstream ss;
        ss << name << "#" << portno;
        std::string logex = ss.str();
        //qDebug() << QString::fromStdString(logex);
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
        sdc = connectsock("localhost", member.cap(3).toUShort(), "TCP"); //member.cap(2).toUtf8().constData()
        if (sdc < 0) errexit("Failed to connect.");
        recving *recving1 = new recving(sdc);
        recving1->start();
        connect(recving1, SIGNAL(appendtext(QString)), this, SLOT(settext(QString)));
    }
}

void MainWindow::on_lineEdit_4_returnPressed()
{
    on_pushButton_3_clicked();
}
