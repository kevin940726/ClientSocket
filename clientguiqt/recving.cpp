#include "recving.h"
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "mainwindow.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BLEN    1024

//Receive message thread.
void recving::run()
{
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);

    int length;
    while (true){
        if ((length = SSL_read(ssls, bptr, buflen)) > 0)
            emit appendtext(buf);
        else if (length == 0){
            emit appendtext("Disconnected.");
            break;
        }
        memset(buf, 0, BLEN);
    }
}
