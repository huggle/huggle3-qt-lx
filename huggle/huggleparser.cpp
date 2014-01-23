//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggleparser.hpp"

using namespace Huggle;

QString HuggleParser::GetSummaryOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.count())
    {
        QString line = Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.at(id);
        if (line.startsWith(key + ";"))
        {
            return HuggleParser::GetValueFromKey(line);
        }
        id++;
    }
    return Configuration::HuggleConfiguration->LocalConfig_DefaultSummary;
}

QString HuggleParser::GetNameOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count())
    {
        QString line = Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(id);
        if (line.startsWith(key) + ";")
        {
            return HuggleParser::GetValueFromKey(line);
        }
        id++;
    }
    return key;
}

QString HuggleParser::GetKeyOfWarningTypeFromWarningName(QString id)
{
    int i=0;
    while (i<Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count())
    {
        QString line = Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(i);
        if (line.endsWith(id) || line.endsWith(id + ","))
        {
            return HuggleParser::GetKeyFromValue(line);
        }
        i++;
    }
    return id;
}

void HuggleParser::ParsePats(QString text)
{
    Configuration::HuggleConfiguration->LocalConfig_ScoreParts.clear();
    while (text.contains("score-parts("))
    {
        text = text.mid(text.indexOf("score-parts(") + 12);
        if (!text.contains(")"))
        {
            return;
        }
        int score = text.mid(0, text.indexOf(")")).toInt();

        if (score == 0)
        {
            continue;
        }

        QStringList word;

        if (!text.contains(":"))
        {
            return;
        }

        text = text.mid(text.indexOf(":") + 1);

        QStringList lines = text.split("\n");

        int line = 1;
        while (line < lines.count())
        {
            QString l = lines.at(line);
            QStringList items = l.split(",");
            int CurrentItem = 0;
            while ( CurrentItem < items.count() )
            {
                QString w = HuggleParser::Trim(items.at(CurrentItem));
                if (w == "")
                {
                    CurrentItem++;
                    continue;
                }
                word.append(w);
                CurrentItem++;
            }
            if (!l.endsWith(",") || HuggleParser::Trim(l) == "")
            {
                break;
            }
            line++;
        }

        line = 0;
        while (line < word.count())
        {
            Configuration::HuggleConfiguration->LocalConfig_ScoreParts.append(ScoreWord(word.at(line), score));
            line++;
        }
    }
}

void HuggleParser::ParseWords(QString text)
{
    Configuration::HuggleConfiguration->LocalConfig_ScoreWords.clear();
    while (text.contains("score-words("))
    {
        text = text.mid(text.indexOf("score-words(") + 12);
        if (!text.contains(")"))
        {
            return;
        }
        int score = text.mid(0, text.indexOf(")")).toInt();

        if (score == 0)
        {
            continue;
        }

        QStringList word;

        if (!text.contains(":"))
        {
            return;
        }

        text = text.mid(text.indexOf(":") + 1);

        QStringList lines = text.split("\n");

        int line = 1;
        while (line < lines.count())
        {
            QString l = lines.at(line);
            QStringList items = l.split(",");
            int CurrentItem = 0;
            while ( CurrentItem < items.count() )
            {
                QString w = HuggleParser::Trim(items.at(CurrentItem));
                if (w == "")
                {
                    CurrentItem++;
                    continue;
                }
                word.append(w);
                CurrentItem++;
            }
            if (!l.endsWith(",") || HuggleParser::Trim(l) == "")
            {
                break;
            }
            line++;
        }

        line = 0;
        while (line < word.count())
        {
            Configuration::HuggleConfiguration->LocalConfig_ScoreWords.append(ScoreWord(word.at(line), score));
            line++;
        }
    }
}

QString HuggleParser::GetValueFromKey(QString item)
{
    if (item.contains(";"))
    {
        QString type = item.mid(item.indexOf(";") + 1);
        if (type.endsWith(","))
        {
            type = type.mid(0, type.length() - 1);
        }
        return type;
    }
    return item;
}

QString HuggleParser::GetKeyFromValue(QString item)
{
    if (item.contains(";"))
    {
        QString type = item.mid(0, item.indexOf(";"));
        return type;
    }
    return item;
}

QString HuggleParser::Trim(QString text)
{
    while (text.startsWith(" "))
    {
        if (text == "")
        {
            break;
        }
        text = text.mid(1);
    }

    while (text.endsWith(" "))
    {
        text = text.mid(0, text.length() - 1);
    }

    return text;
}

int HuggleParser::GetLevel(QString page)
{
    if (Configuration::HuggleConfiguration->TrimOldWarnings)
    {
        // we need to get rid of old warnings now
        QString orig = page;
        // first we split the page by sections
        QStringList sections;
        int CurrentIndex = 0;
        while (CurrentIndex < page.length())
        {
            if (!page.startsWith("==") && !page.contains("\n=="))
            {
                // no sections
                sections.append(page);
                break;
            }

            // we need to get to start of section now
            CurrentIndex = 0;
            if (!page.startsWith("==") && page.contains("\n=="))
            {
                page = page.mid(page.indexOf("\n==") + 1);
            }

            // get to bottom of it
            int bottom = 0;
            if (!page.mid(CurrentIndex).contains("\n=="))
            {
                sections.append(page);
                break;
            }
            bottom = page.indexOf("\n==", CurrentIndex);

            QString section = page.mid(0, bottom);
            page = page.mid(bottom);
            sections.append(section);
        }

        // now we browse all sections and remove these with no current date

        CurrentIndex = 0;

        page = orig;

        while (CurrentIndex < sections.count())
        {
            // we need to find a date in this section
            if (!sections.at(CurrentIndex).contains("(UTC)"))
            {
                // there is none
                page = page.replace(sections.at(CurrentIndex), "");
                CurrentIndex++;
                continue;
            }
            QString section = sections.at(CurrentIndex);
            section = section.mid(0, section.indexOf("(UTC)"));
            if (section.endsWith(" "))
            {
                // we remove trailing white space
                section = section.mid(0, section.length() - 1);
            }

            if (!section.contains(","))
            {
                // this is some borked date let's remove it
                page = page.replace(sections.at(CurrentIndex), "");
                CurrentIndex++;
                continue;
            }

            QString time = section.mid(section.lastIndexOf(","));
            if (time.length() < 2)
            {
                // what the fuck
                page = page.replace(sections.at(CurrentIndex), "");
                CurrentIndex++;
                continue;
            }

            // we remove the comma
            time = time.mid(2);
            QDate date = QDate::fromString(time, "d MMMM yyyy");
            if (!date.isValid())
            {
                page = page.replace(sections.at(CurrentIndex), "");
                CurrentIndex++;
                continue;
            } else
            {
                // now check if it's at least 1 month old
                if (QDate::currentDate().addDays(Configuration::HuggleConfiguration->LocalConfig_TemplateAge) > date)
                {
                    // we don't want to parse this thing
                    page = page.replace(sections.at(CurrentIndex), "");
                    CurrentIndex++;
                    continue;
                }
            }
            CurrentIndex++;
        }
    }

    int level = 4;
    while (level > 0)
    {
        int xx=0;
        while (xx<Configuration::HuggleConfiguration->LocalConfig_WarningDefs.count())
        {
            QString defs=Configuration::HuggleConfiguration->LocalConfig_WarningDefs.at(xx);
            if (HuggleParser::GetKeyFromValue(defs).toInt() == level)
            {
                if (page.contains(HuggleParser::GetValueFromKey(defs)))
                {
                    return level;
                }
            }
            xx++;
        }
        level--;
    }
    return 0;
}
