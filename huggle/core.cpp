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
Core  *Core::HuggleCore = NULL;

void Core::Init()
{
    this->StartupTime = QDateTime::currentDateTime();
    // preload of config
    Configuration::HuggleConfiguration->WikiDB = Configuration::GetConfigurationPath() + "wikidb.xml";
    if (Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
        Syslog::HuggleLogs->Log("DEBUG: Huggle is running in a safe mode");
    }
#ifdef HUGGLE_BREAKPAD
    Syslog::HuggleLogs->Log("Dumping enabled using google breakpad");
#endif
    this->gc = new GC();
    GC::gc = this->gc;
    Query::NetworkManager = new QNetworkAccessManager();
    QueryPool::HugglePool = new QueryPool();
    this->HGQP = QueryPool::HugglePool;
    this->HuggleSyslog = Syslog::HuggleLogs;
    Core::VersionRead();
#if QT_VERSION >= 0x050000
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#else
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    Syslog::HuggleLogs->Log("Huggle 3 QT-LX, version " + Configuration::HuggleConfiguration->HuggleVersion);
    Resources::Init();
    Syslog::HuggleLogs->Log("Loading configuration");
    this->Processor = new ProcessorThread();
    this->Processor->start();
    this->LoadLocalizations();
    Huggle::Syslog::HuggleLogs->Log("Home: " + Configuration::GetConfigurationPath());
    if (QFile().exists(Configuration::GetConfigurationPath() + HUGGLE_CONF))
    {
        Configuration::LoadSystemConfig(Configuration::GetConfigurationPath() + HUGGLE_CONF);
    } else if (QFile().exists(QCoreApplication::applicationDirPath() + HUGGLE_CONF))
    {
        Configuration::LoadSystemConfig(QCoreApplication::applicationDirPath() + HUGGLE_CONF);
    }
    Syslog::HuggleLogs->DebugLog("Loading defs");
    this->LoadDefs();
    Syslog::HuggleLogs->DebugLog("Loading wikis");
    this->LoadDB();
    Syslog::HuggleLogs->DebugLog("Loading queue");
    // These are separators that we use to parse words, less we have, faster huggle will be,
    // despite it will fail more to detect vandals. Keep it low but precise enough!!
    Configuration::HuggleConfiguration->SystemConfig_WordSeparators << " " << "." << "," << "(" << ")" << ":" << ";" << "!"
                                                                    << "?" << "/" << "<" << ">" << "[" << "]";
    HuggleQueueFilter::Filters.append(HuggleQueueFilter::DefaultFilter);
    if (!Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
#ifdef PYTHONENGINE
        Syslog::HuggleLogs->Log("Loading python engine");
        this->Python = new Python::PythonEngine(Configuration::GetExtensionsRootPath());
#endif
        Syslog::HuggleLogs->Log("Loading plugins in " + Configuration::GetExtensionsRootPath());
        this->ExtensionLoad();
    } else
    {
        Syslog::HuggleLogs->Log("Not loading plugins in a safe mode");
    }
    Syslog::HuggleLogs->Log("Loaded in " + QString::number(this->StartupTime.msecsTo(QDateTime::currentDateTime())) + "ms");
}

Core::Core()
{
#ifdef PYTHONENGINE
    this->Python = NULL;
#endif
    this->Main = NULL;
    this->fLogin = NULL;
    this->SecondaryFeedProvider = NULL;
    this->PrimaryFeedProvider = NULL;
    this->HuggleLocalizations = NULL;
    this->Processor = NULL;
    this->HuggleSyslog = NULL;
    this->StartupTime = QDateTime::currentDateTime();
    this->Running = true;
    this->gc = NULL;
}

Core::~Core()
{
    delete this->Main;
    delete this->fLogin;
    delete this->SecondaryFeedProvider;
    delete this->PrimaryFeedProvider;
    delete this->gc;
    delete this->Processor;
}

