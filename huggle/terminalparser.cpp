//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "terminalparser.hpp"
using namespace Huggle;
using namespace std;

TerminalParser::TerminalParser(int argc_, QStringList argv)
{
    this->argc = argc_;
    this->Silent = false;
    this->args = argv;
}

bool TerminalParser::Parse()
{
    int x = 1;
    while (x < this->args.count())
    {
        bool valid = false;
        QString text = this->args.at(x);
        if (text == "-h" || text == "--help")
        {
            DisplayHelp();
            return true;
        }
        if (!text.startsWith("--") && text.startsWith("-"))
        {
            text = text.mid(1);
            while (text.length())
            {
                if (this->ParseChar(text.at(0)))
                {
                    return true;
                }
                text = text.mid(1);
            }
            valid = true;
        }
        if (text == "--safe")
        {
            Configuration::HuggleConfiguration->_SafeMode = true;
            valid = true;
        }
        if (!valid)
        {
            if (!Silent)
            {
                cout << (QString("This parameter isn't valid: ") + text).toStdString() << endl;
            }
            return true;
        }
        x++;
    }
    return false;
}

bool TerminalParser::ParseChar(QChar x)
{
    switch (x.toAscii())
    {
        case 'v':
            Configuration::HuggleConfiguration->Verbosity++;
            return false;
        case 'h':
            this->DisplayHelp();
            return true;
    }
    return false;
}

void TerminalParser::DisplayHelp()
{
    if (Silent)
    {
        return;
    }
    cout << "Huggle 3 QT-LX" << endl << endl;
    cout << "Parameters:" << endl;
    cout << "  -v: Increases verbosity" << endl;
    cout << "  --safe: Start huggle in special mode where lot of stuff is skipped during load" << endl;
    cout << "  -h | --help: Display this help" << endl<< endl;
    cout << "Huggle is open source, contribute at https://github.com/huggle/huggle3-qt-lx" << endl;
}
