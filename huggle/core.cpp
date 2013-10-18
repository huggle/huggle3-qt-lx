//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "core.h"

using namespace Huggle;

// definitions
#ifdef PYTHONENGINE
PythonEngine *Core::Python = NULL;
#endif

QString Core::HtmlHeader = "";
QString Core::HtmlFooter = "</table></body></html>";

MainWindow *Core::Main = NULL;
Login *Core::f_Login = NULL;
HuggleFeed *Core::SecondaryFeedProvider = NULL;
HuggleFeed *Core::PrimaryFeedProvider = NULL;
QStringList Core::RingLog;
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
QList<Language*> Core::LocalizationData;
QList<HuggleQueueFilter *> Core::FilterDB;

void Core::Init()
{
    if (Configuration::_SafeMode)
    {
        Core::Log("DEBUG: Huggle is running in a safe mode");
    }
    Core::VersionRead();
    QFile *vf;
#if QT_VERSION >= 0x050000
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#else
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    vf = new QFile(":/huggle/resources/Resources/Header.txt");
    vf->open(QIODevice::ReadOnly);
    Core::HtmlHeader = QString(vf->readAll());
    vf->close();
    delete vf;
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
    Core::LoadLocalizations();
    if (!Configuration::_SafeMode)
    {
        Core::Log("Loading plugins");
        Core::ExtensionLoad();
    } else
    {
        Core::Log("Not loading plugins in a safe mode");
    }
    Core::Log("Loaded in " + QString::number(Core::StartupTime.msecsTo(QDateTime::currentDateTime())));
}

void Core::LoadDB()
{
    Configuration::ProjectList.clear();
    Configuration::ProjectList << &Configuration::Project;
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
        db.close();
    }

    if (text == "")
    {
        QFile vf(":/huggle/resources/Resources/Definitions.txt");
        vf.open(QIODevice::ReadOnly);
        text = QString(vf.readAll());
        vf.close();
    }

    QDomDocument d;
    d.setContent(text);
    QDomNodeList list = d.elementsByTagName("wiki");
    int xx=0;
    while (xx < list.count())
    {
        QDomElement e = list.at(xx).toElement();
        if (!e.attributes().contains("name"))
        {
            continue;
        }
        if (!e.attributes().contains("url"))
        {
            continue;
        }
        WikiSite *site = new WikiSite(e.attribute("name"), e.attribute("url"));
        site->IRCChannel = "";
        site->SupportOAuth = false;
        site->SupportHttps = false;
        site->WhiteList = "test";
        // name="testwiki" url="test.wikipedia.org/" path="wiki/" script="w/" https="true" oauth="true" channel="#test.wikipedia" wl="test
        if (e.attributes().contains("path"))
        {
            site->LongPath = e.attribute("path");
        }
        if (e.attributes().contains("wl"))
        {
            site->WhiteList = e.attribute("wl");
        }
        if (e.attributes().contains("script"))
        {
            site->ScriptPath = e.attribute("script");
        }
        if (e.attributes().contains("https"))
        {
            site->SupportHttps = Core::SafeBool(e.attribute("https"));
        }
        if (e.attributes().contains("oauth"))
        {
            site->SupportOAuth = Core::SafeBool(e.attribute("oauth"));
        }
        if (e.attributes().contains("channel"))
        {
            site->IRCChannel = e.attribute("channel");
        }
        Configuration::ProjectList.append(site);
        xx++;
    }
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
    m->Section = section;
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
    if (Configuration::TrimOldWarnings)
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
                if (QDate::currentDate().addDays(Configuration::LocalConfig_TemplateAge) > date)
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
    item->RegisterConsumer("core");
    Core::RunningQueries.append(item);
}

void Core::Log(QString Message)
{
    Message = "<" + QDateTime::currentDateTime().toString() + "> " + Message;
    std::cout << Message.toStdString() << std::endl;
    Core::InsertToRingLog(Message);
    if (Core::Main != NULL)
    {
        Core::Main->lUnwrittenLogs.lock();
        Core::Main->UnwrittenLogs.append(Message);
        Core::Main->lUnwrittenLogs.unlock();
    }
}

