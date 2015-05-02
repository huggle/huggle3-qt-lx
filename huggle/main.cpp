//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLE_LIBRARY
#include "definitions.hpp"

#include <QApplication>
#include <QStringList>
#include <QString>
#include "configuration.hpp"
#include "core.hpp"
#include "syslog.hpp"
#include "updateform.hpp"
#include "terminalparser.hpp"
#include "login.hpp"
#include "exception.hpp"
#ifdef _MSC_VER
#    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

//! This function just read the parameters and return true if we can continue or not
bool TerminalParse(Huggle::TerminalParser *p)
{
    // if parser get an argument which requires app to exit (like --help or --version)
    // we can terminate it now
    if (p->Parse())
    {
        delete p;
        return false;
    }
    // otherwise we can delete it and continue
    delete p;
    return true;
}

int Fatal(Huggle::Exception *fail)
{
    Huggle::Syslog::HuggleLogs->ErrorLog("FATAL: Unhandled exception occured, description: " + fail->Message
                                         + "\nSource: " + fail->Source + "\nStack: " + fail->StackTrace);
    delete Huggle::Core::HuggleCore;
    Huggle::Exception::ExitBreakpad();
    return fail->ErrorCode;
}

int main(int argc, char *argv[])
{
    int ReturnCode = 0;
    Huggle::Exception::InitBreakpad();
    try
    {
        // we need to create terminal parser now and rest later
        Huggle::TerminalParser *parser = new Huggle::TerminalParser(argc, argv);
        // we need to do this before we init the qapp because otherwise it would not work on systems
        // that don't have an X org
        if (parser->Init())
        {
            delete parser;
            Huggle::Exception::ExitBreakpad();
            return ReturnCode;
        }
        Huggle::HgApplication a(argc, argv);
        QApplication::setApplicationName("Huggle");
        QApplication::setOrganizationName("Wikimedia");
        Huggle::Configuration::HuggleConfiguration = new Huggle::Configuration();
        // check if arguments don't need to exit program
        if (!TerminalParse(parser))
        {
            Huggle::Exception::ExitBreakpad();
            delete Huggle::Configuration::HuggleConfiguration;
            return ReturnCode;
        }
        if (hcfg->SystemConfig_UM)
        {
            // we start huggle in updater mode, so that it performs some updates
            // which needs to be done in separate process
            Huggle::UpdateForm *update_form = new Huggle::UpdateForm();
            update_form->show();
            ReturnCode = a.exec();
            delete update_form;
            return ReturnCode;
        }
        // we load the core
        Huggle::Core::HuggleCore = new Huggle::Core();
        Huggle::Core::HuggleCore->Init();
        // now we can start the huggle :o
        Huggle::Core::HuggleCore->fLogin = new Huggle::Login();
        Huggle::Core::HuggleCore->fLogin->show();
        ReturnCode = a.exec();
        delete Huggle::Core::HuggleCore;
        Huggle::Exception::ExitBreakpad();
        return ReturnCode;
    } catch (Huggle::Exception *fail)
    {
        ReturnCode = Fatal(fail);
        delete fail;
        return ReturnCode;
    } catch (Huggle::Exception& fail)
    {
        return Fatal(&fail);
    }
}

#endif