void Core::LoadDB()
{
    Configuration::HuggleConfiguration->ProjectList.clear();
    if (Configuration::HuggleConfiguration->Project != NULL)
    {
        Configuration::HuggleConfiguration->ProjectList << Configuration::HuggleConfiguration->Project;
    }
    QString text = "";
    if (QFile::exists(Configuration::HuggleConfiguration->WikiDB))
    {
        QFile db(Configuration::HuggleConfiguration->WikiDB);
        if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            Syslog::HuggleLogs->ErrorLog("Unable to read " + Configuration::HuggleConfiguration->WikiDB);
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
        edit->Previous = NULL;
        edit->Next = NULL;
        edit->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
        return;
    }

    if (edit->Previous != NULL)
    {
        edit->Previous->Next = NULL;
        edit->Previous = NULL;
    }

    if (edit->Next != NULL)
    {
        edit->Next->Previous = NULL;
        edit->Next = NULL;
    }

    edit->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
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
        Huggle::Syslog::HuggleLogs->ErrorLog("Can't open " + Configuration::GetConfigurationPath() + "users.xml");
        return;
    }
    QString xx = "<definitions>\n";
    WikiUser::TrimProblematicUsersList();
    int x = 0;
    WikiUser::ProblematicUserListLock.lock();
    while (x<WikiUser::ProblematicUsers.count())
    {
        xx += "<user name=\"" + WikiUser::ProblematicUsers.at(x)->Username + "\" badness=\"" +
                QString::number(WikiUser::ProblematicUsers.at(x)->GetBadnessScore()) +"\"></user>\n";
        x++;
    }
    WikiUser::ProblematicUserListLock.unlock();
    xx += "</definitions>";
    file.write(xx.toUtf8());
    file.close();
    QFile().remove(Configuration::GetConfigurationPath() + "users.xml~");
}

void Core::LoadDefs()
{
    QFile defs(Configuration::GetConfigurationPath() + "users.xml");
    if (QFile(Configuration::GetConfigurationPath() + "users.xml~").exists())
    {
        Huggle::Syslog::HuggleLogs->Log("WARNING: recovering definitions from last session");
        QFile(Configuration::GetConfigurationPath() + "users.xml").remove();
        if (QFile(Configuration::GetConfigurationPath() + "users.xml~").copy(Configuration::GetConfigurationPath() + "users.xml"))
        {
            QFile().remove(Configuration::GetConfigurationPath() + "users.xml~");
        } else
        {
            Huggle::Syslog::HuggleLogs->Log("WARNING: Unable to recover the definitions");
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
                user->SetBadnessScore(e.attribute("badness").toInt());
            }
            WikiUser::ProblematicUsers.append(user);
            i++;
        }
    }
    Syslog::HuggleLogs->DebugLog("Loaded " + QString::number(WikiUser::ProblematicUsers.count()) + " records from last session");
    defs.close();
}

void Core::ExtensionLoad()
{
    QString path_ = Configuration::GetExtensionsRootPath();
    if (QDir().exists(path_))
    {
        QDir d(path_);
        QStringList extensions = d.entryList();
        int xx = 0;
        while (xx < extensions.count())
        {
            QString name = extensions.at(xx).toLower();
            if (name.endsWith(".so") || name.endsWith(".dll"))
            {
                name = QString(path_) + extensions.at(xx);
                QPluginLoader *extension = new QPluginLoader(name);
                if (extension->load())
                {
                    QObject* root = extension->instance();
                    if (root)
                    {
                        iExtension *interface = qobject_cast<iExtension*>(root);
                        if (!interface)
                        {
                            Huggle::Syslog::HuggleLogs->Log("Unable to cast the library to extension");
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
                                Huggle::Syslog::HuggleLogs->Log("Successfully loaded: " + extensions.at(xx));
                            }
                            else
                            {
                                Huggle::Syslog::HuggleLogs->Log("Unable to register: " + extensions.at(xx));
                            }
                        }
                    }
                } else
                {
                    Huggle::Syslog::HuggleLogs->Log("Failed to load (reason: " + extension->errorString() + "): " + extensions.at(xx));
                    delete extension;
                }
            } else if (name.endsWith(".py"))
            {
#ifdef PYTHONENGINE
                name = QString(path_) + extensions.at(xx);
                if (Core::Python->LoadScript(name))
                {
                    Huggle::Syslog::HuggleLogs->Log("Loaded python script: " + name);
                } else
                {
                    Huggle::Syslog::HuggleLogs->Log("Failed to load a python script: " + name);
                }
#endif
            }
            xx++;
        }
    } else
    {
        Huggle::Syslog::HuggleLogs->Log("There is no extensions folder, skipping load");
    }
    Huggle::Syslog::HuggleLogs->Log("Extensions: " + QString::number(Core::Extensions.count()));
}

