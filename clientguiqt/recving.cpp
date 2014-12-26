#include "recving.h"
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "mainwindow.h"

#define BLEN    1024

void recving::run()
{
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);

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
}
