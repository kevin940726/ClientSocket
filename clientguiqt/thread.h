#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <openssl/ssl.h>
#include <openssl/err.h>

class Thread: public QThread {
    Q_OBJECT

public:
    Thread(int sds, SSL_CTX* ctxs) {this->sds = sds; this->ctxs = ctxs;}
    int getsdc() {return this->sdc;}
    void setsds(int sds) {this->sds = sds;}

protected:
    void run();

signals:
    void appendtext(QString s);
    void resettext(QString s);
    void sdcSignal(int sd);
    void sslSignal(SSL *ssl);

private:
    int sds, sdc;
    SSL_CTX* ctxs;
};
#endif // THREAD_H