void Core::ExtensionLoad()
{
    if (QDir().exists(EXTENSION_PATH))
    {
        QDir d(EXTENSION_PATH);
        QStringList extensions = d.entryList();
        int xx = 0;
        while (xx < extensions.count())
        {
            QString name = extensions.at(xx).toLower();
            if (name.endsWith(".so") || name.endsWith(".dll"))
            {
                name = QString(EXTENSION_PATH) + QDir::separator() + extensions.at(xx);
                QPluginLoader *extension = new QPluginLoader(name);
                if (extension->load())
                {
                    QObject* root = extension->instance();
                    if (root)
                    {
                        iExtension *interface = qobject_cast<iExtension*>(root);
                        if (!interface)
                        {
                            Core::Log("Unable to cast the library to extension");
                        }else
                        {
                            if (interface->Register())
                            {
                                Core::Extensions.append(interface);
                                Core::Log("Successfully loaded: " + extensions.at(xx));
                            }
                            else
                            {
                                Core::Log("Unable to register: " + extensions.at(xx));
                            }
                        }
                    }
                } else
                {
                    Core::Log("Failed to load (reason: " + extension->errorString() + "): " + extensions.at(xx));
                    delete extension;
                }
            }
            xx++;
        }
    } else
    {
        Core::Log("There is no extensions folder, skipping load");
    }
    Core::Log("Extensions: " + QString::number(Core::Extensions.count()));
}

void Core::VersionRead()
{
    QFile *vf = new QFile(":/huggle/git/version.txt");
    vf->open(QIODevice::ReadOnly);
    QString version(vf->readAll());
    version = version.replace("\n", "");
    Configuration::HuggleVersion += " " + version;
#ifdef PRODUCTION_BUILD
    Configuration::HuggleVersion += " production";
#endif
    vf->close();
    delete vf;
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
    while (i<Core::RingLog.size())
    {
        text = Core::RingLog.at(i) + "\n" + text;
        i++;
    }
    return text;
}

QStringList Core::RingLogToQStringList()
{
    return QStringList(Core::RingLog);
}

void Core::InsertToRingLog(QString text)
{
    if (Core::RingLog.size()+1 > Configuration::RingLogMaxSize)
    {
        Core::RingLog.removeAt(0);
    }
    Core::RingLog.append(text);
}

void Core::DeveloperError()
{
    QMessageBox *mb = new QMessageBox();
    mb->setWindowTitle("Function is restricted now");
    mb->setText("You can't perform this action in developer mode, because you aren't logged into the wiki");
    mb->exec();
    delete mb;
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

    _e->EditMadeByHuggle = _e->Summary.contains(Configuration::EditSuffixOfHuggle);

    _e->Status = StatusProcessed;
}

