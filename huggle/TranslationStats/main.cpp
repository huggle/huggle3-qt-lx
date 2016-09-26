#include <QCoreApplication>
#include <iostream>
#include <math.h>
#include "localization.hpp"

using namespace Huggle;

QString Bar(double size, QString color)
{
    QString text = "<span style=\"color:" + color + "; background-color:" + color + ";\">";
    while (size > 0)
    {
        text += "_";
        size--;
    }
    return text + "</span>";
}

void HTML()
{
    std::cout << "<table>\n<tr><th>Language</th><th>Completed</th><th>Percent</th><th></th></tr>" << std::endl;
    Language *english = Localizations::HuggleLocalizations->LocalizationData.at(0);
    QList<QString> keys = english->Messages.keys();
    int language = 1;
    double max = (double)keys.count();
    while (language < Localizations::HuggleLocalizations->LocalizationData.count())
    {
        double Missing = 0;
        double Untranslated = 0;
        Language *l = Localizations::HuggleLocalizations->LocalizationData.at(language);
        int x = 0;
        while (x < keys.count())
        {
            if (!l->Messages.contains(keys.at(x)))
            {
                Missing++;
            } else if (english->Messages[keys.at(x)] == l->Messages[keys.at(x)])
            {
                Untranslated++;
            }
            x++;
        }
        double Completed = keys.count() - (Missing + Untranslated);
        double Percent = (Completed / max) * 100;
        double rp = round(Percent);
        std::cout << QString("<tr><td>" + l->LanguageName + "</td><td>" + QString::number(Completed)
                     + "/" + QString::number(keys.count())
                     + "</td><td>"  + QString::number(Percent) +  "%</td><td>"
                     + Bar(rp, "green") + Bar(100 - rp, "red")
                     + "</td></tr>").toStdString() << std::endl;
        language++;
    }
    std::cout << "</table>" << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Localizations::HuggleLocalizations = new Localizations();
    Localizations::HuggleLocalizations->LocalInit("en"); // English
    Localizations::HuggleLocalizations->LocalInit("ar"); // Arabic
    Localizations::HuggleLocalizations->LocalInit("bg"); // Bulgarian
    //Localizations::HuggleLocalizations->LocalInit("bn"); // Bengali
    Localizations::HuggleLocalizations->LocalInit("br"); // Brezhoneg
    Localizations::HuggleLocalizations->LocalInit("cz"); // Czech
    Localizations::HuggleLocalizations->LocalInit("de"); // Deutsch
    Localizations::HuggleLocalizations->LocalInit("es"); // Spanish
    Localizations::HuggleLocalizations->LocalInit("fa"); // Persian
    Localizations::HuggleLocalizations->LocalInit("fr"); // French
    Localizations::HuggleLocalizations->LocalInit("gu");
    Localizations::HuggleLocalizations->LocalInit("he"); // Hebrew
    Localizations::HuggleLocalizations->LocalInit("hi"); // Hindi
    Localizations::HuggleLocalizations->LocalInit("it"); // Italian
    Localizations::HuggleLocalizations->LocalInit("ja"); // Japanese
    Localizations::HuggleLocalizations->LocalInit("ka"); // ?
    Localizations::HuggleLocalizations->LocalInit("kk-cyrl");
    //Localizations::HuggleLocalizations->LocalInit("km"); // Khmer
    Localizations::HuggleLocalizations->LocalInit("kn"); // Kannada
    Localizations::HuggleLocalizations->LocalInit("ko"); // Korean
    Localizations::HuggleLocalizations->LocalInit("ksh");
    Localizations::HuggleLocalizations->LocalInit("lb"); // Lebanon
    Localizations::HuggleLocalizations->LocalInit("lt");
    Localizations::HuggleLocalizations->LocalInit("mk"); // Macedonian
    Localizations::HuggleLocalizations->LocalInit("ml"); // Malayalam
    Localizations::HuggleLocalizations->LocalInit("mr"); // Marathi
    Localizations::HuggleLocalizations->LocalInit("ms");
    Localizations::HuggleLocalizations->LocalInit("nl"); // Dutch
    Localizations::HuggleLocalizations->LocalInit("no"); // Norwegian
    Localizations::HuggleLocalizations->LocalInit("oc"); // Occitan
    Localizations::HuggleLocalizations->LocalInit("or"); // Oriya
    Localizations::HuggleLocalizations->LocalInit("pl");
    Localizations::HuggleLocalizations->LocalInit("pt"); // Portuguese
    Localizations::HuggleLocalizations->LocalInit("pt-BR"); // Portuguese (in Brazil)
    Localizations::HuggleLocalizations->LocalInit("ru"); // Russian
    Localizations::HuggleLocalizations->LocalInit("sa");
    Localizations::HuggleLocalizations->LocalInit("sv"); // Swedish
    Localizations::HuggleLocalizations->LocalInit("ta");
    Localizations::HuggleLocalizations->LocalInit("tr"); // Turkish
    Localizations::HuggleLocalizations->LocalInit("ur");
    Localizations::HuggleLocalizations->LocalInit("zh-hant"); // Chinese
    Localizations::HuggleLocalizations->LocalInit("zh"); // Chinese
    HTML();
    return 0;
}
