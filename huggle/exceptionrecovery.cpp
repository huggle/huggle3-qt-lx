#include "exceptionrecovery.h"
#include "ui_exceptionrecovery.h"

ExceptionRecovery::ExceptionRecovery(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExceptionRecovery)
{
    ui->setupUi(this);
}

ExceptionRecovery::~ExceptionRecovery()
{
    delete ui;
}
