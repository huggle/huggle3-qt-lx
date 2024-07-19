//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exception.hpp"
#include "huggleparser.hpp"
#include "huggleprofiler.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "userconfiguration.hpp"
#include "projectconfiguration.hpp"
#include "syslog.hpp"
#include "wikisite.hpp"
#include <yaml-cpp/yaml.h>

using namespace Huggle;

QString HuggleParser::UserConfig_NonEmpty(const QString& key, const QString& value, const QString& default_val)
{
    // We have to copy the string here, because replace() in Qt alters the string itself
    QString temp = value;
    // Remove all whitespace and special symbols
    temp.replace(" ", "");
    temp.replace("\t", "");
    if (temp.isEmpty())
    {
        HUGGLE_WARNING("User configuration key " + key + " had empty value, which is not allowed for this key. Restoring to project config value: " + default_val);
        return default_val;
    }
    return value;
}

QString HuggleParser::ConfigurationParse(const QString &key, const QString &content, const QString &missing, bool non_empty)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    /// \todo this parses the config a lot different than HG2 (here only one line, mising replaces...)
    // if first line in config
    if (content.startsWith(key + ":"))
    {
        QString value = content.mid(key.length() + 1);
        if (value.contains("\n"))
        {
            value = value.mid(0, value.indexOf("\n"));
        }
        if (non_empty)
            return UserConfig_NonEmpty(key, value, missing);
        return value;
    }

    // make sure it's not inside of some string
    if (content.contains("\n" + key + ":"))
    {
        QString value = content.mid(content.indexOf("\n" + key + ":") + key.length() + 2);
        if (value.contains("\n"))
        {
            value = value.mid(0, value.indexOf("\n"));
        }
        if (non_empty)
            return UserConfig_NonEmpty(key, value, missing);
        return value;
    }
    return missing;
}

bool HuggleParser::ConfigurationParseBool(const QString &key, const QString &content, bool missing)
{
    return Generic::SafeBool(ConfigurationParse(key, content, Generic::Bool2String(missing)));
}

QString HuggleParser::GetSummaryOfWarningTypeFromWarningKey(const QString& key, ProjectConfiguration *project_conf, UserConfiguration *user_conf)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    foreach (QString line, project_conf->RevertSummaries)
        if (line.startsWith(key + ";"))
            return HuggleParser::GetValueFromSSItem(line);
    if (!user_conf)
        return project_conf->DefaultSummary;

    return user_conf->DefaultSummary;
}

QString HuggleParser::GetNameOfWarningTypeFromWarningKey(QString key, ProjectConfiguration *project_conf)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    // get a key
    foreach (QString line, project_conf->WarningTypes)
        if (line.startsWith(key + ";"))
            return HuggleParser::GetValueFromSSItem(line);
    return key;
}

QString HuggleParser::GetKeyOfWarningTypeFromWarningName(QString id, ProjectConfiguration *project_conf)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    foreach (QString line, project_conf->WarningTypes)
    {
        if (line.endsWith(id) || line.endsWith(id + ","))
        {
            return HuggleParser::GetKeyFromSSItem(line);
        }
    }
    return id;
}

static QList<ScoreWord> ParseScoreWords(QString text, const QString& wt)
{
    QList<ScoreWord> contents;
    while (text.contains(wt + "("))
    {
        text = text.mid(text.indexOf(wt + "(") + wt.length() + 1);
        if (!text.contains(")"))
        {
            return contents;
        }
#ifdef QT6_BUILD
        int score = text.mid(0, text.indexOf(")")).toInt();
#else
        int score = text.midRef(0, text.indexOf(")")).toInt();
#endif
        if (score == 0)
        {
            continue;
        }
        if (!text.contains(":"))
        {
            return contents;
        }
        text = text.mid(text.indexOf(":") + 1);
        QStringList lines = text.split("\n");
        int line = 1;
        QStringList word;
        while (line < lines.count())
        {
            const QString& l = lines.at(line);
            QStringList items = l.split(",");
            foreach(QString w, items)
            {
                w = w.trimmed();
                if (w.isEmpty())
                    continue;
                word.append(w);
            }
            if (l.trimmed().isEmpty() || !l.endsWith(","))
                break;
            line++;
        }
        foreach(QString w, word)
            contents.append(ScoreWord(w, score));
    }
    return contents;
}

