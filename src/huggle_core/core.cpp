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
#include <QFile>
#include <QPluginLoader>
#include <huggle_l10n/huggle_l10n.hpp>
#include "configuration.hpp"
#include "exception.hpp"
#include "exceptionhandler.hpp"
#include "events.hpp"
#include "gc.hpp"
#include "generic.hpp"
#include "hugglefeed.hpp"
#include "huggleprofiler.hpp"
#include "hugglequeuefilter.hpp"
#include "iextension.hpp"
#include "localization.hpp"
#include "hooks.hpp"
#include "sleeper.hpp"
#include "resources.hpp"
#include "query.hpp"
#include "querypool.hpp"
#include "scripting/script.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

// definitions
Core    *Core::HuggleCore = nullptr;

void Core::Init()
{
    // This goes first otherwise we can't throw exceptions
    this->exceptionHandler = new ExceptionHandler();
    // We check if this isn't an attempt to start huggle core which was already started, this can cause serious hard to debug problems with network
    if (this->loaded)
        throw new Huggle::Exception("Initializing huggle core that was already loaded", BOOST_CURRENT_FUNCTION);
    HUGGLE_PROFILER_RESET;
    this->loaded = true;

    if (Events::Global)
        throw new Huggle::Exception("Global events are already loaded", BOOST_CURRENT_FUNCTION);
    Events::Global = new Events();

    // preload of config
    hcfg->WikiDB = Generic::SanitizePath(Configuration::GetConfigurationPath() + "wikidb.xml");
    if (hcfg->SystemConfig_SafeMode)
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
    Syslog::HuggleLogs->Log("Huggle version " + Configuration::HuggleConfiguration->HuggleVersion);
    Resources::Init();
    Syslog::HuggleLogs->Log("Loading configuration");
    this->processorThread = new WikiEdit_ProcessorThread();
    this->processorThread->start();
    this->LoadLocalizations();
    Huggle::Syslog::HuggleLogs->Log("Home: " + hcfg->HomePath);
    if (QFile().exists(Configuration::GetConfigurationPath() + HUGGLE_CONF))
    {
        Configuration::LoadSystemConfig(Configuration::GetConfigurationPath() + HUGGLE_CONF);
    } else if (QFile().exists(QCoreApplication::applicationDirPath() + HUGGLE_CONF))
    {
        Configuration::LoadSystemConfig(QCoreApplication::applicationDirPath() + HUGGLE_CONF);
    }
    hcfg->WebRequest_UserAgent = QString("Huggle/" + QString(HUGGLE_VERSION) + " (http://en.wikipedia.org/wiki/WP:Huggle; " + hcfg->HuggleVersion + ")").toUtf8();
    HUGGLE_DEBUG1("UserAgent: " + QString(hcfg->WebRequest_UserAgent));
    // Create a global wiki, now that we loaded the configuration which is only place where it can be changed
    hcfg->GlobalWiki = new WikiSite("GlobalWiki", hcfg->GlobalConfigurationWikiAddress);
    HUGGLE_PROFILER_PRINT_TIME("Core::Init()@conf");
    HUGGLE_DEBUG1("Loading wikis");
    this->LoadDB();
    HUGGLE_DEBUG1("Loading queue");
    // These are separators that we use to parse individual words, too many items will have negative impact on performance,
    // so despite it would help us detect vandals, we need to keep it low, but precise enough!!
    hcfg->SystemConfig_WordSeparators << " " << "." << "," << "(" << ")" << ":" << ";" << "!"
                                      << "?" << "/" << "<" << ">" << "[" << "]";
    if (!hcfg->SystemConfig_SafeMode)
    {
#ifdef HUGGLE_GLOBAL_EXTENSION_PATH
        Syslog::HuggleLogs->Log("Loading plugins in " + Generic::SanitizePath(QString(HUGGLE_GLOBAL_EXTENSION_PATH)) + " and " + Configuration::GetExtensionsRootPath());
#else
        Syslog::HuggleLogs->Log("Loading plugins in " + Configuration::GetExtensionsRootPath());
#endif
        this->ExtensionLoad();
    } else
    {
        Syslog::HuggleLogs->Log("Not loading plugins in a safe mode");
    }
    Syslog::HuggleLogs->Log("Loaded in " + QString::number(this->StartupTime.msecsTo(QDateTime::currentDateTime())) + "ms");
    HUGGLE_PROFILER_PRINT_TIME("Core::Init()@finalize");
}

