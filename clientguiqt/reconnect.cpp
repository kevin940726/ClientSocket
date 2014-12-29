#include "reconnect.h"
#include "ui_reconnect.h"
#include <QtDebug>

Reconnect::Reconnect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Reconnect)
{
    ui->setupUi(this);
}

Reconnect::~Reconnect()
{
    delete ui;
}

void Reconnect::on_buttonBox_accepted()
{
    emit getPort(ui->lineEdit_2->text());
    emit reinit();
}
