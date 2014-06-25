//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "core.hpp"
#include <QtXml>
#include <QMessageBox>
#include <QPluginLoader>
#include "configuration.hpp"
#include "exception.hpp"
#include "exceptionwindow.hpp"
#include "hooks.hpp"
#include "hugglefeed.hpp"
#include "hugglequeuefilter.hpp"
#include "iextension.hpp"
#include "localization.hpp"
#include "login.hpp"
#include "mainwindow.hpp"
#include "sleeper.hpp"
#include "resources.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikipage.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

// definitions
Core    *Core::HuggleCore = nullptr;

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
    this->gc = new Huggle::GC();
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
    HUGGLE_DEBUG1("Loading defs");
    this->LoadDefs();
    HUGGLE_DEBUG1("Loading wikis");
    this->LoadDB();
    HUGGLE_DEBUG1("Loading queue");
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
#ifdef HUGGLE_GLOBAL_EXTENSION_PATH
        Syslog::HuggleLogs->Log("Loading plugins in " + QString(HUGGLE_GLOBAL_EXTENSION_PATH) + " and " + Configuration::GetExtensionsRootPath());
#else
        Syslog::HuggleLogs->Log("Loading plugins in " + Configuration::GetExtensionsRootPath());
#endif
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
    this->Python = nullptr;
#endif
    this->Main = nullptr;
    this->fLogin = nullptr;
    this->SecondaryFeedProvider = nullptr;
    this->PrimaryFeedProvider = nullptr;
    this->Processor = nullptr;
    this->HuggleSyslog = nullptr;
    this->StartupTime = QDateTime::currentDateTime();
    this->Running = true;
    this->gc = nullptr;
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
    if (Configuration::HuggleConfiguration->Project != nullptr)
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

    if (text.isEmpty())
    {
        QFile vf(":/huggle/resources/Resources/Definitions.xml");
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
        if (!e.attributes().contains("name") ||
            !e.attributes().contains("url"))
        {
            continue;
        }
        WikiSite *site = new WikiSite(e.attribute("name"), e.attribute("url"));
        site->IRCChannel = "";
        site->SupportOAuth = false;
        site->SupportHttps = false;
        site->WhiteList = "test";
        if (e.attributes().contains("path"))
            site->LongPath = e.attribute("path");
        if (e.attributes().contains("wl"))
            site->WhiteList = e.attribute("wl");
        if (e.attributes().contains("script"))
            site->ScriptPath = e.attribute("script");
        if (e.attributes().contains("https"))
            site->SupportHttps = Configuration::SafeBool(e.attribute("https"));
        if (e.attributes().contains("oauth"))
            site->SupportOAuth = Configuration::SafeBool(e.attribute("oauth"));
        if (e.attributes().contains("channel"))
            site->IRCChannel = e.attribute("channel");
        if (e.attributes().contains("rtl"))
            site->IsRightToLeft = Configuration::SafeBool(e.attribute("rtl"));
        Configuration::HuggleConfiguration->ProjectList.append(site);
        xx++;
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
    HUGGLE_DEBUG1("Loaded " + QString::number(WikiUser::ProblematicUsers.count()) + " records from last session");
    defs.close();
}

