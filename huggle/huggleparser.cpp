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

byte HuggleParser::GetLevel(QString page, QDate bt)
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
                if (bt.addDays(Configuration::HuggleConfiguration->LocalConfig_TemplateAge) > date)
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

    byte level = 4;
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

QStringList HuggleParser::ConfigurationParse_QL(QString key, QString content, bool CS)
{
    QStringList list;
    if (content.startsWith(key + ":"))
    {
        QString value = content.mid(key.length() + 1);
        QStringList lines = value.split("\n");
        int curr = 1;
        while (curr < lines.count())
        {
            QString _line = HuggleParser::Trim(lines.at(curr));
            if (_line.endsWith(","))
            {
                list.append(_line);
            } else
            {
                if (_line != "")
                {
                    list.append(_line);
                    break;
                }
            }
            curr++;
        }
        if (CS)
        {
            // now we need to split values by comma as well
            QStringList f;
            int c = 0;
            while (c<list.count())
            {
                QStringList xx = list.at(c).split(",");
                int i2 = 0;
                while (i2<xx.count())
                {
                    if (HuggleParser::Trim(xx.at(i2)) != "")
                    {
                        f.append(HuggleParser::Trim(xx.at(i2)));
                    }
                    i2++;
                }
                c++;
            }
            list = f;
        }
        return list;
    } else if (content.contains("\n" + key + ":"))
    {
        QString value = content.mid(content.indexOf("\n" + key + ":") + key.length() + 2);
        QStringList lines = value.split("\n");
        int curr = 1;
        while (curr < lines.count())
        {
            QString _line = HuggleParser::Trim(lines.at(curr));
            if (_line.endsWith(","))
            {
                list.append(_line);
            } else
            {
                if (_line != "")
                {
                    list.append(_line);
                    break;
                }
            }
            curr++;
        }
        if (CS)
        {
            // now we need to split values by comma as well
            QStringList f;
            int c = 0;
            while (c<list.count())
            {
                QStringList xx = list.at(c).split(",");
                int i2 = 0;
                while (i2<xx.count())
                {
                    if (HuggleParser::Trim(xx.at(i2)) != "")
                    {
                        f.append(HuggleParser::Trim(xx.at(i2)));
                    }
                    i2++;
                }
                c++;
            }
            list = f;
        }
        return list;
    }
    return list;
}

QStringList HuggleParser::ConfigurationParse_QL(QString key, QString content, QStringList list, bool CS)
{
    QStringList result = HuggleParser::ConfigurationParse_QL(key, content, CS);
    if (result.count() == 0)
    {
        return list;
    }
    return result;
}

QStringList HuggleParser::ConfigurationParseTrimmed_QL(QString key, QString content, bool CS, bool RemoveNull)
{
    QStringList result = HuggleParser::ConfigurationParse_QL(key, content, CS);
    int x = 0;
    QStringList trimmed;
    while (x < result.count())
    {
        QString item = result.at(x);
        if (RemoveNull && item.replace(",", "") == "")
        {
            x++;
            continue;
        }
        if (item.endsWith(","))
        {
            trimmed.append(item.mid(0, item.length() - 1));
        } else
        {
            trimmed.append(item);
        }
        x++;
    }
    return trimmed;
}

QList<HuggleQueueFilter*> HuggleParser::ConfigurationParseQueueList(QString content, bool locked)
{
    QList<HuggleQueueFilter*> ReturnValue;

    if (!content.contains("queues:"))
    {
        return ReturnValue;
    }

    // we need to parse all blocks that contain information about queue
    content = content.mid(content.indexOf("queues:") + 8);
    QStringList Filtered = content.split("\n");
    QStringList Info;

    // we need to assume that all queues are intended with at least 4 spaces
    int line = 0;

    while (line < Filtered.count())
    {
        QString lt = Filtered.at(line);
        if (lt.startsWith("    ") || lt == "")
        {
            Info.append(lt);
        } else
        {
            // we reached the end of block with queue defs
            break;
        }
        line++;
    }

    // now we can split the queue info
    line = 0;
    while (line < Info.count())
    {
        QString text = Info.at(line);
        if (text.startsWith("    ") && !text.startsWith("        ") && text.contains(":"))
        {
            // this is a queue definition beginning, because it is intended with 4 spaces
            HuggleQueueFilter *filter = new HuggleQueueFilter();
            // we need to disable all filters because that's how is it expected in config for some reason
            filter->setIgnoreBots(false);
            filter->setIgnoreFriends(false);
            filter->setIgnoreIP(false);
            filter->setIgnoreMinor(false);
            filter->setIgnoreNP(false);
            filter->setIgnoreReverts(false);
            filter->setIgnoreSelf(false);
            filter->setIgnoreUsers(false);
            filter->setIgnoreWL(false);
            ReturnValue.append(filter);
            filter->ProjectSpecific = locked;
            QString name = text;
            while (name.startsWith(" "))
            {
                name = name.mid(1);
            }
            name.replace(":", "");
            filter->QueueName = name;
            line++;
            text = Info.at(line);
            while (text.startsWith("        ") && text.contains(":") && line < Info.count())
            {
                // we need to parse the info
                line++;
                while (text.startsWith(" "))
                {
                    text = text.mid(1);
                }
                QString val = text.mid(text.indexOf(":") + 1);
                QString key = text.mid(0, text.indexOf(":"));
                text = Info.at(line);
                if (key == "filter-ignored")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreWL(true);
                    } else
                    {
                        filter->setIgnoreWL(false);
                    }
                    continue;
                }
                if (key == "filter-bots")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreBots(true);
                    } else
                    {
                        filter->setIgnoreBots(false);
                    }
                    continue;
                }
                if (key == "filter-assisted")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreFriends(true);
                    } else
                    {
                        filter->setIgnoreFriends(false);
                    }
                    continue;
                }
                if (key == "filter-ip")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreIP(true);
                    } else
                    {
                        filter->setIgnoreIP(false);
                    }
                    continue;
                }
                if (key == "filter-reverts")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreReverts(true);
                    } else
                    {
                        filter->setIgnoreReverts(false);
                    }
                    continue;
                }
                if (key == "filter-new-pages")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreNP(true);
                    } else
                    {
                        filter->setIgnoreNP(false);
                    }
                    continue;
                }
                if (key == "filter-me")
                {
                    if (val == "exclude")
                    {
                        filter->setIgnoreSelf(true);
                    } else
                    {
                        filter->setIgnoreSelf(false);
                    }
                    continue;
                }
            }
        } else
        {
            line++;
        }
    }

    return ReturnValue;
}