void HuggleParser::ParseNoTalkWords(const QString &text, WikiSite *site)
{
    site->ProjectConfig->NoTalkScoreWords.clear();
    site->ProjectConfig->NoTalkScoreWords = ParseScoreWords(text, "score-words-no-talk");
}

void HuggleParser::ParseNoTalkPatterns(const QString &text, WikiSite *site)
{
    site->ProjectConfig->NoTalkScoreParts.clear();
    site->ProjectConfig->NoTalkScoreParts = ParseScoreWords(text, "score-parts-no-talk");
}

void HuggleParser::ParsePatterns(const QString &text, WikiSite *site)
{
    site->ProjectConfig->ScoreParts.clear();
    site->ProjectConfig->ScoreParts = ParseScoreWords(text, "score-parts");
}

void HuggleParser::ParseWords(const QString &text, WikiSite *site)
{
    site->ProjectConfig->ScoreWords.clear();
    site->ProjectConfig->ScoreWords = ParseScoreWords(text, "score-words");
}

QString HuggleParser::GetValueFromSSItem(QString item)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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

QString HuggleParser::GetKeyFromSSItem(QString item)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (item.contains(";"))
    {
        QString type = item.mid(0, item.indexOf(";"));
        return type;
    }
    return item;
}

static int DateMark(const QString& page, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int m = 0;
    int position = 0;
    QString mark = "";
    while (m < site->GetProjectConfig()->Parser_Date_Suffix.count())
    {
        QString m_ = site->GetProjectConfig()->Parser_Date_Suffix.at(m);
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

byte_ht HuggleParser::GetLevel(QString page, QDate bt, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (Configuration::HuggleConfiguration->SystemConfig_TrimOldWarnings)
    {
        // we want to know the highter warning level present on this talk page
        // cut talk page content in sections (all paragraph that are separated by a blank line)
        // to try to find warnings (by tags they have) and date (discard too old messages depending on config)
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
            int dp = DateMark(sections.at(CurrentIndex), site);
            // we need to find a date in this section
            if (!dp)
            {
                // there is none
                CurrentIndex++;
                continue;
            }

            // discard content after CET or CEST, because we know the date is just before it
            QString section = sections.at(CurrentIndex);
            section = section.mid(0, dp).trimmed();

            // language-specific logic may be needed to parse dates from signature
            // French dates in signatures have dates in a fixed position before the CET: 30 novembre 2023 à 22:10 (CET)
            // English ones are exactly between a comma and UTC:  22:58, 17 July 2023 (UTC)
            QString time;
            if (site->Name.startsWith("fr")) {
                QStringList parts_section = section.split(' ');
                // we know the last part from 5 spaces before the end to the CET (end of string) is the date/time
                if (parts_section.length() < 5) {
                    // this is some borked date let's remove it
                    CurrentIndex++;
                    continue;
                }
                time = parts_section.at(parts_section.length() - 5) + " " + parts_section.at(parts_section.length() - 4) + " " + parts_section.at(parts_section.length() - 3);
            } else {
                if (!section.contains(site->GetProjectConfig()->Parser_Date_Prefix))
                {
                    // this is some borked date let's remove it
                    CurrentIndex++;
                    continue;
                }
                time = section.mid(section.lastIndexOf(site->GetProjectConfig()->Parser_Date_Prefix) + site->GetProjectConfig()->Parser_Date_Prefix.length());
            }

            // now we need this uberhack so that we can get a month name from localized version
            // let's hope that month is a word in a middle of string
            time = time.trimmed();
            QString month_name = "";
            QStringList parts_time = time.split(' ');
            if (parts_time.count() < 3)
            {
                // this is invalid string
                HUGGLE_DEBUG("Unable to split month: " + time, 12);
                CurrentIndex++;
                continue;
            }
            QString day = parts_time.at(0);
            // e.g. dewiki's days end with dot
            if(day.endsWith('.'))
                day = day.mid(0, day.length() - 1);
            // on some wikis months have spaces in name
            int i = 1;
            while (i < parts_time.count() - 1)
            {
                month_name += parts_time.at(i) + " ";
                i++;
            }
            month_name = month_name.trimmed();
            byte_ht month = HuggleParser::GetIDOfMonth(month_name, site);

             // let's create a new time string from converted one, just to make sure it will be parsed properly
            if (month > 0)
            {
                time = day + " " + QString::number(month) + " " + parts_time.last();
            } else
            {
                time = day + " " + parts_time.at(1); + " " + parts_time.last();
            }
            QDate date = QDate::fromString(time, "d M yyyy");
            if (!date.isValid())
            {
                HUGGLE_DEBUG("Invalid date: " + time, 1);
                CurrentIndex++;
                continue;
            } else
            {
                // now check if it's more recent than the delay in config (ie 1 month)
                if (bt.addDays(site->ProjectConfig->TemplateAge) > date)
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

    // now searching in user talk page tags as defined in wiki config, like <!-- Template:Huggle/warn-spam-1 -->
    // and keep the highter one found
    byte_ht level = 4;
    while (level > 0)
    {
        foreach (QString df, site->ProjectConfig->WarningDefs)
        {
            if (HuggleParser::GetKeyFromSSItem(df).toInt() == level && page.contains(HuggleParser::GetValueFromSSItem(df)))
            {
                return level;
            }
        }
        level--;
    }
    return 0;
}

QStringList HuggleParser::ConfigurationParse_QL(const QString &key, const QString &content, bool CS)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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

QStringList HuggleParser::ConfigurationParse_QL(const QString &key, const QString &content, QStringList list, bool CS)
{
    QStringList result = HuggleParser::ConfigurationParse_QL(key, content, CS);
    if (result.count() == 0)
    {
        return list;
    }
    return result;
}

QStringList HuggleParser::ConfigurationParseTrimmed_QL(const QString &key, const QString &content, bool CS, bool RemoveNull)
{
    QStringList result = HuggleParser::ConfigurationParse_QL(key, content, CS);
    QStringList trimmed;
    foreach (QString item, result)
    {
        if (RemoveNull)
        {
            // this replace must be done on copy of string because this Qt function will modify the original string
            QString rp = item;
            rp = rp.replace(",", "");
            if (rp.isEmpty())
            {
                // we don't want to process a null string here
                continue;
            }
        }
        if (item.endsWith(","))
            trimmed.append(item.mid(0, item.length() - 1));
        else
            trimmed.append(item);
    }
    return trimmed;
}

static HuggleQueueFilterMatch F2B(const QString& result)
{
    if (result == "exclude")
        return HuggleQueueFilterMatchExclude;
    if (result == "require")
        return HuggleQueueFilterMatchRequire;
    return HuggleQueueFilterMatchIgnore;
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
    foreach (QString lt, Filtered)
    {
        if (lt.startsWith("    ") || lt.length() == 0)
        {
            Info.append(lt);
        } else
        {
            // we reached the end of block with queue defs
            break;
        }
    }
    // now we can split the queue info
    int line = 0;
    while (line < Info.count())
    {
        QString text = Info.at(line);
        if (text.startsWith("    ") && !text.startsWith("        ") && text.contains(":"))
        {
            // this is a queue definition beginning, because it is intended with 4 spaces
            HuggleQueueFilter *filter = new HuggleQueueFilter();
            // we need to disable all filters because that's how is it expected in config for some reason
            filter->setIgnoreBots(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreFriends(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreIP(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreMinor(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreNP(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreTalk(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreReverts(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreSelf(HuggleQueueFilterMatchIgnore);
            filter->setIgnore_UserSpace(HuggleQueueFilterMatchIgnore);
            filter->setIgnoreWL(HuggleQueueFilterMatchIgnore);
            ReturnValue.append(filter);
            filter->ProjectSpecific = locked;
            QString name = text.trimmed();
            name.replace(":", "");
            filter->QueueName = name;
            line++;
            if (line >= Info.count())
                goto exit;
            text = Info.at(line);
            while (text.startsWith("        ") && text.contains(":") && line < Info.count())
            {
                // we need to parse the info
                line++;
                if (line >= Info.count())
                    goto exit;
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
                if (key == "filtered-ns")
                {
                    QStringList ns = val.split(",");
                    foreach (QString namespace_id, ns)
                    {
                        if (namespace_id.isEmpty())
                            continue;
                        int nsid = namespace_id.toInt();
                        if (filter->Namespaces.contains(nsid))
                            filter->Namespaces[nsid] = true;
                        else
                            filter->Namespaces.insert(nsid, true);
                    }
                    continue;
                }
                if (key == "filter-assisted")
                {
                    filter->setIgnoreFriends(F2B(val));
                    continue;
                }
                if (key == "filter-talk")
                {
                    filter->setIgnoreTalk(F2B(val));
                    continue;
                }
                if (key == "filter-ip")
                {
                    filter->setIgnoreIP(F2B(val));
                    continue;
                }
                if (key == "filter-reverts")
                {
                    filter->setIgnoreReverts(F2B(val));
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
                if (key == "filter-watched")
                {
                    filter->setIgnoreWatched(F2B(val));
                }
                if (key == "nsfilter-user")
                {
                    filter->setIgnore_UserSpace(F2B(val));
                    continue;
                }
                if (key == "required-tags")
                {
                    filter->SetRequiredTags_CommaSeparated(val);
                    continue;
                }
                if (key == "ignored-tags")
                {
                    filter->SetIgnoredTags_CommaSeparated(val);
                    continue;
                }
                if (key == "required-categories")
                {
                    filter->SetRequiredCategories_CommaSeparated(val);
                    continue;
                }
                if (key == "ignored-categories")
                {
                    filter->SetIgnoredCategories_CommaSeparated(val);
                    continue;
                }
            }
        } else
        {
            line++;
        }
    }
  exit:
    return ReturnValue;
}

byte_ht HuggleParser::GetIDOfMonth(QString month, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int i = 0;
    month = month.toLower();
    while (i < site->ProjectConfig->Months.count())
    {
        if (site->ProjectConfig->Months.at(i).toLower() == month)
            return static_cast<byte_ht>(i+1);
        i++;
    }
    i = 1;
    while (i < 13)
    {
        if (site->ProjectConfig->AlternativeMonths[i].contains(month, Qt::CaseInsensitive))
            return static_cast<byte_ht>(i);
        i++;
    }
    return -6;
}

QString HuggleParser::FetchYAML(QString source, bool *failed)
{
    if (!source.contains("---"))
    {
        if (failed)
            *failed = true;
        return "";
    }

    if (failed)
        *failed = false;

    if (source.contains(HUGGLE_BOC))
    {
        source = source.mid(source.indexOf(HUGGLE_BOC) + QString(HUGGLE_BOC).length());
    }

    source.replace("<syntaxhighlight lang='yaml'>", "");
    source.replace("<syntaxhighlight lang=\"yaml\">", "");
    source.replace("<syntaxhighlight lang=yaml>", "");
    source.replace("</syntaxhighlight>", "");

    return source;
}

bool HuggleParser::YAML2Bool(const QString& key, YAML::Node &node, bool missing)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        return node[key.toStdString()].as<bool>(missing);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

QString HuggleParser::YAML2String(const QString& key, YAML::Node &node, const QString &missing, bool non_empty)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        // This is needed for some really weird OSX related bug that randomly segfaults, if we don't make a copy of strings
        std::string temp = node[key.toStdString()].as<std::string>(missing.toStdString());
        if (non_empty)
            return UserConfig_NonEmpty(key, QString::fromStdString(temp), missing);
        return QString::fromStdString(temp);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

int HuggleParser::YAML2Int(const QString& key, YAML::Node &node, int missing)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        return node[key.toStdString()].as<int>(missing);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

unsigned int HuggleParser::YAML2UInt(const QString& key, YAML::Node &node, unsigned int missing)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        return node[key.toStdString()].as<unsigned int>(missing);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

double HuggleParser::YAML2Double(const QString& key, YAML::Node &node, double missing)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        return node[key.toStdString()].as<double>(missing);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

long long HuggleParser::YAML2LongLong(const QString& key, YAML::Node &node, long long missing)
{
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        return node[key.toStdString()].as<long long>(missing);
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

QStringList HuggleParser::YAML2QStringList(YAML::Node &node, bool *ok)
{
    QStringList missing;
    return YAML2QStringList(node, missing, ok);
}

QStringList HuggleParser::YAML2QStringList(YAML::Node &node, QStringList missing, bool *ok)
{
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            return missing;

        QStringList results;
        if (!node.IsSequence())
            return missing;

        // Even if it's empty, we are good to go
        if (ok)
            *ok = true;

        for (auto list_item : node)
        {
            std::string value = list_item.as<std::string>();
            results << QString::fromStdString(value);
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (node to QStringList): " + QString(exception.what()));
    }
    return missing;
}

QStringList HuggleParser::YAML2QStringList(const QString &key, YAML::Node &node, bool *ok)
{
    QStringList missing;
    return YAML2QStringList(key, node, missing, ok);
}

QStringList HuggleParser::YAML2QStringList(const QString& key, YAML::Node &node, QStringList missing, bool *ok)
{
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        YAML::Node temp = node[key.toStdString()];
        return YAML2QStringList(temp, missing, ok);

    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

QList<QStringList> HuggleParser::YAML2QListOfQStringList(const QString& key, YAML::Node &node, bool *ok)
{
    QList<QStringList> results;
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            return results;

        if (!node[key.toStdString()])
            return results;

        YAML::Node seq = node[key.toStdString()];

        if (!seq.IsSequence())
            return results;

        // Even if it's empty, we are good to go
        if (ok)
            *ok = true;

        for (std::size_t i=0;i<seq.size();i++)
        {
            YAML::Node temp = seq[i];
            results << YAML2QStringList(temp, ok);
            if (ok && !*ok)
            {
                HUGGLE_ERROR("Failed parsing list of string lists: " + key);
                return results;
            }
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error " + key + " (node to List of QStringLists): " + QString(exception.what()));
    }
    return results;
}

QHash<QString, QString> HuggleParser::YAML2QStringHash(YAML::Node &node, bool *ok)
{
    QHash<QString, QString> results;
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        // Even if it's empty, we are good to go
        if (ok)
            *ok = true;

        for (YAML::const_iterator it=node.begin();it!=node.end();++it)
        {
            // This is needed for some really weird OSX related bug that randomly segfaults, if we don't make a copy of strings
            std::string temp1 = it->first.as<std::string>();
            std::string temp2 = it->second.as<std::string>();
            results.insert(QString::fromStdString(temp1), QString::fromStdString(temp2));
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error: " + QString(exception.what()));
    }
    return results;
}

QHash<QString, QString> HuggleParser::YAML2QStringHash(const QString &key, YAML::Node &node, bool *ok)
{
    QHash<QString, QString> missing;
    return YAML2QStringHash(key, node, missing, ok);
}

QHash<QString, QVariant> HuggleParser::YAML2QHash(const QString &key, YAML::Node &node, QHash<QString, QVariant> missing, bool *ok)
{
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        QHash<QString, QVariant> results;
        YAML::Node seq = node[key.toStdString()];

        // Even if it's empty, we are good to go
        if (ok)
            *ok = true;

        for (YAML::const_iterator it=seq.begin();it!=seq.end();++it)
        {
            switch(it->second.Type())
            {
                case YAML::NodeType::Null:
                    results.insert(QString::fromStdString(it->first.as<std::string>()), QVariant());
                    break;
                case YAML::NodeType::Scalar:
                    results.insert(QString::fromStdString(it->first.as<std::string>()), QString::fromStdString(it->second.as<std::string>()));
                    break;
                case YAML::NodeType::Sequence:
                {
                    YAML::Node second = it->second;
                    results.insert(QString::fromStdString(it->first.as<std::string>()), YAML2QStringList(second, ok));
                }
                    break;
                case YAML::NodeType::Map:
                    *ok = false;
                    HUGGLE_ERROR("YAML Parsing error (" + key + "): It's not possible to convert QHash to QVariant");
                    return missing;
                case YAML::NodeType::Undefined:
                    break;
            }
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

QHash<QString, QString> HuggleParser::YAML2QStringHash(const QString &key, YAML::Node &node, QHash<QString, QString> missing, bool *ok)
{
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return missing;

        QHash<QString, QString> results;
        YAML::Node seq = node[key.toStdString()];

        // Even if it's empty, we are good to go
        if (ok)
            *ok = true;

        for (YAML::const_iterator it=seq.begin();it!=seq.end();++it)
        {
            results.insert(QString::fromStdString(it->first.as<std::string>()), QString::fromStdString(it->second.as<std::string>()));
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return missing;
}

QHash<QString, QHash<QString, QString>> HuggleParser::YAML2QHashOfHash(const QString& key, YAML::Node &node, bool *ok)
{
    QHash<QString, QHash<QString, QString>> results;
    if (ok)
        *ok = false;
    try
    {
        if (!node)
            throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

        if (!node[key.toStdString()])
            return results;

        YAML::Node seq = node[key.toStdString()];

        // Even if it's empty, we are good to go
        if (ok)
            *ok = true;

        for (YAML::const_iterator it=seq.begin();it!=seq.end();++it)
        {
            QHash<QString, QString> hash;
            YAML::Node yaml_n = it->second;
            for (YAML::const_iterator it2=yaml_n.begin();it2!=yaml_n.end();++it2)
            {
                // This is needed for some really weird OSX related bug that randomly segfaults, if we don't make a copy of strings
                std::string temp1 = it2->first.as<std::string>();
                std::string temp2 = it2->second.as<std::string>();
                hash.insert(QString::fromStdString(temp1), QString::fromStdString(temp2));
            }
            // This is needed for some really weird OSX related bug that randomly segfaults, if we don't make a copy of strings
            std::string temp = it->first.as<std::string>();
            results.insert(QString::fromStdString(temp), hash);
        }
        return results;
    } catch (YAML::Exception exception)
    {
        HUGGLE_ERROR("YAML Parsing error (" + key + "): " + QString(exception.what()));
    }
    return QHash<QString, QHash<QString, QString>>();
}

QList<HuggleQueueFilter *> HuggleParser::ConfigurationParseQueueList_YAML(YAML::Node &node, bool locked)
{
    QList<HuggleQueueFilter*> ReturnValue;

    if (!node)
        throw new Huggle::NullPointerException("YAML::Node *node", BOOST_CURRENT_FUNCTION);

    if (!node["queues"])
        return ReturnValue;

    YAML::Node seq = node["queues"];

    for (YAML::const_iterator it=seq.begin();it!=seq.end();++it)
    {
        HuggleQueueFilter *filter = new HuggleQueueFilter();
        filter->QueueName = QString::fromStdString(it->first.as<std::string>());
        YAML::Node queue_n = it->second;
        QHash<QString, QString> queue_data = HuggleParser::YAML2QStringHash(queue_n);
        filter->ProjectSpecific = locked;
        filter->setIgnoreBots(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreFriends(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreIP(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreMinor(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreNP(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreTalk(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreReverts(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreSelf(HuggleQueueFilterMatchIgnore);
        filter->setIgnore_UserSpace(HuggleQueueFilterMatchIgnore);
        filter->setIgnoreWL(HuggleQueueFilterMatchIgnore);
        ReturnValue.append(filter);
        foreach (QString key, queue_data.keys())
        {
            QString val = queue_data[key];
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
            if (key == "filtered-ns")
            {
                QStringList ns = val.split(",");
                foreach (QString namespace_id, ns)
                {
                    if (namespace_id.isEmpty())
                        continue;
                    int nsid = namespace_id.toInt();
                    if (filter->Namespaces.contains(nsid))
                        filter->Namespaces[nsid] = true;
                    else
                        filter->Namespaces.insert(nsid, true);
                }
                continue;
            }
            if (key == "filter-assisted")
            {
                filter->setIgnoreFriends(F2B(val));
                continue;
            }
            if (key == "filter-talk")
            {
                filter->setIgnoreTalk(F2B(val));
                continue;
            }
            if (key == "filter-ip")
            {
                filter->setIgnoreIP(F2B(val));
                continue;
            }
            if (key == "filter-reverts")
            {
                filter->setIgnoreReverts(F2B(val));
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
            if (key == "filter-watched")
            {
                filter->setIgnoreWatched(F2B(val));
            }
            if (key == "nsfilter-user")
            {
                filter->setIgnore_UserSpace(F2B(val));
                continue;
            }
            if (key == "required-tags")
            {
                filter->SetRequiredTags_CommaSeparated(val);
                continue;
            }
            if (key == "ignored-tags")
            {
                filter->SetIgnoredTags_CommaSeparated(val);
                continue;
            }
            if (key == "required-categories")
            {
                filter->SetRequiredCategories_CommaSeparated(val);
                continue;
            }
            if (key == "ignored-categories")
            {
                filter->SetIgnoredCategories_CommaSeparated(val);
                continue;
            }
        }
    }
    return ReturnValue;
}

static QList<ScoreWord> ParseScoreWords_YAML(YAML::Node &node, const QString& key)
{
    QList<ScoreWord> results;
    QHash<QString, QVariant> score_words = HuggleParser::YAML2QHash(key, node, QHash<QString, QVariant>());
    QList<QString> s_keys = score_words.keys();
    foreach (QString score_str, s_keys)
    {
        int score = score_str.toInt();
        QStringList words = score_words[score_str].toStringList();
        foreach (QString w, words)
        {
            results.append(ScoreWord(w, score));
        }
    }
    return results;
}

void HuggleParser::ParsePatterns_yaml(YAML::Node &node, WikiSite *site)
{
    site->ProjectConfig->ScoreParts.clear();
    site->ProjectConfig->ScoreParts = ParseScoreWords_YAML(node, "score-parts");
}

void HuggleParser::ParseWords_yaml(YAML::Node &node, WikiSite *site)
{
    site->ProjectConfig->ScoreWords.clear();
    site->ProjectConfig->ScoreWords = ParseScoreWords_YAML(node, "score-words");
}

void HuggleParser::ParseNoTalkWords_yaml(YAML::Node &node, WikiSite *site)
{
    site->ProjectConfig->NoTalkScoreWords.clear();
    site->ProjectConfig->NoTalkScoreWords = ParseScoreWords_YAML(node, "score-words-no-talk");
}

void HuggleParser::ParseNoTalkPatterns_yaml(YAML::Node &node, WikiSite *site)
{
    site->ProjectConfig->NoTalkScoreParts.clear();
    site->ProjectConfig->NoTalkScoreParts = ParseScoreWords_YAML(node, "score-parts-no-talk");
}