Core::Core()
{
    this->processorThread = nullptr;
    this->HuggleSyslog = nullptr;
    this->StartupTime = QDateTime::currentDateTime();
    this->Running = true;
    this->gc = nullptr;
}

Core::~Core()
{
    delete this->gc;
    delete this->processorThread;
    delete this->exceptionHandler;
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
        site->SupportHttps = false;
        site->WhiteList = "test";
        if (e.attributes().contains("path"))
            site->LongPath = e.attribute("path");
        if (e.attributes().contains("wl"))
            site->WhiteList = e.attribute("wl");
        if (e.attributes().contains("script"))
            site->ScriptPath = e.attribute("script");
        if (e.attributes().contains("https"))
            site->SupportHttps = Generic::SafeBool(e.attribute("https"));
        if (e.attributes().contains("channel"))
            site->IRCChannel = e.attribute("channel");
        if (e.attributes().contains("rtl"))
            site->IsRightToLeft = Generic::SafeBool(e.attribute("rtl"));
        if (e.attributes().contains("han_irc"))
            site->HANChannel = e.attribute("han_irc");
        if (e.attributes().contains("xmlrcs_name"))
            site->XmlRcsName = e.attribute("xmlrcs_name");
        if (e.attributes().contains("ssl_required"))
            site->ForceSSL = Generic::SafeBool(e.attribute("ssl_required"));
        Configuration::HuggleConfiguration->ProjectList.append(site);
        xx++;
    }
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
        QString global_path = Generic::SanitizePath(QString(HUGGLE_GLOBAL_EXTENSION_PATH) + "/");
        if (QDir().exists(global_path))
        {
            QDir g(global_path);
            files = g.entryList();
            foreach (QString e_, files)
            {
                // we need to prefix the files here so that we can track the full path
                extensions << global_path + e_;
            }
        }
#endif
        foreach (QString ename, extensions)
        {
            if (hcfg->IgnoredExtensions.contains(ename))
            {
                ExtensionHolder *extension = new ExtensionHolder();
                extension->Name = QFile(ename).fileName();
                extension->huggle__internal_SetPath(ename);
                // we append this so that it's possible to re enable it
                Extensions.append(extension);
                HUGGLE_DEBUG1("Path was ignored: " + ename);
                continue;
            }
            QString name = ename.toLower();
            if (name.endsWith(".so") || name.endsWith(".dll") || name.endsWith(".dylib"))
            {
                QPluginLoader *extension = new QPluginLoader(ename);
                if (extension->load())
                {
                    QObject* root = extension->instance();
                    if (root)
                    {
                        iExtension *interface = qobject_cast<iExtension*>(root);
                        if (!interface)
                        {
                            Huggle::Syslog::HuggleLogs->Log("Unable to cast the library to extension: " + ename);
                        }
                        else if (interface->CompiledFor() != QString(HUGGLE_VERSION))
                        {
                            Huggle::Syslog::HuggleLogs->WarningLog("Extension " + ename + " was compiled for huggle " + interface->CompiledFor() + " which is not compatible, unloading");
                            delete interface;
                        }
                        {
                            interface->huggle__internal_SetPath(ename);
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
                                Huggle::Syslog::HuggleLogs->Log("Successfully loaded: " + ename);
                            }
                            else
                            {
                                Huggle::Syslog::HuggleLogs->Log("Unable to register: " + ename);
                            }
                        }
                    }
                } else
                {
                    Huggle::Syslog::HuggleLogs->Log("Failed to load (reason: " + extension->errorString() + "): " + ename);
                    delete extension;
                }
            }
        }
    } else
    {
        Huggle::Syslog::HuggleLogs->Log("There is no extensions folder, skipping load");
    }
    Huggle::Syslog::HuggleLogs->Log("Extensions: " + QString::number(Script::GetScripts().count() + Core::Extensions.count()));
}

