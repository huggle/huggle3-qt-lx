#ifndef EXCEPTIONRECOVERY_H
#define EXCEPTIONRECOVERY_H

#include <QDialog>

namespace Ui {
class ExceptionRecovery;
}

class ExceptionRecovery : public QDialog
{
    Q_OBJECT

public:
    explicit ExceptionRecovery(QWidget *parent = 0);
    ~ExceptionRecovery();

private:
    Ui::ExceptionRecovery *ui;
};

#endif // EXCEPTIONRECOVERY_H
