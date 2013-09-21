#ifndef SCOREWORDSDBFORM_H
#define SCOREWORDSDBFORM_H

#include <QDialog>

namespace Ui {
class ScoreWordsDbForm;
}

class ScoreWordsDbForm : public QDialog
{
    Q_OBJECT
    
public:
    explicit ScoreWordsDbForm(QWidget *parent = 0);
    ~ScoreWordsDbForm();
    
private:
    Ui::ScoreWordsDbForm *ui;
};

#endif // SCOREWORDSDBFORM_H
