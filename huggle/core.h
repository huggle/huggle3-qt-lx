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

#include "configuration.h"
#include "wikiedit.h"
#include "mainwindow.h"

#ifdef PYTHONENGINE
#include "pythonengine.h"
#endif

#include <iostream>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QList>
#include <QString>
#include <QFile>
#include <QtXml>
#include <QMessageBox>

// Predeclaring some types
class Login;
class Query;
class ApiQuery;
class MainWindow;
class HuggleFeed;
class WikiSite;
class WikiPage;
class WikiUser;
class WikiEdit;

class Core
{
public:
    // Global variables
    static void Init();
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

#ifdef PYTHONENGINE
    static PythonEngine *Python;
#endif

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
    static ApiQuery *RevertEdit(WikiEdit* _e, QString summary = "", bool minor = false, bool rollback = true);
    static void LoadDB();
private:
    static QList<QString> *RingLog;
};

#endif // CORE_H