void Core::LoadConfig()
{
    QFile file(Configuration::GetConfigurationPath() + "huggle.xml");
    Core::Log("Home: " + Configuration::GetConfigurationPath());
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
    QDomNodeList l = conf.elementsByTagName("local");
    int item = 0;
    while (item < l.count())
    {
        QDomElement option = l.at(item).toElement();
        QDomNamedNodeMap xx = option.attributes();
        if (!xx.contains("text") || !xx.contains("key"))
        {
            continue;
        }
        if (option.attribute("key") == "DefaultRevertSummary")
        {
            Configuration::DefaultRevertSummary = option.attribute("text");
            item++;
            continue;
        }
        if (option.attribute("key") == "Cache_InfoSize")
        {
            Configuration::Cache_InfoSize = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (option.attribute("key") == "GlobalConfigurationWikiAddress")
        {
            Configuration::GlobalConfigurationWikiAddress = option.attribute("text");
            item++;
            continue;
        }
        if (option.attribute("key") == "IRCIdent")
        {
            Configuration::IRCIdent = option.attribute("text");
            item++;
            continue;
        }
        if (option.attribute("key") == "IRCNick")
        {
            Configuration::IRCNick = option.attribute("text");
            item++;
            continue;
        }
        if (option.attribute("key") == "IRCPort")
        {
            Configuration::IRCPort = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (option.attribute("key") == "IRCServer")
        {
            Configuration::IRCServer = option.attribute("text");
            item++;
            continue;
        }
        if (option.attribute("key") == "Language")
        {
            Configuration::Language = option.attribute("text");
            item++;
            continue;
        }
        if (option.attribute("key") == "ProviderCache")
        {
            Configuration::ProviderCache = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (option.attribute("key") == "AskUserBeforeReport")
        {
            Configuration::AskUserBeforeReport = Core::SafeBool(option.attribute("text"));
            item++;
            continue;
        }
        if (option.attribute("key") == "HistorySize")
        {
            Configuration::HistorySize = option.attribute("text").toInt();
            item++;
            continue;
        }
        if (option.attribute("key") == "Layout_Geom")
        {
            Configuration::Geometry = option.attribute("text").toUtf8();
            item++;
            continue;
        }
        if (option.attribute("key") == "Position")
        {
            Configuration::Position = option.attribute("text").toUtf8();
            item++;
            continue;
        }
        item++;
    }
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
    if (Configuration::Position != "")
    {
        Core::InsertConfig("Position", QString(Configuration::Position), x);
    }
    if (Configuration::Geometry != "")
    {
        Core::InsertConfig("Layout_Geom", QString(Configuration::Geometry), x);
    }
    Core::InsertConfig("Cache_InfoSize", QString::number(Configuration::Cache_InfoSize), x);
    Core::InsertConfig("DefaultRevertSummary", Configuration::DefaultRevertSummary, x);
    Core::InsertConfig("GlobalConfigurationWikiAddress", Configuration::GlobalConfigurationWikiAddress, x);
    Core::InsertConfig("IRCIdent", Configuration::IRCIdent, x);
    Core::InsertConfig("IRCNick", Configuration::IRCNick, x);
    Core::InsertConfig("IRCPort", QString::number(Configuration::IRCPort), x);
    Core::InsertConfig("IRCServer", Configuration::IRCServer, x);
    Core::InsertConfig("Language", Configuration::Language, x);
    Core::InsertConfig("ProviderCache", QString::number(Configuration::ProviderCache), x);
    Core::InsertConfig("AskUserBeforeReport", Configuration::Bool2String(Configuration::AskUserBeforeReport), x);
    Core::InsertConfig("HistorySize", QString::number(Configuration::HistorySize), x);
    Core::InsertConfig("NextOnRv", Configuration::Bool2String(Configuration::NextOnRv), x);
    Core::InsertConfig("QueueNewEditsUp", Configuration::Bool2String(Configuration::QueueNewEditsUp), x);
    Core::InsertConfig("RingLogMaxSize", QString::number(Configuration::RingLogMaxSize), x);
    Core::InsertConfig("TrimOldWarnings", Configuration::Bool2String(Configuration::TrimOldWarnings), x);
    Core::InsertConfig("WarnUserSpaceRoll", Configuration::Bool2String(Configuration::WarnUserSpaceRoll), x);
    Core::InsertConfig("UserName", Configuration::UserName, x);
    x->writeEndDocument();
    delete x;
}

void Core::PostProcessEdit(WikiEdit *_e)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit in PostProcessEdit(WikiEdit *_e) is not a valid edit");
    }
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
                    Core::Log("Unable to revert " + ((ApiQuery*)q)->Target + ": " + q->CustomStatus);
                    q->Result->Failed = true;
                    q->Result->ErrorMessage = q->CustomStatus;
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
        item->UnregisterConsumer("core");
        item->SafeDelete();
        curr++;
    }
}

