//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "localization.hpp"

using namespace Huggle;

Localizations *Localizations::HuggleLocalizations = NULL;

Localizations::Localizations()
{
    this->PreferredLanguage = "en";
}

Language *Localizations::MakeLanguage(QString text, QString name)
{
    Language *l = new Language(name);
    QStringList keys = text.split("\n");
    int p = 0;
    while (p < keys.count())
    {
        if (keys.at(p).startsWith("//") || keys.at(p).startsWith("<"))
        {
            // this is comment in language file
            p++;
            continue;
        }
        if (keys.at(p).contains(":"))
        {
            if (keys.at(p)[0] == '@')
            {
                // this language is using identical text purposefuly so we replace
                // text with at symbol which means "use english locs"
                l->Messages.insert(key, "@");
                p++;
                continue;
            }
            QString line = keys.at(p).trimmed();
            QString key = line.mid(0, line.indexOf(":"));
            QString lang = line.mid(line.indexOf(":") + 1).trimmed();
            if (!l->Messages.contains(key))
            {
                l->Messages.insert(key, lang);
            }
        }
        p++;
    }
    if (l->Messages.contains("name"))
    {
        l->LanguageID = l->Messages["name"];
    }
    return l;
}

void Localizations::LocalInit(QString name)
{
    QFile *f;
    if (Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
        // we don't want to load custom files in safe mode
        f = new QFile(":/huggle/text/Localization/" + name + ".txt");
    } else
    {
        if (QFile().exists(Configuration::GetLocalizationDataPath() + name + ".txt"))
        {
            // there is a custom localization file in directory
            f = new QFile(Configuration::GetLocalizationDataPath() + name + ".txt");
        } else
        {
            f = new QFile(":/huggle/text/Localization/" + name + ".txt");
        }
    }
    f->open(QIODevice::ReadOnly);
    this->LocalizationData.append(Localizations::MakeLanguage(QString(f->readAll()), name));
    f->close();
    delete f;
}

QString Localizations::Localize(QString key)
{
    QString id = key;
    if (id.endsWith("]]"))
    {
        id = key.mid(0, key.length() - 2);
    }
    if (id.startsWith("[["))
    {
        id = id.mid(2);
    }
    if (this->LocalizationData.count() > 0)
    {
        int c=0;
        while (c<this->LocalizationData.count())
        {
            if (this->LocalizationData.at(c)->LanguageName == this->PreferredLanguage)
            {
                Language *l = this->LocalizationData.at(c);
                if (l->Messages.contains(id))
                {
                    QString result = l->Messages[id];
                    if (result == "@")
                    {
                        // reference to english
                        break;
                    }
                    return result;
                }
                // performance tweak
                break;
            }
            c++;
        }
        if (this->LocalizationData.at(0)->Messages.contains(id))
        {
            return this->LocalizationData.at(0)->Messages[id];
        }
    }
    return key;
}

QString Localizations::Localize(QString key, QStringList parameters)
{
    QString id = key;
    if (id.endsWith("]]"))
    {
        id = key.mid(0, key.length() - 2);
    }
    if (id.startsWith("[["))
    {
        id = id.mid(2);
    }
    if (this->LocalizationData.count() > 0)
    {
        int c=0;
        while (c<this->LocalizationData.count())
        {
            if (this->LocalizationData.at(c)->LanguageName == this->PreferredLanguage)
            {
                Language *l = this->LocalizationData.at(c);
                if (l->Messages.contains(id))
                {
                    QString text = l->Messages[id];
                    int x = 0;
                    while (x<parameters.count())
                    {
                        text = text.replace("$" + QString::number(x + 1), parameters.at(x));
                        x++;
                    }
                    return text;
                }
                // performance tweak
                break;
            }
            c++;
        }
        if (this->LocalizationData.at(0)->Messages.contains(id))
        {
            QString text = this->LocalizationData.at(0)->Messages[id];
            int x = 0;
            while (x<parameters.count())
            {
                text = text.replace("$" + QString::number(x + 1), parameters.at(x));
                x++;
            }
            return text;
        }
    }
    return key;
}

QString Localizations::Localize(QString key, QString par1, QString par2)
{
    QStringList list;
    list << par1 << par2;
    return Localize(key, list);
}

QString Localizations::Localize(QString key, QString parameters)
{
    QStringList list;
    list << parameters;
    return Localize(key, list);
}

Language::Language(QString name)
{
    this->LanguageName = name;
    this->LanguageID = name;
}
