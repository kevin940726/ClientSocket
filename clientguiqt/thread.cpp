#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "thread.h"
#include <QtDebug>
#include "mainwindow.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BLEN    1024

//Waiting for client to connect and communicate with.
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
            SSL *ssls;
            ssls = SSL_new(ctxs);
            SSL_set_fd(ssls, sdc);

            emit resettext("connected.\n");
            emit sdcSignal(sdc);
            emit sslSignal(ssls);
            X509 *cert;
            char *line;
            cert = SSL_get_peer_certificate(ssls);
            if ( cert == NULL )
                emit appendtext("No certificates.\n");

            int length;
            if ( SSL_accept(ssls) == -1 ) qDebug() << "ssls";
            while (true){
                if ((length = SSL_read(ssls, bptr, buflen)) > 0)
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
