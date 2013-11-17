//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "core.hpp"

using namespace Huggle;

// definitions
#ifdef PYTHONENGINE
PythonEngine *Core::Python = NULL;
#endif


Core  *Core::HuggleCore = NULL;

void Core::Init()
{
    this->StartupTime = QDateTime::currentDateTime();
    this->WriterLock = new QMutex(QMutex::Recursive);
    // preload of config
    Configuration::HuggleConfiguration->WikiDB = Configuration::GetConfigurationPath() + "wikidb.xml";
    if (Configuration::HuggleConfiguration->_SafeMode)
    {
        this->Log("DEBUG: Huggle is running in a safe mode");
    }
    this->gc = new GC();
    GC::gc = this->gc;
    Query::NetworkManager = new QNetworkAccessManager();
    Core::VersionRead();
    QFile *vf;
#if QT_VERSION >= 0x050000
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#else
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    vf = new QFile(":/huggle/resources/Resources/Header.txt");
    vf->open(QIODevice::ReadOnly);
    this->HtmlHeader = QString(vf->readAll());
    vf->close();
    delete vf;
    this->Log("Huggle 3 QT-LX, version " + Configuration::HuggleConfiguration->HuggleVersion);
    this->Log("Loading configuration");
    Processor = new ProcessorThread();
    Processor->start();
    Configuration::LoadConfig();
    this->DebugLog("Loading defs");
    this->LoadDefs();
    Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.append("Test edits;Reverted edits by [[Special:Contributions/$1|$1]] identified as test edits");
#ifdef PYTHONENGINE
    Core::Log("Loading python engine");
    Core::Python = new PythonEngine();
#endif
    this->DebugLog("Loading wikis");
    this->LoadDB();
    this->LoadLocalizations();
    this->DebugLog("Loading queue");
    // these are separators that we use to parse words, less we have, faster huggle will be, despite it will fail more to detect vandals
    // keep it low but precise enough
    Configuration::HuggleConfiguration->Separators << " " << "." << "," << "(" << ")" << ":" << ";" << "!" << "?" << "/";
    HuggleQueueFilter::Filters.append(HuggleQueueFilter::DefaultFilter);
    if (!Configuration::HuggleConfiguration->_SafeMode)
    {
        this->Log("Loading plugins");
        this->ExtensionLoad();
    } else
    {
        this->Log("Not loading plugins in a safe mode");
    }
    this->Log("Loaded in " + QString::number(this->StartupTime.msecsTo(QDateTime::currentDateTime())));
}

Core::Core()
{
    this->HtmlHeader = "";
    this->HtmlFooter = "</table></body></html>";
    this->Main = NULL;
    this->f_Login = NULL;
    this->SecondaryFeedProvider = NULL;
    this->PrimaryFeedProvider = NULL;
    this->WriterLock = NULL;
    this->Processor = NULL;
    this->StartupTime = QDateTime::currentDateTime();
    this->Running = true;
    this->AIVP = NULL;
    this->UAAP = NULL;
    this->gc = NULL;
}

Core::~Core()
{
    delete this->Main;
    delete this->f_Login;
    delete this->SecondaryFeedProvider;
    delete this->PrimaryFeedProvider;
    delete this->gc;
    delete this->AIVP;
    delete this->WriterLock;
    delete this->UAAP;
    delete this->Processor;
}

void Core::LoadDB()
{
    Configuration::HuggleConfiguration->ProjectList.clear();
    Configuration::HuggleConfiguration->ProjectList << Configuration::HuggleConfiguration->Project;
    QString text = "";
    if (QFile::exists(Configuration::HuggleConfiguration->WikiDB))
    {
        QFile db(Configuration::HuggleConfiguration->WikiDB);
        if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->Log("ERROR: Unable to read " + Configuration::HuggleConfiguration->WikiDB);
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
            site->SupportHttps = Configuration::SafeBool(e.attribute("https"));
        }
        if (e.attributes().contains("oauth"))
        {
            site->SupportOAuth = Configuration::SafeBool(e.attribute("oauth"));
        }
        if (e.attributes().contains("channel"))
        {
            site->IRCChannel = e.attribute("channel");
        }
        Configuration::HuggleConfiguration->ProjectList.append(site);
        xx++;
    }
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

    while (text.endsWith(" "))
    {
        text = text.mid(0, text.length() - 1);
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
        edit->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
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

    edit->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
}

