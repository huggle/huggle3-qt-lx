//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// this is nasty hack to ensure that Python.h is included first
// see http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif
// we can finally include the normal, unbroken headers now
#include <QApplication>
#include <QStringList>
#include <QString>
#include "configuration.hpp"
#include "syslog.hpp"
#include "core.hpp"
#include "terminalparser.hpp"
#include "login.hpp"
#include "exception.hpp"

//! This function just read the parameters and return true if we can continue or not
bool TerminalParse(int argc, char *argv[])
{
    QStringList args;
    int i=0;
    while (i<argc)
    {
        args.append(QString(argv[i]));
        i++;
    }
    // we create a new terminal parser
    Huggle::TerminalParser *p = new Huggle::TerminalParser(args);
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
                                         + "\nSource: " + fail->Source);
    delete Huggle::Core::HuggleCore;
    Huggle::Exception::ExitBreakpad();
    return fail->ErrorCode;
}

int main(int argc, char *argv[])
{
    int ReturnCode = 0;
    Huggle::Exception::InitBreakpad();
    Huggle::HgApplication a(argc, argv);
    QApplication::setApplicationName("Huggle");
    QApplication::setOrganizationName("Wikimedia");
    Huggle::Configuration::HuggleConfiguration = new Huggle::Configuration();
    try
    {
        // check if arguments don't need to exit program
        if (!TerminalParse(argc, argv))
        {
            Huggle::Exception::ExitBreakpad();
            delete Huggle::Configuration::HuggleConfiguration;
            return 0;
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

