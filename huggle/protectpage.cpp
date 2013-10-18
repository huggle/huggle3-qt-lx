#include "protectpage.h"
#include "ui_protectpage.h"

ProtectPage::ProtectPage(QWidget *parent) : QDialog(parent), ui(new Ui::ProtectPage)
{
	ui->setupUi(this);
}

ProtectPage::~ProtectPage()
{
	delete ui;
}