QString Core::GetSummaryOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.count())
    {
        QString line = Configuration::HuggleConfiguration->LocalConfig_RevertSummaries.at(id);
        if (line.startsWith(key + ";"))
        {
            return Core::GetValueFromKey(line);
        }
        id++;
    }
    return Configuration::HuggleConfiguration->DefaultRevertSummary;
}

QString Core::GetNameOfWarningTypeFromWarningKey(QString key)
{
    int id=0;
    while (id<Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count())
    {
        QString line = Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(id);
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
    while (i<Configuration::HuggleConfiguration->LocalConfig_WarningTypes.count())
    {
        QString line = Configuration::HuggleConfiguration->LocalConfig_WarningTypes.at(i);
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
            Configuration::HuggleConfiguration->LocalConfig_ScoreParts.append(ScoreWord(word.at(line), score));
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
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        this->Log("ERROR: can't open " + Configuration::GetConfigurationPath() + "users.xml");
        return;
    }
    QString xx = "<definitions>\n";
    WikiUser::TrimProblematicUsersList();
    int x = 0;
    WikiUser::ProblematicUserListLock.lock();
    while (x<WikiUser::ProblematicUsers.count())
    {
        xx += "<user name=\"" + WikiUser::ProblematicUsers.at(x)->Username + "\" badness=\"" +
                QString::number(WikiUser::ProblematicUsers.at(x)->getBadnessScore()) +"\"></user>\n";
        x++;
    }
    WikiUser::ProblematicUserListLock.unlock();
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
            Configuration::HuggleConfiguration->LocalConfig_ScoreWords.append(ScoreWord(word.at(line), score));
            line++;
        }
    }
}

Message *Core::MessageUser(WikiUser *user, QString message, QString title, QString summary, bool section, Query *dependency, bool nosuffix)
{
    if (user == NULL)
    {
        this->Log("Cowardly refusing to message NULL user");
        return NULL;
    }

    Message *m = new Message(user, message, summary);
    m->title = title;
    m->Dependency = dependency;
    m->Section = section;
    m->Suffix = !nosuffix;
    Core::Messages.append(m);
    m->Send();
    this->Log("Sending message to user " + user->Username);

    return m;
}

void Core::LoadDefs()
{
    QFile defs(Configuration::GetConfigurationPath() + "users.xml");
    if (QFile(Configuration::GetConfigurationPath() + "users.xml~").exists())
    {
        this->Log("WARNING: recovering definitions from last session");
        QFile(Configuration::GetConfigurationPath() + "users.xml").remove();
        if (QFile(Configuration::GetConfigurationPath()
                   + "users.xml~").copy(Configuration::GetConfigurationPath()
                   + "users.xml"))
        {
            QFile().remove(Configuration::GetConfigurationPath() + "users.xml~");
        } else
        {
            this->Log("WARNING: Unable to recover the definitions");
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
                user->setBadnessScore(e.attribute("badness").toInt());
            }
            WikiUser::ProblematicUsers.append(user);
            i++;
        }
    }
    defs.close();
}

