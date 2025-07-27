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
#include <huggle_core/configuration.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/syslog.hpp>
#include <QDesktopServices>
#include <QLocale>

using namespace Huggle;

WelcomeInfo::WelcomeInfo(QWidget *parent) : QDialog(parent), ui(new Ui::WelcomeInfo)
{
    this->ui->setupUi(this);
    int localization_ix=0, preferred=0;

    // \todo This doesn't work yet - QLocale returns locale names that are completely different from key names in Huggle
    // If this is a first time Huggle was started we can detect language preferred by OS and if we support it, offer it to user
    QString default_language = QLocale().name();
    HUGGLE_DEBUG1("OS reported language: " + default_language);

    while (localization_ix<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        this->ui->cb_Language->addItem(Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageID);
        if (Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageName == Localizations::HuggleLocalizations->PreferredLanguage)
        {
            preferred = localization_ix;
        }
        localization_ix++;
    }
    QString title = "Huggle " + Configuration::HuggleConfiguration->HuggleVersion;
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
    while (localization_ix<Localizations::HuggleLocalizations->LocalizationData.count())
    {
        this->ui->cb_Language->addItem(Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageID);
        if (Localizations::HuggleLocalizations->LocalizationData.at(localization_ix)->LanguageName == Localizations::HuggleLocalizations->PreferredLanguage)
        {
            preferred = localization_ix;
        }
        localization_ix++;
    }
    if (hcfg->Verbosity > 0)
    {
        // add debug lang "qqx" last
        this->ui->cb_Language->addItem(Localizations::LANG_QQX);
        if(Localizations::HuggleLocalizations->PreferredLanguage == Localizations::LANG_QQX)
            preferred = localization_ix;
        localization_ix++;
    }

}

WelcomeInfo::~WelcomeInfo()
{
    delete this->ui;
}

void Huggle::WelcomeInfo::on_btnOK_clicked()
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

void Huggle::WelcomeInfo::on_lblWelcomeText_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void WelcomeInfo::DisableFirst()
{
    this->ui->label_3->setVisible(false);
    this->ui->label_4->setVisible(false);
    this->ui->label_5->setVisible(false);
    this->ui->cb_Language->setVisible(false);
}


void WelcomeInfo::on_cb_Language_currentIndexChanged(int index)
{
    if (this->loading)  return;
    // This is just a fallback in case there is some weird bug with combo box
    // it's not a default language, don't change it here
    QString arg1 = this->ui->cb_Language->itemText(index);
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

