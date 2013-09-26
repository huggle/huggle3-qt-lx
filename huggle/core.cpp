//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "core.h"

#ifdef PYTHONENGINE
PythonEngine *Core::Python = NULL;
#endif

// definitions
// This needs to be moved to resource file

QString Core::HtmlHeader = "<html><head><style type=\"text/css\">"\
        "table.diff{background:white}td.diff-otitle{background:#ffffff}td.diff-ntitle{background:#ffffff}"\
        "td.diff-addedline{background:#ccffcc;font-size:smaller;border:solid 2px black}"\
        "td.diff-deletedline{background:#ffffaa;font-size:smaller;border:dotted 2px black}"\
        "td.diff-context{background:#eeeeee;font-size:smaller}.diffchange{color:red;font-weight:bold;text-decoration:underline}"\
        "</style></head><body><table class='diff diff-contentalign-left'>";
QString Core::HtmlFooter = "</table></body></html>";

MainWindow *Core::Main = NULL;
Login *Core::f_Login = NULL;
HuggleFeed *Core::SecondaryFeedProvider = NULL;
HuggleFeed *Core::PrimaryFeedProvider = NULL;
QList<QString> *Core::RingLog = new QList<QString>();
QList<Query*> Core::RunningQueries;
QList<WikiEdit*> Core::ProcessingEdits;
QList<WikiEdit*> Core::ProcessedEdits;
ProcessorThread *Core::Processor = NULL;
QList<Message*> Core::Messages;
QList<EditQuery*> Core::PendingMods;
QDateTime Core::StartupTime = QDateTime::currentDateTime();
bool Core::Running = true;
QList<iExtension*> Core::Extensions;
WikiPage *Core::AIVP = NULL;

void Core::Init()
{
    QFile vf(":/huggle/git/version.txt");
    vf.open(QIODevice::ReadOnly);
    QString version(vf.readAll());
    Configuration::HuggleVersion += " " + version;
    vf.close();
    Core::Log("Huggle 3 QT-LX, version " + Configuration::HuggleVersion);
    Core::Log("Loading configuration");
    Processor = new ProcessorThread();
    Processor->start();
    Core::LoadConfig();
    Core::DebugLog("Loading defs");
    Core::LoadDefs();
    Configuration::LocalConfig_IgnorePatterns.append("/sandbox");
    Configuration::LocalConfig_IgnorePatterns.append("/Sandbox");
    Configuration::LocalConfig_RevertSummaries.append("Test edits;Reverted edits by [[Special:Contributions/$1|$1]] identified as test edits");
#ifdef PYTHONENGINE
    Core::Log("Loading python engine");
    Core::Python = new PythonEngine();
#endif
    Core::DebugLog("Loading wikis");
    Core::LoadDB();
    Core::Log("Loaded in " + QString::number(Core::StartupTime.msecsTo(QDateTime::currentDateTime())));
}

void Core::LoadDB()
{
    Configuration::ProjectList << Configuration::Project;
    // this is a temporary only for oauth testing
    //Configuration::ProjectList << WikiSite("testwiki", "test.wikipedia.org/");
    //Configuration::ProjectList << WikiSite("mediawiki","www.mediawiki.org/");
    QString text = "";
    if (QFile::exists(Configuration::WikiDB))
    {
        QFile db(Configuration::WikiDB);
        if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            Core::Log("ERROR: Unable to read " + Configuration::WikiDB);
            return;
        }
        text = QString(db.readAll());
    }

    if (text == "")
    {
        QFile vf(":/huggle/resources/Resources/Definitions.txt");
        vf.open(QIODevice::ReadOnly);
        text = QString(vf.readAll());
    }

    QDomDocument d;
    d.setContent(text);
}

bool Core::SafeBool(QString value, bool defaultvalue)
{
    if (value.toLower() == "true")
    {
        return true;
    }
    return defaultvalue;
}

