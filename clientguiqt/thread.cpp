#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "thread.h"
#include <QtDebug>
#include "mainwindow.h"

#define BLEN    1024

void Thread::run() {
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);

    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);

    while (true){
        qDebug() << "Waiting...";
        if((sdc = accept(sds, (struct sockaddr *)&clientAddr, &addrlen)) > 0){
            qDebug() << "received";
            emit resettext("connected.\n");
            emit sdcSignal(sdc);
            int length;
            while (true){
                if ((length = recv(sdc, bptr, buflen, 0)) > 0)
                    emit appendtext(buf);
                else if (length == 0){
                    emit appendtext("Disconnected.");
                    break;
                }
                memset(buf, 0, BLEN);
            }

            return;
        }
    }

}