void Core::ExtensionLoad()
{
    QString path_ = Configuration::GetExtensionsRootPath();
    if (QDir().exists(path_))
    {
        QDir d(path_);
        QStringList extensions;
        QStringList files = d.entryList();
        foreach (QString e_, files)
        {
            // we need to prefix the files here so that we can track the full path
            extensions << path_ + e_;
        }
#ifdef HUGGLE_GLOBAL_EXTENSION_PATH
        if (QDir().exists(HUGGLE_GLOBAL_EXTENSION_PATH))
        {
            QString globalpath(HUGGLE_GLOBAL_EXTENSION_PATH);
            // ensure it's finished with slash
            globalpath += "/";
            QDir g(globalpath);
            files = g.entryList();
            foreach (QString e_, files)
            {
                // we need to prefix the files here so that we can track the full path
                extensions << globalpath + e_;
            }
        }
#endif
        int xx = 0;
        while (xx < extensions.count())
        {
            QString name = extensions.at(xx).toLower();
            if (name.endsWith(".so") || name.endsWith(".dll"))
            {
                name = extensions.at(xx);
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
                        } else
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
                            interface->Localization = Localizations::HuggleLocalizations;
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
                name = extensions.at(xx);
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
#ifndef PYTHONENGINE
    Huggle::Syslog::HuggleLogs->Log("Extensions: " + QString::number(Core::Extensions.count()));
#else
    Huggle::Syslog::HuggleLogs->Log("Extensions: " + QString::number(Core::Python->Count() + Core::Extensions.count()));
#endif
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
    // we need to disable all extensions first
    Hooks::Shutdown();
    // now we can shutdown whole huggle
    this->Running = false;
    // grace time for subthreads to finish
    if (this->Main != nullptr)
    {
        if (this->PrimaryFeedProvider && this->PrimaryFeedProvider->IsWorking())
        {
            this->PrimaryFeedProvider->Stop();
        }
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
    QueryPool::HugglePool = nullptr;
    if (this->fLogin != nullptr)
    {
        delete this->fLogin;
        this->fLogin = nullptr;
    }
    if (this->Main != nullptr)
    {
        delete this->Main;
        this->Main = nullptr;
    }
    MainWindow::HuggleMain = nullptr;
    delete this->HGQP;
    this->HGQP = nullptr;
    QueryPool::HugglePool = nullptr;
    // now stop the garbage collector and wait for it to finish
    GC::gc->Stop();
    Syslog::HuggleLogs->Log("SHUTDOWN: waiting for garbage collector to finish");
    while(GC::gc->IsRunning())
        Sleeper::usleep(200);
    // last garbage removal
    GC::gc->DeleteOld();
#ifdef HUGGLE_PROFILING
    Syslog::HuggleLogs->Log("Profiler info: locks " + QString::number(Collectable::LockCt));
    foreach (Collectable *q, GC::gc->list)
    {
        // retrieve GC info
        Syslog::HuggleLogs->Log(q->DebugHgc());
    }
#endif
    Syslog::HuggleLogs->DebugLog("GC: " + QString::number(GC::gc->list.count()) + " objects");
    delete GC::gc;
    GC::gc = nullptr;
    this->gc = nullptr;
    delete Configuration::HuggleConfiguration;
    delete Localizations::HuggleLocalizations;
    QApplication::quit();
}

void Core::TestLanguages()
{
    if (Configuration::HuggleConfiguration->SystemConfig_LanguageSanity)
    {
        Language *english = Localizations::HuggleLocalizations->LocalizationData.at(Localizations::EnglishID);
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
    if (Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
        Localizations::HuggleLocalizations->LocalInit("en"); // English, when in safe mode
        Huggle::Syslog::HuggleLogs->Log("Skipping load of other languages, because of safe mode");
        return;
    }
    Localizations::HuggleLocalizations->LocalInit("ar"); // Arabic
    Localizations::HuggleLocalizations->LocalInit("bg"); // Bulgarian
    //Localizations::HuggleLocalizations->LocalInit("bn"); // Bengali
    Localizations::HuggleLocalizations->LocalInit("cz"); // Czech
    Localizations::HuggleLocalizations->LocalInit("de"); // Deutsch
    Localizations::HuggleLocalizations->LocalInit("en"); // English
    Localizations::HuggleLocalizations->LocalInit("es"); // Spanish
    Localizations::HuggleLocalizations->LocalInit("fa"); // Persian
    Localizations::HuggleLocalizations->LocalInit("fr"); // French
    Localizations::HuggleLocalizations->LocalInit("he"); // Hebrew
    Localizations::HuggleLocalizations->LocalInit("hi"); // Hindi
    //Localizations::HuggleLocalizations->LocalInit("it"); // Italian
    Localizations::HuggleLocalizations->LocalInit("ja"); // Japanese
    Localizations::HuggleLocalizations->LocalInit("ka"); // ?
    //Localizations::HuggleLocalizations->LocalInit("km"); // Khmer
    Localizations::HuggleLocalizations->LocalInit("kn"); // Kannada
    Localizations::HuggleLocalizations->LocalInit("ko"); // Korean
    Localizations::HuggleLocalizations->LocalInit("lb"); // Lebanon
    Localizations::HuggleLocalizations->LocalInit("mk"); // Macedonian
    Localizations::HuggleLocalizations->LocalInit("ml"); // Malayalam
    Localizations::HuggleLocalizations->LocalInit("mr"); // Marathi
    Localizations::HuggleLocalizations->LocalInit("nl"); // Dutch
    Localizations::HuggleLocalizations->LocalInit("no"); // Norwegian
    Localizations::HuggleLocalizations->LocalInit("oc"); // Occitan
    Localizations::HuggleLocalizations->LocalInit("or"); // Oriya
    Localizations::HuggleLocalizations->LocalInit("pt"); // Portuguese
    Localizations::HuggleLocalizations->LocalInit("pt-BR"); // Portuguese (in Brazil)
    Localizations::HuggleLocalizations->LocalInit("ru"); // Russian
    Localizations::HuggleLocalizations->LocalInit("sv"); // Swedish
    Localizations::HuggleLocalizations->LocalInit("tr"); // Turkish
    Localizations::HuggleLocalizations->LocalInit("zh"); // Chinese
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
        delete ex;
    }catch (Huggle::Exception &ex)
    {
        Core::ExceptionHandler(&ex);
    }
    return done;
}