QStringList Core::ConfigurationParse_QL(QString key, QString content, bool CS)
{
    QStringList list;
    if (content.startsWith(key + ":"))
    {
        QString value = content.mid(key.length() + 1);
        QStringList lines = value.split("\n");
        int curr = 1;
        while (curr < lines.count())
        {
            QString _line = Core::Trim(lines.at(curr));
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
                    if (Core::Trim(xx.at(i2)) != "")
                    {
                        f.append(Core::Trim(xx.at(i2)));
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
            QString _line = Core::Trim(lines.at(curr));
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
                    if (Core::Trim(xx.at(i2)) != "")
                    {
                        f.append(Core::Trim(xx.at(i2)));
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

QString Core::Trim(QString text)
{
    while (text.startsWith(" "))
    {
        if (text == "")
        {
            break;
        }
        text = text.mid(1);
    }

    return text;
}

void Core::DeleteEdit(WikiEdit *edit)
{
    if (edit == NULL)
    {
        return;
    }

    if (edit->Previous != NULL && edit->Next != NULL)
    {
        edit->Previous->Next = edit->Next;
        edit->Next->Previous = edit->Previous;
        delete edit;
        return;
    }

    if (edit->Previous != NULL)
    {
        edit->Previous->Next = NULL;
    }

    if (edit->Next != NULL)
    {
        edit->Next->Previous = NULL;
    }

    delete edit;
}

QString Core::GetSummaryOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::LocalConfig_RevertSummaries.count())
    {
        QString line = Configuration::LocalConfig_RevertSummaries.at(id);
        if (line.startsWith(key + ";"))
        {
            return Core::GetValueFromKey(line);
        }
        id++;
    }
    return Configuration::DefaultRevertSummary;
}

QString Core::GetNameOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::LocalConfig_WarningTypes.count())
    {
        QString line = Configuration::LocalConfig_WarningTypes.at(id);
        if (line.startsWith(key) + ";")
        {
            return Core::GetValueFromKey(line);
        }
        id++;
    }
    return key;
}

QString Core::GetKeyOfWarningTypeFromWarningName(QString id)
{
    int i=0;
    while (i<Configuration::LocalConfig_WarningTypes.count())
    {
        QString line = Configuration::LocalConfig_WarningTypes.at(i);
        if (line.endsWith(id) || line.endsWith(id + ","))
        {
            return Core::GetKeyFromValue(line);
        }
        i++;
    }
    return id;
}

void Core::ParsePats(QString text)
{
    Configuration::LocalConfig_ScoreParts.clear();
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
                QString w = Core::Trim(items.at(CurrentItem));
                if (w == "")
                {
                    CurrentItem++;
                    continue;
                }
                word.append(w);
                CurrentItem++;
            }
            if (!l.endsWith(",") || Core::Trim(l) == "")
            {
                break;
            }
            line++;
        }

        line = 0;
        while (line < word.count())
        {
            Configuration::LocalConfig_ScoreParts.append(ScoreWord(word.at(line), score));
            line++;
        }
    }
}

void Core::SaveDefs()
{
    QFile file(Configuration::GetConfigurationPath() + "users.xml");
    if (QFile(Configuration::GetConfigurationPath() + "users.xml").exists())
    {
        QFile(Configuration::GetConfigurationPath() + "users.xml").copy(Configuration::GetConfigurationPath() + "users.xml~");
        QFile(Configuration::GetConfigurationPath() + "users.xml").remove();
    }
    if (!file.open(QIODevice::WriteOnly))
    {
        Core::Log("ERROR: can't open " + Configuration::GetConfigurationPath() + "users.xml");
        return;
    }
    QString xx = "<definitions>\n";
    int x = 0;
    while (x<WikiUser::ProblematicUsers.count())
    {
        xx += "<user name=\"" + WikiUser::ProblematicUsers.at(x)->Username + "\" badness=\"" +
                QString::number(WikiUser::ProblematicUsers.at(x)->BadnessScore) +"\"></user>\n";
        x++;
    }
    xx += "</definitions>";
    file.write(xx.toUtf8());
    file.close();
    QFile().remove(Configuration::GetConfigurationPath() + "users.xml~");
}

QString Core::GetValueFromKey(QString item)
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

QString Core::GetKeyFromValue(QString item)
{
    if (item.contains(";"))
    {
        QString type = item.mid(0, item.indexOf(";"));
        return type;
    }
    return item;
}

void Core::ParseWords(QString text)
{
    Configuration::LocalConfig_ScoreWords.clear();
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
                QString w = Core::Trim(items.at(CurrentItem));
                if (w == "")
                {
                    CurrentItem++;
                    continue;
                }
                word.append(w);
                CurrentItem++;
            }
            if (!l.endsWith(",") || Core::Trim(l) == "")
            {
                break;
            }
            line++;
        }

        line = 0;
        while (line < word.count())
        {
            Configuration::LocalConfig_ScoreWords.append(ScoreWord(word.at(line), score));
            line++;
        }
    }
}

