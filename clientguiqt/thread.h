#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class Thread: public QThread {
    Q_OBJECT

public:
    Thread(int sds) {this->sds = sds;}
    int getsdc() {return this->sdc;}
    void setsds(int sds) {this->sds = sds;}

protected:
    void run();

signals:
    void appendtext(QString s);
    void resettext(QString s);
    void sdcSignal(int sd);

private:
    int sds, sdc;
};
#endif // THREAD_H