bool Core::PreflightCheck(WikiEdit *_e)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit in PreflightCheck(WikiEdit *_e) is not a valid edit");
    }
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
    if (_e == NULL)
    {
        throw new Exception("NULL edit in RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep) is not a valid edit");
    }
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
        if (!Configuration::Rights.contains("rollback"))
        {
            Core::Log("You don't have rollback rights");
            delete query;
            return NULL;
        }
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
            query->RegisterConsumer("keep");
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
                return "Edit was reverted by someone else - skipping";
            }

            if (Error == "onlyauthor")
            {
                return "ERROR: Cannot rollback - page only has one author";
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
    Configuration::LocalConfig_WelcomeTypes = Core::ConfigurationParse_QL("welcome-messages", config);
    Configuration::LocalConfig_ReportSummary = Core::ConfigurationParse("report-summary", config);
    Configuration::LocalConfig_RequireEdits = Core::ConfigurationParse("require-edits", config, "0").toInt();
    Configuration::LocalConfig_DeletionTemplates = Core::ConfigurationParse_QL("speedy-options", config);
    Configuration::LocalConfig_TemplateAge = Core::ConfigurationParse("template-age", config, QString::number(Configuration::LocalConfig_TemplateAge)).toInt();
    Core::AIVP = new WikiPage(Configuration::LocalConfig_ReportPath);
    Core::ParsePats(config);
    Core::ParseWords(config);
    QStringList namespaces = Core::ConfigurationParse_QL("namespace-names", config, true);

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
   // Configuration::LocalConfig_Ignores = Core::ConfigurationParse_QL("ignore", config, Configuration::LocalConfig_Ignores);
    Configuration::LocalConfig_IPScore = Core::ConfigurationParse("score-ip", config, QString::number(Configuration::LocalConfig_IPScore)).toInt();
    Configuration::LocalConfig_ScoreFlag = Core::ConfigurationParse("score-flag", config, QString::number(Configuration::LocalConfig_ScoreFlag)).toInt();
    Configuration::LocalConfig_WarnSummary = Core::ConfigurationParse("warn-summary", config, Configuration::LocalConfig_WarnSummary);
    Configuration::LocalConfig_WarnSummary2 = Core::ConfigurationParse("warn-summary-2", config, Configuration::LocalConfig_WarnSummary2);
    Configuration::LocalConfig_WarnSummary3 = Core::ConfigurationParse("warn-summary-3", config, Configuration::LocalConfig_WarnSummary3);
    Configuration::LocalConfig_WarnSummary4 = Core::ConfigurationParse("warn-summary-4", config, Configuration::LocalConfig_WarnSummary4);
    Configuration::LocalConfig_TemplateAge = Core::ConfigurationParse("template-age", config, QString::number(Configuration::LocalConfig_TemplateAge)).toInt();
    QStringList l1 = Core::ConfigurationParse_QL("template-summ", config);
    if (l1.count() > 0)
    {
        Configuration::LocalConfig_TemplateSummary = l1;
    }
    //Configuration::LocalConfig_WarningTypes = Core::ConfigurationParse_QL("warning-types", config);
    //Configuration::LocalConfig_WarningDefs = Core::ConfigurationParse_QL("warning-template-tags", config);
    Configuration::LocalConfig_BotScore = Core::ConfigurationParse("score-bot", config, QString(Configuration::LocalConfig_BotScore)).toInt();
    NormalizeConf();
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
    s->writeAttribute("key", key);
    s->writeAttribute("text", value);
    s->writeEndElement();
}

void Core::ExceptionHandler(Exception *exception)
{
    ExceptionWindow *w = new ExceptionWindow(exception);
    w->exec();
}

