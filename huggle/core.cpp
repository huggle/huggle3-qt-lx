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
        "td.diff-context{background:#eeeeee;font-size:smaller}.diffchange{color:silver;font-weight:bold;text-decoration:underline}"\
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

void Core::Init()
{
    Core::Log("Huggle 3 QT-LX, version " + Configuration::HuggleVersion);
    Core::Log("Loading configuration");
    Core::LoadConfig();
#ifdef PYTHONENGINE
    Core::Log("Loading python engine");
    Core::Python = new PythonEngine();
#endif
    Core::DebugLog("Loading wikis");
    Core::LoadDB();
}

void Core::LoadDB()
{
    Configuration::ProjectList << Configuration::Project;
    // this is a temporary only for oauth testing
    Configuration::ProjectList << WikiSite("testwiki", "test.wikipedia.org/");
    Configuration::ProjectList << WikiSite("mediawiki","www.mediawiki.org/");
    if (QFile::exists(Configuration::WikiDB))
    {
        QFile db(Configuration::WikiDB);
        if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            Core::Log("ERROR: Unable to read " + Configuration::WikiDB);
            return;
        }
        QDomDocument *d = new QDomDocument();
        d->setContent(&db);
    }
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
#ifdef PYTHONENGINE
    Core::Log("Unloading python");
    delete Core::Python;
#endif
    delete Core::f_Login;
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

}

void Core::PostProcessEdit(WikiEdit *_e)
{
    _e->PostProcess();
    Core::ProcessingEdits.append(_e);
}

void Core::CheckQueries()
{
     //  <?xml version="1.0"?><api servedby="mw1128"><error code="alreadyrolled" info="The page you tried to rollback was already rolled back" /></api>
    int curr = 0;
    if (Core::RunningQueries.count() == 0)
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
            Core::DebugLog("Query finished with: " + q->Result->Data);
            q->CustomStatus = Core::GetCustomRevertStatus(q->Result->Data);
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
        delete item;
        curr++;
    }
}

bool Core::PreflightCheck(WikiEdit *_e)
{
    return true;
}

ApiQuery *Core::RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback)
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
            token = token.mid(0, token.indexOf("+")) + "%2B\\";
        }
        query->Parameters = "title=" + QUrl::toPercentEncoding(_e->Page->PageName)
                + "&token=" + token
                + "&user=" + QUrl::toPercentEncoding(_e->User->Username)
                + "&summary=" + QUrl::toPercentEncoding(summary);
        query->UsingPOST = true;
        DebugLog("Rolling back " + _e->Page->PageName + " using token " + token);
        query->Process();
        Core::RunningQueries.append(query);
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
    DebugLog(config);
    return true;
}

bool Core::ParseLocalConfig(QString config)
{
    return true;
}

