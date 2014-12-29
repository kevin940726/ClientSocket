#ifndef RECONNECT_H
#define RECONNECT_H

#include <QDialog>

namespace Ui {
class Reconnect;
}

class Reconnect : public QDialog
{
    Q_OBJECT

public:
    explicit Reconnect(QWidget *parent = 0);
    ~Reconnect();

private slots:
    void on_buttonBox_accepted();


signals:
    void getPort(QString s);

    void reinit();

private:
    Ui::Reconnect *ui;
};

#endif // RECONNECT_H
