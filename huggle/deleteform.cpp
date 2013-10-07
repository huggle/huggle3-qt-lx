#include "deleteform.h"
#include "ui_deleteform.h"

DeleteForm::DeleteForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeleteForm)
{
    ui->setupUi(this);
}

DeleteForm::~DeleteForm()
{
    delete ui;
}