void Core::VersionRead()
{
    QFile *vf = new QFile(":/huggle/git/version.txt");
    vf->open(QIODevice::ReadOnly);
    QString version(vf->readAll());
    version = version.replace("\r", "");
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
    // Now we can shutdown whole huggle
    this->Running = false;
    // We need to disable all extensions first
    Hooks::Shutdown();
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
    {
        if (site->Provider && site->Provider->IsWorking())
            site->Provider->Stop();
    }
    // Grace time for subthreads to finish
    Syslog::HuggleLogs->Log("SHUTDOWN: giving a gracetime to other threads to finish");
    Sleeper::msleep(200);
    if (this->processorThread->isRunning())
        this->processorThread->exit();

    // We need to make a copy of list here, because calling delete would remove the pointer from original list
    // that could cause some issues.
    QList<Script*> sl = Script::GetScripts();
    foreach (Script *s, sl)
    {
        s->Hook_Shutdown();
        delete s;
    }
    QueryPool::HugglePool = nullptr;
    Configuration::SaveSystemConfig();
    delete this->HGQP;
    this->HGQP = nullptr;
    QueryPool::HugglePool = nullptr;
    // Now stop the garbage collector and wait for it to finish
    GC::gc->Stop();
    Syslog::HuggleLogs->Log("SHUTDOWN: waiting for garbage collector to finish");
    while(GC::gc->IsRunning())
        Sleeper::usleep(200);
    // Last garbage removal
    GC::gc->DeleteOld();
#ifdef HUGGLE_PROFILING
    Syslog::HuggleLogs->Log("Profiler data:");
    Syslog::HuggleLogs->Log("==========================");
    Syslog::HuggleLogs->Log("Locks " + QString::number(Collectable::LockCt));
    foreach (Collectable *q, GC::gc->list)
    {
        // retrieve GC info
        Syslog::HuggleLogs->Log(q->DebugHgc());
    }
    Syslog::HuggleLogs->Log("Function calls:");
    QStringList functions = Profiler::GetRegisteredCounterFunctions();
    foreach (QString fx, functions)
    {
        Syslog::HuggleLogs->Log(QString::number(Profiler::GetCallsForFunction(fx)) + ": " + fx);
    }
#endif
    Syslog::HuggleLogs->DebugLog("GC: " + QString::number(GC::gc->list.count()) + " objects");
    delete GC::gc;
    HuggleQueueFilter::Delete();
    GC::gc = nullptr;
    this->gc = nullptr;
    delete Query::NetworkManager;
    delete Configuration::HuggleConfiguration;
    // We need to change these to null so that functions that would want to access there later during destruction of Qt derived
    // HW objects would know that they are no longer available and wouldn't crash huggle
    Configuration::HuggleConfiguration = nullptr;
    delete Localizations::HuggleLocalizations;
    Localizations::HuggleLocalizations = nullptr;
    // Syslog should be deleted last because since now there is no way to effectively report stuff to terminal
    delete Syslog::HuggleLogs;
    Syslog::HuggleLogs = nullptr;
    Resources::Uninit();
    delete Events::Global;
    Events::Global = nullptr;
    QCoreApplication::quit();
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

void Core::LoadLocalizations()
{
    Localizations::HuggleLocalizations = new Localizations();
    if (Configuration::HuggleConfiguration->SystemConfig_SafeMode)
    {
        Localizations::HuggleLocalizations->LocalInit("en"); // English, when in safe mode
        Huggle::Syslog::HuggleLogs->Log("Skipping load of other languages, because of safe mode");
        return;
    }
    QStringList localizations = Huggle_l10n::GetLocalizations();
    foreach (QString localization_name, localizations)
        Localizations::HuggleLocalizations->LocalInit(localization_name);
    this->TestLanguages();
}

void Core::InstallNewExceptionHandler(ExceptionHandler *eh)
{
    delete this->exceptionHandler;
    this->exceptionHandler = eh;
}

qint64 Core::GetUptimeInSeconds()
{
    return this->StartupTime.secsTo(QDateTime::currentDateTime());
}

// Exception handling
void Core::HandleException(Exception *exception)
{
    Core::HuggleCore->exceptionHandler->HandleException(exception);
}
