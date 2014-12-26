#ifndef RECVING_H
#define RECVING_H

#include <QThread>

class recving: public QThread {
    Q_OBJECT

public:
    recving(int sdc) {this->sdc = sdc;}

protected:
    void run();

signals:
    void appendtext(QString s);
    void resettext(QString s);

private:
    int sdc;
};

#endif // RECVING_H
