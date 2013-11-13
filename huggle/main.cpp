//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include <QApplication>
#include <QStringList>
#include <QString>
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
    Huggle::TerminalParser *p = new Huggle::TerminalParser(argc, args);
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

int main(int argc, char *argv[])
{
    try
    {
        // check if arguments don't need to exit program
        if (!TerminalParse(argc, argv))
        {
            return 0;
        }
        // we load the core
        Huggle::Core::HuggleCore = new Huggle::Core();
        Huggle::Core::HuggleCore->Init();
        // now we can start the huggle :o
        Huggle::HgApplication a(argc, argv);
        Huggle::Core::HuggleCore->f_Login = new Huggle::Login();
        Huggle::Core::HuggleCore->f_Login->show();

        int rv = a.exec();
        delete Huggle::Core::HuggleCore;
        return rv;
    } catch (Huggle::Exception fail)
    {
        Huggle::Core::HuggleCore->Log("FATAL: Unhandled exception occured, description: " + fail.Message);
        delete Huggle::Core::HuggleCore;
        return fail.ErrorCode;
    }
}

