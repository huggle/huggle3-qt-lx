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
#include "core.h"
#include "terminalparser.h"
#include "login.h"
#include "exception.h"

int main(int argc, char *argv[])
{
    try
    {
        QStringList args;
        int i=0;
        while (i<argc)
        {
            args.append(QString(argv[i]));
            i++;
        }
        // we create a new terminal parser
        TerminalParser *p = new TerminalParser(argc, args);
        // if parser get an argument which requires app to exit (like --help or --version)
        // we can terminate it now
        if (p->Parse())
        {
            delete p;
            return 0;
        }
        // otherwise we can delete it and continue
        delete p;
        p = NULL;
        // we load the core
        Core::Init();
        // now we can start the huggle :o
        QApplication a(argc, argv);
        Core::f_Login = new Login();
        Core::f_Login->show();

        return a.exec();
    } catch (Exception fail)
    {
        Core::Log("FATAL: Unhandled exception occured, description: " + fail.Message);
        return fail.ErrorCode;
    }
}
