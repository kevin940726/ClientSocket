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
#include <QDir>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BLEN 1024

int sd, sds, sdc;
SSL_CTX *ctx;
SSL *ssl;
SSL *ssls;
std::string name;
const char* host = "localhost";
u_short portno = 6900;
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

void MainWindow::getssl(SSL *ssl){
    ssls = ssl;
}

void MainWindow::settext(QString s){
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText("<<" + s);
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::resettext(QString s){
    ui->textBrowser->setText(s);
}

//Set up and connect to the server.
void init(){
    if ((sd = connectsock("localhost", portno)) < 0){
        errwarning("Faild to connect to server.");
    }
    else{
        SSL_library_init();
        ctx = InitCTX();
        ssl = SSL_new(ctx);      /* create new SSL connection state */
        SSL_set_fd(ssl, sd);    /* attach the socket descriptor */
        if (SSL_connect(ssl) == -1) errwarning("ssl error");
        QDir::setCurrent(QCoreApplication::applicationDirPath());
    }
}

//Send messages to the other client.
void MainWindow::on_pushButton_3_clicked()
{
    std::string buf = ui->lineEdit_4->text().toUtf8().constData();
    buf = buf + "\n";
    if (SSL_write(ssls, buf.c_str(), strlen(buf.c_str())) < 0) qDebug() << "send";
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->textBrowser->insertPlainText(QString::fromStdString(buf));
    ui->textBrowser->moveCursor(QTextCursor::End);
    ui->lineEdit_4->setText("");
}

//Communicate with the server.
void MainWindow::on_pushButton_2_clicked()
{
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);

    std::string req = ui->lineEdit_3->text().toStdString();
    if (SSL_write(ssl, req.c_str(), strlen(req.c_str())) < 0)
        errexit("Failed to send to server.");
    if (SSL_read(ssl, bptr, buflen) < 0)
        errexit("Failed to receive the response.");

    QRegExp reg("REGISTER#(.*)");
    int pos = reg.indexIn(QString::fromStdString(req));
    if (pos > -1) {
        ui->label->setText(QString(buf));
        name = reg.cap(1).toStdString();
        this->setWindowTitle(QString::fromStdString(name));
        qDebug() << QString::fromStdString(name);
        showcerts(ssl);

        std::string log = "#(.*)#(.*)";
        log = name + log;
        QRegExp logre(QString::fromStdString(log));
        memset(buf, 0, BLEN);
        if (SSL_write(ssl, "List", 4) < 0)
            errexit("Failed to send the request");
        if (SSL_read(ssl, bptr, buflen) < 0)
            errexit("Failed to receive the response.");
        pos = logre.indexIn(QString(buf));
        if (pos > -1){
            qDebug() << logre.cap(1) << "#" << logre.cap(2);
            portno = logre.cap(2).toUShort();
            sds = passivesock(logre.cap(2).toUShort()+1, SOMAXCONN);

            SSL_CTX *ctxs;
            SSL_library_init();
            ctxs = InitServerCTX();        /* initialize SSL */
            LoadCertificates(ctxs, "mycert.pem", "mykey.pem");

            Thread *thread1 = new Thread(sds, ctxs);
            thread1->start();
            connect(thread1, SIGNAL(appendtext(QString)), this, SLOT(settext(QString)));
            connect(thread1, SIGNAL(sdcSignal(int)), this, SLOT(getsdc(int)));
            connect(thread1, SIGNAL(sslSignal(SSL*)), this, SLOT(getssl(SSL*)));
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

//Connect to other client.
void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QRegExp member("(.*)#(.*)#(.*)");
    int pos = member.indexIn(item->text());
    if (pos > -1){
        qDebug() << member.cap(3).toUShort();
        int sdc = connectsock("localhost", member.cap(3).toUShort()+1);
        if (sdc < 0) errexit("Failed to connect.");
        ui->textBrowser->setText("connected.");
        SSL_library_init();
        SSL_CTX *ctxc;
        ctxc = InitCTX();
        ssls = SSL_new(ctxc);
        SSL_set_fd(ssls, sdc);
        if (SSL_connect(ssls) == -1) errexit("ssls");
        showcerts(ssls);

        recving *recving1 = new recving(sdc, ssls);
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

//Handle reconnection.
void MainWindow::on_actionReconnect_triggered()
{
    newwindow = new Reconnect();
    newwindow->show();
    connect(newwindow, SIGNAL(getPort(QString)), this, SLOT(setPort(QString)));
    connect(newwindow, SIGNAL(reinit(void)), this, SLOT(reinit(void)));
}

void MainWindow::setPort(QString s){
    portno = s.toUShort();
    qDebug() << portno;
}

void MainWindow::reinit(){
    init();
}

void MainWindow::showcerts(SSL *ssl){
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        ui->textBrowser->append("Server certificates:");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        ui->textBrowser->append("Subject: " + QString::fromUtf8(line));
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        ui->textBrowser->append("Issuer: " + QString::fromUtf8(line) + "\n");
        free(line);
        X509_free(cert);
    }
}
