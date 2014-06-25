//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef TERMINALPARSER_H
#define TERMINALPARSER_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// https://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QStringList>
#include <QString>

namespace Huggle
{
    //! Parses the data provided by user
    class TerminalParser
    {
        public:
            TerminalParser(int argc, char *argv[]);
            TerminalParser(QStringList argv);
            bool Init();
            bool Parse();
            bool ParseChar(QChar x);
            void DisplayHelp();
            bool Silent = false;
        private:
            QStringList args;
    };
}

#endif // TERMINALPARSER_H
