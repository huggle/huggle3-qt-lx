//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QList>
#include <QString>
#include <QFile>
#include <QtXml>
#include <QMessageBox>
#include "login.h"
#include "configuration.h"
#include "query.h"
#include "wikiedit.h"
#include "mainwindow.h"
#include "message.h"
#include "editquery.h"
#include "apiquery.h"

#ifdef PYTHONENGINE
#include "pythonengine.h"
#endif

// Predeclaring some types
class Login;
class Query;
class ApiQuery;
class MainWindow;
class HuggleFeed;
class EditQuery;
class ProcessorThread;
class WikiSite;
class WikiPage;
class WikiUser;
class WikiEdit;
class Message;

class Core
{
public:
    // Global variables
    static QDateTime StartupTime;
    static MainWindow *Main;
    static Login *f_Login;
    static QString HtmlHeader;
    static QString HtmlFooter;
    static HuggleFeed *PrimaryFeedProvider;
    static HuggleFeed *SecondaryFeedProvider;
    static QList<Query*> RunningQueries;
    //! This is basically a list of edits we went through, that
    //! can be deleted from memory anytime we want
    static QList<WikiEdit*> ProcessedEdits;
    //! This is a list of all edits that are being processed by some way
    //! whole list needs to be checked and probed everytime once a while
    static QList<WikiEdit*> ProcessingEdits;
    static ProcessorThread * Processor;
    static QList<Message*> Messages;
    static QList<EditQuery*> PendingMods;
    static bool Running;

#ifdef PYTHONENGINE
    static PythonEngine *Python;
#endif

    static void Init();
    static void Log(QString Message);
    static void DebugLog(QString Message, unsigned int Verbosity = 1);
    //! Helper function that will return URL of project in question
    static QString GetProjectURL(WikiSite Project);
    //! Return a full url like http://en.wikipedia.org/wiki/
    static QString GetProjectWikiURL(WikiSite Project);
    //! Return a script url like http://en.wikipedia.org/w/
    static QString GetProjectScriptURL(WikiSite Project);
    static QString GetProjectURL();
    //! Return a full url like http://en.wikipedia.org/wiki/
    static QString GetProjectWikiURL();
    //! Return a script url like http://en.wikipedia.org/w/
    static QString GetProjectScriptURL();
    static void ProcessEdit(WikiEdit *e);
    static void Shutdown();
    static QString RingLogToText();
    static void InsertToRingLog(QString text);
    static void DeveloperError();
    //! Save the local configuration to file
    static void SaveConfig();
    //! Load the local configuration from disk
    static void LoadConfig();
    //! Check the edit summary and similar in order to
    //! determine several edit attributes etc
    static void PreProcessEdit(WikiEdit *_e);
    //! Perform more expensive tasks to finalize
    //! edit processing
    static void PostProcessEdit(WikiEdit *_e);
    static void CheckQueries();
    //! Check if we can revert this edit
    static bool PreflightCheck(WikiEdit *_e);
    static ApiQuery *RevertEdit(WikiEdit* _e, QString summary = "", bool minor = false, bool rollback = true, bool keep = false);
    static QString GetCustomRevertStatus(QString RevertData);
    static bool ParseGlobalConfig(QString config);
    static bool ParseLocalConfig(QString config);
    static QString ConfigurationParse(QString key, QString content, QString missing = "");
    static void LoadDB();
    static bool SafeBool(QString value, bool defaultvalue = false);
    static QStringList ConfigurationParse_QL(QString key, QString content, bool CS = false);
    static QString Trim(QString text);
    static void DeleteEdit(WikiEdit *edit);
    // the mess bellow exist because of a way how huggle config stores the lists
    static QString GetSummaryOfWarningTypeFromWarningKey(QString key);
    static QString GetNameOfWarningTypeFromWarningKey(QString key);
    static QString GetKeyOfWarningTypeFromWarningName(QString id);
    //! Parse a part patterns for score words
    static void ParsePats(QString text);
    static void SaveDefs();
    static QString GetValueFromKey(QString item);
    static QString GetKeyFromValue(QString item);
    static void ParseWords(QString text);
    static Message *MessageUser(WikiUser *user, QString message, QString title, QString summary, bool section = true, Query *dependency = NULL);
    static void LoadDefs();
    static void FinalizeMessages();
    //! Get a level of warning from talk page
    static int GetLevel(QString page);
    static QString RetrieveTemplateToWarn(QString type);
    static EditQuery *EditPage(WikiPage *page, QString text, QString summary = "Edited using huggle", bool minor = false);
    static void AppendQuery(Query* item);
private:
    static QList<QString> *RingLog;
};

#endif // CORE_H