Message *Core::MessageUser(WikiUser *user, QString message, QString title, QString summary, bool section, Query *dependency)
{
    if (user == NULL)
    {
        Core::Log("Cowardly refusing to message NULL user");
        return NULL;
    }

    Message *m = new Message(user, message, summary);
    m->title = title;
    m->Dependency = dependency;
    Core::Messages.append(m);
    m->Send();
    Core::Log("Sending message to user " + user->Username);

    return m;
}

void Core::LoadDefs()
{
    QFile defs(Configuration::GetConfigurationPath() + "users.xml");
    if (QFile(Configuration::GetConfigurationPath() + "users.xml~").exists())
    {
        Core::Log("WARNING: recovering definitions from last session");
        QFile(Configuration::GetConfigurationPath() + "users.xml").remove();
        if (QFile(Configuration::GetConfigurationPath()
                   + "users.xml~").copy(Configuration::GetConfigurationPath()
                   + "users.xml"))
        {
            QFile().remove(Configuration::GetConfigurationPath() + "users.xml~");
        } else
        {
            Core::Log("WARNING: Unable to recover the definitions");
        }
    }
    if (!defs.exists())
    {
        return;
    }
    defs.open(QIODevice::ReadOnly);
    QString Contents(defs.readAll());
    QDomDocument list;
    list.setContent(Contents);
    QDomNodeList l = list.elementsByTagName("user");
    if (l.count() > 0)
    {
        int i=0;
        while (i<l.count())
        {
            WikiUser *user;
            QDomElement e = l.at(i).toElement();
            if (!e.attributes().contains("name"))
            {
                i++;
                continue;
            }
            user = new WikiUser();
            user->Username = e.attribute("name");
            if (e.attributes().contains("badness"))
            {
                user->BadnessScore = e.attribute("badness").toInt();
            }
            WikiUser::ProblematicUsers.append(user);
            i++;
        }
    }
    defs.close();
}

void Core::FinalizeMessages()
{
    if (Core::Messages.count() < 1)
    {
        return;
    }
    int x=0;
    QList<Message*> list;
    while (x<Core::Messages.count())
    {
        if (Core::Messages.at(x)->Finished())
        {
            list.append(Core::Messages.at(x));
        }
        x++;
    }
    x=0;
    while (x<list.count())
    {
        Core::Messages.removeOne(list.at(x));
        x++;
    }
}

