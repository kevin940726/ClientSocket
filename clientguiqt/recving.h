#ifndef RECVING_H
#define RECVING_H

#include <QThread>
#include <openssl/ssl.h>
#include <openssl/err.h>

class recving: public QThread {
    Q_OBJECT

public:
    recving(int sdc, SSL *ssls) {this->sdc = sdc; this->ssls = ssls;}

protected:
    void run();

signals:
    void appendtext(QString s);
    void resettext(QString s);

private:
    int sdc;
    SSL *ssls;
};

#endif // RECVING_H
