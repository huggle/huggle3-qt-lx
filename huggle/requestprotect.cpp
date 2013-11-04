#include "requestprotect.h"
#include "ui_requestprotect.h"

RequestProtect::RequestProtect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestProtect)
{
    ui->setupUi(this);
}

RequestProtect::~RequestProtect()
{
    delete ui;
}