int Core::GetLevel(QString page)
{
    int level = 4;
    while (level > 0)
    {
        int xx=0;
        while (xx<Configuration::LocalConfig_WarningDefs.count())
        {
            QString defs=Configuration::LocalConfig_WarningDefs.at(xx);
            if (Core::GetKeyFromValue(defs).toInt() == level)
            {
                if (page.contains(Core::GetValueFromKey(defs)))
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

QString Core::RetrieveTemplateToWarn(QString type)
{
    int x=0;
    while (x < Configuration::LocalConfig_WarningTemplates.count())
    {
        if (Core::GetKeyFromValue(Configuration::LocalConfig_WarningTemplates.at(x)) == type)
        {
            return Core::GetValueFromKey(Configuration::LocalConfig_WarningTemplates.at(x));
        }
        x++;
    }
    return "";
}

EditQuery *Core::EditPage(WikiPage *page, QString text, QString summary, bool minor)
{
    if (page == NULL)
    {
        return NULL;
    }
    // retrieve a token
    EditQuery *_e = new EditQuery();
    if (!summary.endsWith(Configuration::EditSuffixOfHuggle))
    {
        summary = summary + " " + Configuration::EditSuffixOfHuggle;
    }
    _e->page = page->PageName;
    Core::PendingMods.append(_e);
    _e->text = text;
    _e->summary = summary;
    _e->Process();
    return _e;
}

void Core::AppendQuery(Query *item)
{
    item->Consumers.append("core");
    Core::RunningQueries.append(item);
}

void Core::Log(QString Message)
{
    std::cout << Message.toStdString() << std::endl;
    Core::InsertToRingLog(Message);
    if (Core::Main != NULL)
    {
        Core::Main->lUnwrittenLogs.lock();
        Core::Main->UnwrittenLogs.append(Message);
        Core::Main->lUnwrittenLogs.unlock();
    }
}

void Core::DebugLog(QString Message, unsigned int Verbosity)
{
    if (Configuration::Verbosity >= Verbosity)
    {
        Core::Log("DEBUG[" + QString::number(Verbosity) + "]: " + Message);
    }
}

QString Core::GetProjectURL(WikiSite Project)
{
    return Configuration::GetURLProtocolPrefix() + Project.URL;
}

QString Core::GetProjectWikiURL(WikiSite Project)
{
    return Core::GetProjectURL(Project) + Project.LongPath;
}

QString Core::GetProjectScriptURL(WikiSite Project)
{
    return Core::GetProjectURL(Project) + Project.ScriptPath;
}

QString Core::GetProjectURL()
{
    return Configuration::GetURLProtocolPrefix() + Configuration::Project.URL;
}

QString Core::GetProjectWikiURL()
{
    return Core::GetProjectURL(Configuration::Project) + Configuration::Project.LongPath;
}

QString Core::GetProjectScriptURL()
{
    return Core::GetProjectURL(Configuration::Project) + Configuration::Project.ScriptPath;
}

void Core::ProcessEdit(WikiEdit *e)
{
    Core::Main->ProcessEdit(e);
}

void Core::Shutdown()
{
    Running = false;
    // grace time for subthreads to finish
    if (Core::Main != NULL)
    {
        Core::Main->hide();
    }
#if QT_VERSION >= 0x050000
    QThread::usleep(200000);
#endif
    if (Processor->isRunning())
    {
        Processor->exit();
    }
    //delete Processor;
    Processor = NULL;
    Core::SaveDefs();
    Core::SaveConfig();
#ifdef PYTHONENGINE
    Core::Log("Unloading python");
    delete Core::Python;
#endif
    delete Core::f_Login;
    Core::f_Login = NULL;
    QApplication::quit();
}

QString Core::RingLogToText()
{
    int i = 0;
    QString text = "";
    while (i<Core::RingLog->size())
    {
        text = Core::RingLog->at(i) + "\n" + text;
        i++;
    }
    return text;
}

void Core::InsertToRingLog(QString text)
{
    if (Core::RingLog->size()+1 > Configuration::RingLogMaxSize)
    {
        Core::RingLog->removeAt(0);
    }
    Core::RingLog->append(text);
}

void Core::DeveloperError()
{
    QMessageBox *mb = new QMessageBox();
    mb->setWindowTitle("Function is restricted now");
    mb->setText("You can't perform this action in developer mode, because you aren't logged into the wiki");
    mb->exec();
    delete mb;
}

void Core::LoadConfig()
{
    QFile file(Configuration::GetConfigurationPath() + "huggle.xml");
    if (!file.exists())
    {
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        Core::DebugLog("Unable to read config file");
        return;
    }
    QDomDocument conf;
    conf.setContent(file.readAll());
}

void Core::PreProcessEdit(WikiEdit *_e)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit");
    }

    if (_e->Status == StatusProcessed)
    {
        return;
    }

    if (_e->User == NULL)
    {
        throw new Exception("Edit user was NULL in Core::PreProcessEdit");
    }

    _e->Whitelisted = Configuration::WhiteList.contains(_e->User->Username);
    _e->EditMadeByHuggle = _e->Summary.contains(Configuration::EditSuffixOfHuggle);

    _e->Status = StatusProcessed;
}

void Core::SaveConfig()
{
    QFile file(Configuration::GetConfigurationPath() + QDir::separator() + "huggle3.xml");
    if (!file.open(QIODevice::WriteOnly))
    {
        Core::Log("Unable to save configuration because the file can't be open");
        return;
    }
    QXmlStreamWriter *x = new QXmlStreamWriter();
    x->setDevice(&file);
    x->writeStartDocument();
    Core::InsertConfig("Cache_InfoSize", QString::number(Configuration::Cache_InfoSize), x);
    Core::InsertConfig("DefaultRevertSummary", Configuration::DefaultRevertSummary, x);
    x->writeEndDocument();
    delete x;
}

void Core::PostProcessEdit(WikiEdit *_e)
{
    _e->PostProcess();
    Core::ProcessingEdits.append(_e);
}

void Core::CheckQueries()
{
    int curr = 0;
    if (Core::PendingMods.count() > 0)
    {
        while (curr < Core::PendingMods.count())
        {
            if (Core::PendingMods.at(curr)->Processed())
            {
                Core::PendingMods.removeAt(curr);
            } else
            {
                curr++;
            }
        }
    }
    curr = 0;
    if (Core::RunningQueries.count() < 1)
    {
        return;
    }
    QList<Query*> Finished;
    while (curr < Core::RunningQueries.count())
    {
        Query *q = Core::RunningQueries.at(curr);
        Core::Main->Queries->UpdateQuery(q);
        if (q->Processed())
        {
            Finished.append(q);
            Core::DebugLog("Query finished with: " + q->Result->Data, 6);
            if (q->QueryTypeToString() == "ApiQuery (rollback)")
            {
                q->CustomStatus = Core::GetCustomRevertStatus(q->Result->Data);
                if (q->CustomStatus != "Reverted")
                {
                    q->Result->Failed = true;
                } else
                {
                    HistoryItem item;
                    item.Target = ((ApiQuery*)q)->Target;
                    item.Type = HistoryRollback;
                    item.Result = "Success";
                    if (Core::Main != NULL)
                    {
                        Core::Main->_History->Prepend(item);
                    }
                }
            }
            Core::Main->Queries->UpdateQuery(q);
            Core::Main->Queries->RemoveQuery(q);
        }
        curr++;
    }
    curr = 0;
    while (curr < Finished.count())
    {
        Query *item = Finished.at(curr);
        Core::RunningQueries.removeOne(item);
        item->Consumers.removeAll("core");
        item->SafeDelete();
        curr++;
    }
}

bool Core::PreflightCheck(WikiEdit *_e)
{
    if (Configuration::WarnUserSpaceRoll && _e->Page->IsUserpage())
    {
        QMessageBox::StandardButton q = QMessageBox::question(NULL, "Revert edit"
                      , "This page is in userspace, so even if it looks like it is a vandalism,"\
                      " it may not be, are you sure you want to revert it?"
                      , QMessageBox::Yes|QMessageBox::No);
        if (q == QMessageBox::No)
        {
            return false;
        }
    }
    return true;
}

ApiQuery *Core::RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep)
{
    if (_e->User == NULL)
    {
        throw new Exception("Object user was NULL in Core::Revert");
    }
    ApiQuery *query = new ApiQuery();
    if (_e->Page == NULL)
    {
        throw new Exception("Object page was NULL");
    }

    if (summary == "")
    {
        summary = Configuration::GetDefaultRevertSummary(_e->User->Username);
    }

    if (summary.contains("$1"))
    {
        summary = summary.replace("$1", _e->User->Username);
    }

    _e->User->BadnessScore += 200;
    WikiUser::UpdateUser(_e->User);

    if (rollback)
    {
        if (_e->RollbackToken == "")
        {
            Log("ERROR, unable to rollback, because the rollback token was empty: " + _e->Page->PageName);
            delete query;
            return NULL;
        }
        query->SetAction(ActionRollback);
        QString token = _e->RollbackToken;
        if (_e->RollbackToken.endsWith("+\\"))
        {
            //token = token.mid(0, token.indexOf("+")) + "%2B\\";
            token = QUrl::toPercentEncoding(token);
        }
        query->Parameters = "title=" + QUrl::toPercentEncoding(_e->Page->PageName)
                + "&token=" + token
                + "&user=" + QUrl::toPercentEncoding(_e->User->Username)
                + "&summary=" + QUrl::toPercentEncoding(summary);
        query->Target = _e->Page->PageName;
        query->UsingPOST = true;
        if (keep)
        {
            query->Consumers.append("keep");
        }
        Core::AppendQuery(query);
        DebugLog("Rolling back " + _e->Page->PageName);
        query->Process();
    }

    return query;
}

QString Core::GetCustomRevertStatus(QString RevertData)
{
    QDomDocument d;
    d.setContent(RevertData);
    QDomNodeList l = d.elementsByTagName("error");
    if (l.count() > 0)
    {
        if (l.at(0).toElement().attributes().contains("code"))
        {
            QString Error = "";
            Error = l.at(0).toElement().attribute("code");

            if (Error == "alreadyrolled")
            {
                return "Reverted by someone else - skip";
            }

            return "In error (" + Error +")";
        }
    }
    return "Reverted";
}

bool Core::ParseGlobalConfig(QString config)
{
    Configuration::GlobalConfig_EnableAll = Core::SafeBool(Core::ConfigurationParse("enable-all", config));
    QString temp = Core::ConfigurationParse("documentation", config);
    if (temp != "")
    {
        Configuration::GlobalConfig_DocumentationPath = temp;
    }
    temp = Core::ConfigurationParse("feedback", config);
    if (temp != "")
    {
        Configuration::GlobalConfig_FeedbackPath = temp;
    }
    Configuration::GlobalConfigWasLoaded = true;
    return true;
}

bool Core::ParseLocalConfig(QString config)
{
    Configuration::LocalConfig_AIV = Core::SafeBool(Core::ConfigurationParse("aiv-reports", config));
    Configuration::LocalConfig_EnableAll = Core::SafeBool(Core::ConfigurationParse("enable-all", config));
    Configuration::LocalConfig_RequireAdmin = Core::SafeBool(Core::ConfigurationParse("require-admin", config));
    Configuration::LocalConfig_RequireRollback = Core::SafeBool(Core::ConfigurationParse("require-rollback", config));
    Configuration::LocalConfig_UseIrc = Core::SafeBool(Core::ConfigurationParse("irc", config));
    Configuration::LocalConfig_Ignores = Core::ConfigurationParse_QL("ignore", config, true);
    Configuration::LocalConfig_IPScore = Core::ConfigurationParse("score-ip", config, "800").toInt();
    Configuration::LocalConfig_ScoreFlag = Core::ConfigurationParse("score-flag", config).toInt();
    Configuration::LocalConfig_WarnSummary = Core::ConfigurationParse("warn-summary", config);
    Configuration::LocalConfig_WarnSummary2 = Core::ConfigurationParse("warn-summary-2", config);
    Configuration::LocalConfig_WarnSummary3 = Core::ConfigurationParse("warn-summary-3", config);
    Configuration::LocalConfig_WarnSummary4 = Core::ConfigurationParse("warn-summary-4", config);
    Configuration::LocalConfig_RevertSummaries = Core::ConfigurationParse_QL("template-summ", config);
    Configuration::LocalConfig_WarningTypes = Core::ConfigurationParse_QL("warning-types", config);
    Configuration::LocalConfig_WarningDefs = Core::ConfigurationParse_QL("warning-template-tags", config);
    Configuration::LocalConfig_BotScore = Core::ConfigurationParse("score-bot", config, "-200000").toInt();
    Configuration::LocalConfig_ReportPath = Core::ConfigurationParse("aiv", config);
    Configuration::LocalConfig_AIVExtend = Core::SafeBool(Core::ConfigurationParse("aiv-extend", config));
    Configuration::LocalConfig_ReportSt = Core::ConfigurationParse("aiv-section", config).toInt();
    Configuration::LocalConfig_IPVTemplateReport = Core::ConfigurationParse("aiv-ip", config);
    Configuration::LocalConfig_RUTemplateReport = Core::ConfigurationParse("aiv-user", config);
    Core::AIVP = new WikiPage(Configuration::LocalConfig_ReportPath);
    Core::ParsePats(config);
    Core::ParseWords(config);

    // templates
    int CurrentTemplate=0;
    while (CurrentTemplate<Configuration::LocalConfig_WarningTypes.count())
    {
        QString type = Core::GetKeyFromValue(Configuration::LocalConfig_WarningTypes.at(CurrentTemplate));
        int CurrentWarning = 1;
        while (CurrentWarning <= 4)
        {
            QString xx = Core::ConfigurationParse(type + QString::number(CurrentWarning), config);
            if (xx != "")
            {
                Configuration::LocalConfig_WarningTemplates.append(type + QString::number(CurrentWarning) + ";" + xx);
            }
            CurrentWarning++;
        }
        CurrentTemplate++;
    }
    // sanitize
    if (Configuration::LocalConfig_ReportPath == "")
    {
        Configuration::LocalConfig_AIV = false;
    }
    return true;
}

bool Core::ParseUserConfig(QString config)
{
    Configuration::LocalConfig_EnableAll = Core::SafeBool(Core::ConfigurationParse("enable", config));
    return true;
}

QString Core::ConfigurationParse(QString key, QString content, QString missing)
{
    if (content.startsWith(key + ":"))
    {
        QString value = content.mid(key.length() + 1);
        if (value.contains("\n"))
        {
            value = value.mid(0, value.indexOf("\n"));
        }
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
        return value;
    }
    return missing;
}

void Core::InsertConfig(QString key, QString value, QXmlStreamWriter *s)
{
    s->writeStartElement("local");
    s->writeAttribute(key, value);
    s->writeEndElement();
}
