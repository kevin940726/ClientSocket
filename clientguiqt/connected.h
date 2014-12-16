#ifndef CONNECTED_H
#define CONNECTED_H

#include <QObject>

class connected : public QObject
{
    Q_OBJECT
public:
    explicit connected(QObject *parent = 0);

signals:
    void appendtext(QString s);
public slots:

};

#endif // CONNECT_H
