#ifndef PROTECTPAGE_H
#define PROTECTPAGE_H

#include <QDialog>

namespace Ui {
class ProtectPage;
}

class ProtectPage : public QDialog
{
	Q_OBJECT

public:
	explicit ProtectPage(QWidget *parent = 0);
	~ProtectPage();

private:
	Ui::ProtectPage *ui;
};

#endif // PROTECTPAGE_H
