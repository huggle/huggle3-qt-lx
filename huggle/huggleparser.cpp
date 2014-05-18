//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggleparser.hpp"
#include "configuration.hpp"

using namespace Huggle;

QString HuggleParser::GetSummaryOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->ProjectConfig_RevertSummaries.count())
    {
        QString line = Configuration::HuggleConfiguration->ProjectConfig_RevertSummaries.at(id);
        if (line.startsWith(key + ";"))
        {
            return HuggleParser::GetValueFromKey(line);
        }
        id++;
    }
    return Configuration::HuggleConfiguration->ProjectConfig_DefaultSummary;
}

QString HuggleParser::GetNameOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.count())
    {
        QString line = Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(id);
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
    while (i<Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.count())
    {
        QString line = Configuration::HuggleConfiguration->ProjectConfig_WarningTypes.at(i);
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
    Configuration::HuggleConfiguration->ProjectConfig_ScoreParts.clear();
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
                QString w = items.at(CurrentItem).trimmed();
                if (w.length() == 0)
                {
                    CurrentItem++;
                    continue;
                }
                word.append(w);
                CurrentItem++;
            }
            if (!l.endsWith(",") || l.trimmed().length() <= 0)
            {
                break;
            }
            line++;
        }
        line = 0;
        while (line < word.count())
        {
            Configuration::HuggleConfiguration->ProjectConfig_ScoreParts.append(ScoreWord(word.at(line), score));
            line++;
        }
    }
}

