#include "recving.h"
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "mainwindow.h"

#define BLEN    1200

void recving::run()
{
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);

    while (true){
        if (recv(sdc, bptr, buflen, 0) > 0) //qDebug() << "recv";
        //qDebug() << buf;
        emit appendtext(buf);
        memset(buf, 0, BLEN);
    }
}
