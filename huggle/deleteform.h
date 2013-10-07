#ifndef DELETEFORM_H
#define DELETEFORM_H

#include <QDialog>

namespace Ui {
class DeleteForm;
}

class DeleteForm : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteForm(QWidget *parent = 0);
    ~DeleteForm();

private:
    Ui::DeleteForm *ui;
};

#endif // DELETEFORM_H