void HuggleParser::ParseWords(QString text)
{
    Configuration::HuggleConfiguration->ProjectConfig_ScoreWords.clear();
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
        if (!text.contains(":"))
        {
            return;
        }
        text = text.mid(text.indexOf(":") + 1);
        QStringList lines = text.split("\n");
        int line = 1;
        QStringList word;
        while (line < lines.count())
        {
            QString l = lines.at(line);
            QStringList items = l.split(",");
            int CurrentItem = 0;
            while ( CurrentItem < items.count() )
            {
                QString w = items.at(CurrentItem).trimmed();
                if (w.length() == 0)
                {
                    CurrentItem++;
                    continue;
                }
                word.append(w);
                CurrentItem++;
            }
            if (!l.endsWith(",") || l.trimmed().length() == 0)
            {
                break;
            }
            line++;
        }
        line = 0;
        while (line < word.count())
        {
            Configuration::HuggleConfiguration->ProjectConfig_ScoreWords.append(ScoreWord(word.at(line), score));
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

static int DateMark(QString page)
{
    QStringList marks;
    marks << "(UTC)" << "(CET)" << "(CEST)";
    int m = 0;
    int position = 0;
    QString mark = "";
    while (m < marks.count())
    {
        QString m_ = marks.at(m);
        if (page.contains(m_))
        {
            int mp = page.lastIndexOf(m_);
            if (mp > position)
            {
                mark = m_;
                position = mp;
            }
        }
        m++;
    }
    return position;
}

byte_ht HuggleParser::GetLevel(QString page, QDate bt)
{
    if (Configuration::HuggleConfiguration->TrimOldWarnings)
    {
        // we need to get rid of old warnings now
        QStringList sections;
        // windows fix
        page.replace("\r", "");
        while (page.length() > 1)
        {
            while (page[0] == '\n')
            {
                // remove all leading extra lines on page
                page = page.mid(1);
            }
            if (!page.contains("\n\n"))
            {
                // no sections
                sections.append(page);
                break;
            }
            // get to bottom of it
            int bottom = 0;
            bottom = page.indexOf("\n\n");
            QString section = page.mid(0, bottom);
            page = page.mid(bottom + 2);
            sections.append(section);
        }

        // now we browse all sections and remove these with no current date
        int CurrentIndex = 0;
        page = "";
        while (CurrentIndex < sections.count())
        {
            int dp = DateMark(sections.at(CurrentIndex));
            // we need to find a date in this section
            if (!dp)
            {
                // there is none
                CurrentIndex++;
                continue;
            }
            QString section = sections.at(CurrentIndex);
            section = section.mid(0, dp).trimmed();
            if (!section.contains(","))
            {
                // this is some borked date let's remove it
                CurrentIndex++;
                continue;
            }
            QString time = section.mid(section.lastIndexOf(","));
            if (time.length() < 2)
            {
                // what the fuck
                Syslog::HuggleLogs->DebugLog("Negative position: " + time);
                CurrentIndex++;
                continue;
            }
            // we remove the comma
            time = time.mid(2);
            // now we need this uberhack so that we can get a month name from localized version
            // let's hope that month is a word in a middle of string
            QString month_name = time;
            QStringList parts_time = time.split(' ');
            if (parts_time.count() < 3)
            {
                // this is invalid string
                Syslog::HuggleLogs->DebugLog("Unable to convert month to number: " + time, 12);
                CurrentIndex++;
                continue;
            }
            month_name = parts_time.at(1);
            int month = HuggleParser::GetIDOfMonth(month_name);
            if (month > 0)
            {
                // let's create a new time string from converted one, just to make sure it will be parsed properly
                time = parts_time.at(0) + " " + QString::number(month) + " " + parts_time.at(2);
            }
            QDate date = QDate::fromString(time, "d M yyyy");
            if (!date.isValid())
            {
                Syslog::HuggleLogs->DebugLog("Invalid date: " + time);
                CurrentIndex++;
                continue;
            } else
            {
                // now check if it's at least 1 month old
                if (bt.addDays(Configuration::HuggleConfiguration->ProjectConfig_TemplateAge) > date)
                {
                    // we don't want to parse this thing
                    CurrentIndex++;
                    continue;
                }
            }
            page += sections.at(CurrentIndex) + "\n";
            CurrentIndex++;
        }
    }
    byte_ht level = 4;
    while (level > 0)
    {
        int xx=0;
        while (xx<Configuration::HuggleConfiguration->ProjectConfig_WarningDefs.count())
        {
            QString defs=Configuration::HuggleConfiguration->ProjectConfig_WarningDefs.at(xx);
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
            QString _line = lines.at(curr).trimmed();
            if (_line.endsWith(","))
            {
                list.append(_line);
            } else
            {
                if (_line.length() > 0)
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
                    QString item = xx.at(i2).trimmed();
                    if (item.length() > 0)
                    {
                        f.append(item);
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
            QString _line = lines.at(curr).trimmed();
            if (_line.endsWith(","))
            {
                list.append(_line);
            } else
            {
                if (_line.length() > 0)
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
                    QString item_ = xx.at(i2).trimmed();
                    if (item_.length() > 0)
                    {
                        f.append(item_);
                    }
                    i2++;
                }
                c++;
            }
            list = f;
        }
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
        if (RemoveNull && item.replace(",", "").length() < 1)
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

static bool F2B(QString result)
{
    return result == "exclude";
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
    QStringList Filtered = content.replace("\r", "").split("\n");
    QStringList Info;
    // we need to assume that all queues are intended with at least 4 spaces
    int line = 0;
    while (line < Filtered.count())
    {
        QString lt = Filtered.at(line);
        if (lt.startsWith("    ") || lt.length() == 0)
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
            filter->setIgnore_UserSpace(false);
            filter->setIgnoreUsers(false);
            filter->setIgnoreWL(false);
            ReturnValue.append(filter);
            filter->ProjectSpecific = locked;
            QString name = text.trimmed();
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
                    filter->setIgnoreWL(F2B(val));
                    continue;
                }
                if (key == "filter-bots")
                {
                    filter->setIgnoreBots(F2B(val));
                    continue;
                }
                if (key == "filter-assisted")
                {
                    filter->setIgnoreFriends(F2B(val));
                    continue;
                }
                if (key == "filter-ip")
                {
                    filter->setIgnoreIP(F2B(val));
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
                    filter->setIgnoreNP(F2B(val));
                    continue;
                }
                if (key == "filter-me")
                {
                    filter->setIgnoreSelf(F2B(val));
                    continue;
                }
                if (key == "nsfilter-user")
                {
                    filter->setIgnore_UserSpace(F2B(val));
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


int HuggleParser::GetIDOfMonth(QString month)
{
    int i = 0;
    month = month.toLower();
    while (i < Configuration::HuggleConfiguration->Months.count())
    {
        if (Configuration::HuggleConfiguration->Months.at(i).toLower() == month)
            return i+1;
        i++;
    }
    i = 1;
    while (i < 13)
    {
        if (Configuration::HuggleConfiguration->ProjectConfig_AlternativeMonths[i].contains(month))
            return i;
        i++;
    }
    return -800;
}