Language *Core::MakeLanguage(QString text, QString name)
{
    Core::Log("Loading language: " + name);
    Language *l = new Language(name);
    QStringList keys = text.split("\n");
    int p = 0;
    while (p < keys.count())
    {
        if (keys.at(p).contains(":"))
        {
            QString line = keys.at(p);
            while (line.startsWith(" "))
            {
                line = line.mid(1);
            }
            QString key = line.mid(0, line.indexOf(":"));
            QString lang = line.mid(line.indexOf(":") + 1);
            while (lang.startsWith(" "))
            {
                lang = lang.mid(1);
            }
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

void Core::LocalInit(QString name)
{
    QFile *f = new QFile(":/huggle/text/Localization/" + name + ".txt");
    f->open(QIODevice::ReadOnly);
    Core::LocalizationData.append(Core::MakeLanguage(QString(f->readAll()), name));
    f->close();
    delete f;
}

QString Core::Localize(QString key)
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
    if (Core::LocalizationData.count() > 0)
    {
        int c=0;
        while (c<Core::LocalizationData.count())
        {
            if (Core::LocalizationData.at(c)->LanguageName == Configuration::Language)
            {
                Language *l = Core::LocalizationData.at(c);
                if (l->Messages.contains(id))
                {
                    return l->Messages[id];
                }
                // performance tweak
                break;
            }
            c++;
        }
        if (Core::LocalizationData.at(0)->Messages.contains(id))
        {
            return Core::LocalizationData.at(0)->Messages[id];
        }
    }
    return key;
}

void Core::LoadLocalizations()
{
    Core::LocalInit("en");
    if (Configuration::_SafeMode)
    {
        Core::Log("Skipping load of other languages, because of safe mode");
        return;
    }
    Core::LocalInit("ar");
    Core::LocalInit("bg");
    Core::LocalInit("bn");
    Core::LocalInit("es");
    Core::LocalInit("de");
    Core::LocalInit("fa");
    Core::LocalInit("fr");
    Core::LocalInit("hi");
    Core::LocalInit("it");
    Core::LocalInit("ja");
    Core::LocalInit("ka");
    Core::LocalInit("km");
    Core::LocalInit("kn");
    Core::LocalInit("ko");
    Core::LocalInit("ml");
    Core::LocalInit("mr");
    Core::LocalInit("nl");
    Core::LocalInit("no");
    Core::LocalInit("oc");
    Core::LocalInit("or");
    Core::LocalInit("pt");
    Core::LocalInit("ptb");
    Core::LocalInit("ru");
    Core::LocalInit("sv");
    Core::LocalInit("zh");
}

bool Core::ReportPreFlightCheck()
{
    if (!Configuration::AskUserBeforeReport)
    {
        return true;
    }
    QMessageBox::StandardButton q = QMessageBox::question(NULL, "Report user"
                  , "This user has already reached warning level 4, so no further templates will be "\
                    "delivered to them. You can report them now, but please, make sure that they already reached the proper "\
                    "number of recent warnings! You can do so by clicking the \"talk page\" button in following form. "\
                    "Keep in mind that this form and this warning is displayed no matter if your revert was successful "\
                    "or not, so you might conflict with other users here (double check if user isn't already reported) "\
                    "Do you want to report this user?"
                  , QMessageBox::Yes|QMessageBox::No);
    if (q == QMessageBox::No)
    {
        return false;
    }
    return true;
}

void Core::NormalizeConf()
{
    if (Configuration::LocalConfig_TemplateAge > -1)
    {
        Configuration::LocalConfig_TemplateAge = -30;
    }
    if (Configuration::Cache_InfoSize < 10)
    {
        Configuration::Cache_InfoSize = 10;
    }
}

QString Core::MakeLocalUserConfig()
{
    QString conf = "<nowiki>\n";
    conf += "enable:true\n";
    conf += "version:" + Configuration::HuggleVersion + "\n\n";
    conf += "admin:true\n";
    conf += "patrol-speedy:true\n";
    conf += "speedy-message-title:Speedy deleted\n";
    conf += "report-summary:" + Configuration::LocalConfig_ReportSummary + "\n";
    conf += "prod-message-summary:Notification: Proposed deletion of [[$1]]\n";
    conf += "warn-summary-4:" + Configuration::LocalConfig_WarnSummary4 + "\n";
    conf += "warn-summary-3:" + Configuration::LocalConfig_WarnSummary3 + "\n";
    conf += "warn-summary-2:" + Configuration::LocalConfig_WarnSummary2 + "\n";
    conf += "warn-summary:" + Configuration::LocalConfig_WarnSummary + "\n";
    conf += "auto-advance:false\n";
    conf += "auto-whitelist:true\n";
    conf += "confirm-multiple:true\n";
    conf += "confirm-range:true\n";
    conf += "confirm-page:true\n";
    conf += "template-age:" + QString::number(Configuration::LocalConfig_TemplateAge) + "\n";
    conf += "</nowiki>";
    return conf;
}

Language::Language(QString name)
{
    LanguageName = name;
    LanguageID = name;
}