void Core::FinalizeMessages()
{
    if (this->Messages.count() < 1)
    {
        return;
    }
    int x=0;
    QList<Message*> list;
    while (x<this->Messages.count())
    {
        if (this->Messages.at(x)->Finished())
        {
            list.append(this->Messages.at(x));
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

QString Core::RetrieveTemplateToWarn(QString type)
{
    int x=0;
    while (x < Configuration::HuggleConfiguration->LocalConfig_WarningTemplates.count())
    {
        if (Core::GetKeyFromValue(Configuration::HuggleConfiguration->LocalConfig_WarningTemplates.at(x)) == type)
        {
            return Core::GetValueFromKey(Configuration::HuggleConfiguration->LocalConfig_WarningTemplates.at(x));
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
    if (!summary.endsWith(Configuration::HuggleConfiguration->EditSuffixOfHuggle))
    {
        summary = summary + " " + Configuration::HuggleConfiguration->EditSuffixOfHuggle;
    }
    _e->RegisterConsumer("Core::EditPage");
    _e->page = page->PageName;
    this->PendingMods.append(_e);
    _e->text = text;
    _e->summary = summary;
    _e->Minor = minor;
    _e->Process();
    return _e;
}

void Core::AppendQuery(Query *item)
{
    item->RegisterConsumer("core");
    this->RunningQueries.append(item);
}

void Core::Log(QString Message)
{
    Message = "<" + QDateTime::currentDateTime().toString() + "> " + Message;
    std::cout << Message.toStdString() << std::endl;
    this->InsertToRingLog(Message);
    if (this->Main != NULL)
    {
        this->Main->lUnwrittenLogs.lock();
        this->Main->UnwrittenLogs.append(Message);
        this->Main->lUnwrittenLogs.unlock();
    }
    if (Configuration::HuggleConfiguration->Log2File)
    {
        this->WriterLock->lock();
        QFile *file = new QFile(Configuration::HuggleConfiguration->SyslogPath);
        if (file->open(QIODevice::Append))
        {
            file->write(QString(Message + "\n").toUtf8());
            file->close();
        }
        delete file;
        this->WriterLock->unlock();
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
                            if (interface->RequestNetwork())
                            {
                                interface->Networking = Query::NetworkManager;
                            }
                            if (interface->RequestConfiguration())
                            {
                                interface->Configuration = Configuration::HuggleConfiguration;
                            }
                            if (interface->RequestCore())
                            {
                                interface->HuggleCore = Core::HuggleCore;
                            }
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
            } else if (name.endsWith(".py"))
            {
#ifdef PYTHONENGINE
                name = QString(EXTENSION_PATH) + QDir::separator() + extensions.at(xx);
                if (Core::Python->LoadScript(name))
                {
                    Core::Log("Loaded python script: " + name);
                } else
                {
                    Core::Log("Failed to load a python script: " + name);
                }
#endif
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
    Configuration::HuggleConfiguration->HuggleVersion += " " + version;
#ifdef PRODUCTION_BUILD
    Configuration::HuggleConfiguration->HuggleVersion += " production";
#endif
    vf->close();
    delete vf;
}

void Core::DebugLog(QString Message, unsigned int Verbosity)
{
    if (Configuration::HuggleConfiguration->Verbosity >= Verbosity)
    {
        Core::Log("DEBUG[" + QString::number(Verbosity) + "]: " + Message);
    }
}

QString Core::GetProjectURL(WikiSite Project)
{
    return Configuration::HuggleConfiguration->GetURLProtocolPrefix() + Project.URL;
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
    return Configuration::GetURLProtocolPrefix() + Configuration::HuggleConfiguration->Project->URL;
}

QString Core::GetProjectWikiURL()
{
    return Core::GetProjectURL(Configuration::HuggleConfiguration->Project) + Configuration::HuggleConfiguration->Project->LongPath;
}

QString Core::GetProjectScriptURL()
{
    return Core::GetProjectURL(Configuration::HuggleConfiguration->Project) + Configuration::HuggleConfiguration->Project->ScriptPath;
}

void Core::ProcessEdit(WikiEdit *e)
{
    Core::Main->ProcessEdit(e);
}

void Core::Shutdown()
{
    Running = false;
    // grace time for subthreads to finish
    if (this->Main != NULL)
    {
        this->Main->hide();
    }
    Sleeper::msleep(200);
    if (this->Processor->isRunning())
    {
        this->Processor->exit();
    }
    Core::SaveDefs();
    Configuration::SaveConfig();
#ifdef PYTHONENGINE
    this->Log("Unloading python");
    delete this->Python;
#endif
    delete Configuration::HuggleConfiguration;
    delete Localizations::HuggleLocalizations;
    delete GC::gc;
    GC::gc = NULL;
    this->gc = NULL;
    QApplication::quit();
}

QString Core::RingLogToText()
{
    int i = 0;
    QString text = "";
    while (i<this->RingLog.size())
    {
        text = this->RingLog.at(i) + "\n" + text;
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
    if (Core::RingLog.size()+1 > Configuration::HuggleConfiguration->RingLogMaxSize)
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

    if (_e->Bot)
    {
        _e->User->SetBot(true);
    }

    _e->EditMadeByHuggle = _e->Summary.contains(Configuration::HuggleConfiguration->EditSuffixOfHuggle);

    _e->Status = StatusProcessed;
}

void Core::PostProcessEdit(WikiEdit *_e)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit in PostProcessEdit(WikiEdit *_e) is not a valid edit");
    }
    _e->RegisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
    _e->UnregisterConsumer(HUGGLECONSUMER_WIKIEDIT);
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
                EditQuery *e = Core::PendingMods.at(curr);
                Core::PendingMods.removeAt(curr);
                e->UnregisterConsumer("Core::EditPage");
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
        item->Lock();
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
    if (Configuration::HuggleConfiguration->WarnUserSpaceRoll && _e->Page->IsUserpage())
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

RevertQuery *Core::RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit in RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep) is not a valid edit");
    }
    if (_e->User == NULL)
    {
        throw new Exception("Object user was NULL in Core::Revert");
    }
    _e->RegisterConsumer("Core::RevertEdit");
    if (_e->Page == NULL)
    {
        throw new Exception("Object page was NULL");
    }

    RevertQuery *query = new RevertQuery(_e);
    if (summary != "")
    {
        query->Summary = summary;
    }
    query->MinorEdit = minor;
    Core::AppendQuery(query);
    if (Configuration::HuggleConfiguration->EnforceManualSoftwareRollback)
    {
        query->UsingSR = true;
    } else
    {
        query->UsingSR = !rollback;
    }
    query->Process();

    if (keep)
    {
        query->RegisterConsumer("keep");
    }

    return query;
}

void Core::ExceptionHandler(Exception *exception)
{
    ExceptionWindow *w = new ExceptionWindow(exception);
    w->exec();
    delete w;
}

void Core::LoadLocalizations()
{
    Localizations::HuggleLocalizations = new Localizations();
    Localizations::HuggleLocalizations->LocalInit("en");
    if (Configuration::HuggleConfiguration->_SafeMode)
    {
        Core::Log("Skipping load of other languages, because of safe mode");
        return;
    }
    Localizations::HuggleLocalizations->LocalInit("ar");
    Localizations::HuggleLocalizations->LocalInit("bg");
    Localizations::HuggleLocalizations->LocalInit("bn");
    Localizations::HuggleLocalizations->LocalInit("es");
    Localizations::HuggleLocalizations->LocalInit("de");
    Localizations::HuggleLocalizations->LocalInit("fa");
    Localizations::HuggleLocalizations->LocalInit("fr");
    Localizations::HuggleLocalizations->LocalInit("hi");
    Localizations::HuggleLocalizations->LocalInit("it");
    Localizations::HuggleLocalizations->LocalInit("ja");
    Localizations::HuggleLocalizations->LocalInit("ka");
    Localizations::HuggleLocalizations->LocalInit("km");
    Localizations::HuggleLocalizations->LocalInit("kn");
    Localizations::HuggleLocalizations->LocalInit("ko");
    Localizations::HuggleLocalizations->LocalInit("ml");
    Localizations::HuggleLocalizations->LocalInit("mr");
    Localizations::HuggleLocalizations->LocalInit("nl");
    Localizations::HuggleLocalizations->LocalInit("no");
    Localizations::HuggleLocalizations->LocalInit("oc");
    Localizations::HuggleLocalizations->LocalInit("or");
    Localizations::HuggleLocalizations->LocalInit("pt");
    Localizations::HuggleLocalizations->LocalInit("ptb");
    Localizations::HuggleLocalizations->LocalInit("ru");
    Localizations::HuggleLocalizations->LocalInit("sv");
    Localizations::HuggleLocalizations->LocalInit("zh");
}

bool Core::ReportPreFlightCheck()
{
    if (!Configuration::HuggleConfiguration->AskUserBeforeReport)
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

int Core::RunningQueriesGetCount()
{
    return RunningQueries.count();
}

Language::Language(QString name)
{
    LanguageName = name;
    LanguageID = name;
}


bool HgApplication::notify(QObject *receiver, QEvent *event)
{
    bool done = true;
    try
    {
        done = QApplication::notify(receiver, event);
    }catch (Huggle::Exception *ex)
    {
        Core::ExceptionHandler(ex);
    }catch (Huggle::Exception &ex)
    {
        Core::ExceptionHandler(&ex);
    }
    return done;
}
