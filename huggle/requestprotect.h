#ifndef REQUESTPROTECT_H
#define REQUESTPROTECT_H

#include <QDialog>

namespace Ui {
class RequestProtect;
}

class RequestProtect : public QDialog
{
    Q_OBJECT
    
public:
    explicit RequestProtect(QWidget *parent = 0);
    ~RequestProtect();
    
private:
    Ui::RequestProtect *ui;
};

#endif // REQUESTPROTECT_H
