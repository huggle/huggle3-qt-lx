//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "terminalparser.h"

using namespace std;

TerminalParser::TerminalParser(int argc_, QStringList argv)
{
    this->argc = argc_;
}

bool TerminalParser::Parse()
{
    int x = 0;
    while (x < this->args.count())
    {
        QString text = this->args.at(x);
        if (text == "-h" || text == "--help")
        {
            DisplayHelp();
            return true;
        }
        if (text.startsWith("-v"))
        {
            text = text.mid(1);
            while (text.length() > 0 && text.startsWith("v"))
            {
                Configuration::Verbosity++;
                text = text.mid(1);
            }
        }
        x++;
    }
    return false;
}

void TerminalParser::DisplayHelp()
{
    cout << "Huggle 3 QT-LX" << endl << endl;
    cout << "Parameters:" << endl;
    cout << "  -v: Increases verbosity" << endl;
    cout << "  -h | --help: Display this help" << endl<< endl;
    cout << "Huggle is open source, contribute at https://github.com/huggle/huggle3-qt-lx" << endl;
}
