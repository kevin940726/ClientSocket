#ifndef CLIENTP2P_H
#define CLIENTP2P_H

#include <QWidget>

namespace Ui {
class clientp2p;
}

class clientp2p : public QWidget
{
    Q_OBJECT

public:
    explicit clientp2p(QWidget *parent = 0);
    ~clientp2p();

private:
    Ui::clientp2p *ui;
};

#endif // CLIENTP2P_H
