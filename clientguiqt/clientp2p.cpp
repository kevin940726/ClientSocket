#include "clientp2p.h"
#include "ui_clientp2p.h"

clientp2p::clientp2p(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::clientp2p)
{
    ui->setupUi(this);
}

clientp2p::~clientp2p()
{
    delete ui;
}