void Core::VersionRead()
{
    QFile *vf = new QFile(":/huggle/git/version.txt");
    vf->open(QIODevice::ReadOnly);
    QString version(vf->readAll());
    version = version.replace("\n", "");
    Configuration::HuggleConfiguration->HuggleVersion += " " + version;
#if PRODUCTION_BUILD
    Configuration::HuggleConfiguration->HuggleVersion += " production";
#endif
    vf->close();
    delete vf;
}

void Core::Shutdown()
{
    this->Running = false;
    // grace time for subthreads to finish
    if (this->Main != NULL)
    {
        this->Main->hide();
    }
    Syslog::HuggleLogs->Log("SHUTDOWN: giving a gracetime to other threads to finish");
    Sleeper::msleep(200);
    if (this->Processor->isRunning())
    {
        this->Processor->exit();
    }
    Core::SaveDefs();
    Configuration::SaveSystemConfig();
#ifdef PYTHONENGINE
    if (!Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
        Huggle::Syslog::HuggleLogs->Log("Unloading python");
        delete this->Python;
    }
#endif
    QueryPool::HugglePool = NULL;
    delete Configuration::HuggleConfiguration;
    delete Localizations::HuggleLocalizations;
    delete GC::gc;
    delete this->HGQP;
    GC::gc = NULL;
    this->gc = NULL;
    QApplication::quit();
}

void Core::TestLanguages()
{
    if (Configuration::HuggleConfiguration->SystemConfig_LanguageSanity)
    {
        Language *english = Localizations::HuggleLocalizations->LocalizationData.at(0);
        QList<QString> keys = english->Messages.keys();
        int language = 1;
        while (language < Localizations::HuggleLocalizations->LocalizationData.count())
        {
            Language *l = Localizations::HuggleLocalizations->LocalizationData.at(language);
            int x = 0;
            while (x < keys.count())
            {
                if (!l->Messages.contains(keys.at(x)))
                {
                    Syslog::HuggleLogs->WarningLog("Language " + l->LanguageName + " is missing key " + keys.at(x));
                } else if (english->Messages[keys.at(x)] == l->Messages[keys.at(x)])
                {
                    Syslog::HuggleLogs->WarningLog("Language " + l->LanguageName + " has key " + keys.at(x)
                            + " but its content is identical to english version");
                }
                x++;
            }
            language++;
        }
    }
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
    this->HuggleLocalizations = Localizations::HuggleLocalizations;
    Localizations::HuggleLocalizations->LocalInit("en");
    if (Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
        Huggle::Syslog::HuggleLogs->Log("Skipping load of other languages, because of safe mode");
        return;
    }
    Localizations::HuggleLocalizations->LocalInit("ar");
    Localizations::HuggleLocalizations->LocalInit("bg");
    Localizations::HuggleLocalizations->LocalInit("bn");
    Localizations::HuggleLocalizations->LocalInit("cz");
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
    this->TestLanguages();
}

double Core::GetUptimeInSeconds()
{
    return (double)this->StartupTime.secsTo(QDateTime::currentDateTime());
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
