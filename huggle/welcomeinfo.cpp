//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "welcomeinfo.hpp"
#include "ui_welcomeinfo.h"
#include "configuration.hpp"
#include "localization.hpp"
#include <QDesktopServices>

using namespace Huggle;

WelcomeInfo::WelcomeInfo(QWidget *parent) : QDialog(parent), ui(new Ui::WelcomeInfo)
{
    this->ui->setupUi(this);
    int localization_ix=0, preferred=0;
    while (localization_ix<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        this->ui->cb_Language->addItem(Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageID);
        if (Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageName == Localizations::HuggleLocalizations->PreferredLanguage)
        {
            preferred = localization_ix;
        }
        localization_ix++;
    }
    QString title = "Huggle 3 QT-LX";
    if (hcfg->Verbosity > 0)
    {
        // add debug lang "qqx" last
        this->ui->cb_Language->addItem(Localizations::LANG_QQX);
        if(Localizations::HuggleLocalizations->PreferredLanguage == Localizations::LANG_QQX)
            preferred = localization_ix;
        localization_ix++;
    }
    this->setWindowTitle(title);
    this->ui->cb_Language->setCurrentIndex(preferred);
    this->loading = false;
    this->Localize();
    // let's hide this language bar for now, this form is not yet supporting it

    // this can be removed once there is l10 for this
    this->ui->label_4->setVisible(false);
    this->ui->cb_Language->setVisible(false);
}

WelcomeInfo::~WelcomeInfo()
{
    delete this->ui;
}

void Huggle::WelcomeInfo::on_pushButton_clicked()
{
    hcfg->SystemConfig_FirstRun = false;
    hcfg->SystemConfig_ShowStartupInfo = this->ui->checkBox->isChecked();
    this->close();
}

void WelcomeInfo::Localize()
{
    this->ui->checkBox->setText(_l("welcome-show"));
    this->ui->label_4->setText(_l("login-language"));
}

void Huggle::WelcomeInfo::on_label_2_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void Huggle::WelcomeInfo::on_cb_Language_currentIndexChanged(const QString &arg1)
{
    if (this->loading)  return;
    QString lang = "en";
    int c = 0;
    while (c<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        if (Localizations::HuggleLocalizations->LocalizationData.at(c)->LanguageID == arg1)
        {
            lang = Localizations::HuggleLocalizations->LocalizationData.at(c)->LanguageName;
            break;
        }
        c++;
    }
    if (Localizations::LANG_QQX == arg1)
        lang = Localizations::LANG_QQX;
    Localizations::HuggleLocalizations->PreferredLanguage = lang;
    this->Localize();
}

void WelcomeInfo::DisableFirst()
{
    this->ui->label_3->setVisible(false);
    this->ui->label_4->setVisible(false);
    this->ui->label_5->setVisible(false);
    this->ui->cb_Language->setVisible(false);
}

