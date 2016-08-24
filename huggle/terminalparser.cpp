//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "terminalparser.hpp"
#include <QFile>
#include <iostream>
#include "configuration.hpp"
#include "core.hpp"
#include "resources.hpp"
using namespace Huggle;
using namespace std;

TerminalParser::TerminalParser(QStringList argv)
{
    this->args = argv;
}

TerminalParser::TerminalParser(int argc, char *argv[])
{
    int i=0;
    while (i<argc)
    {
        this->args.append(QString(argv[i]));
        ++i;
    }
}

bool TerminalParser::Init()
{
    int x = 1;
    while (x < this->args.count())
    {
        //bool valid = false;
        QString text = this->args.at(x);
        ++x;
        if (text == "-h" || text == "--help")
        {
            DisplayHelp();
            return true;
        }
    }
    return false;
}

static void DisplayVersion()
{
    // version is stored in built in resource which we need to extract using call to core here
    Core::VersionRead();
    cout << QString("Huggle3 QT-LX " + hcfg->HuggleVersion).toStdString() << endl;
}

bool TerminalParser::Parse()
{
    int x = 1;
    while (x < this->args.count())
    {
        bool valid = false;
        QString text = this->args.at(x);
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
            Configuration::HuggleConfiguration->SystemConfig_SafeMode = true;
            valid = true;
        }
        if (text == "--language-test")
        {
            Configuration::HuggleConfiguration->SystemConfig_LanguageSanity = true;
            valid = true;
        }
        if (text == "--dot")
        {
            Configuration::HuggleConfiguration->SystemConfig_Dot = true;
            valid = true;
        }
        if (text == "--huggleinternal-update")
        {
            if (this->args.count() > x + 1 && !this->args.at(x + 1).startsWith("-"))
            {
                hcfg->UpdaterRoot = this->args.at(x + 1);
                hcfg->SystemConfig_UM = true;
                valid = true;
                x++;
            } else
            {
                cerr << "Parameter --huggleinternal-update requires an argument for it to work!" << endl;
                return true;
            }
        }
        if (text == "--version")
        {
            DisplayVersion();
            return true;
        }
        if (text == "--syslog")
        {
            Configuration::HuggleConfiguration->SystemConfig_Log2File = true;
            if (this->args.count() > x + 1 && !this->args.at(x + 1).startsWith("-"))
            {
                Configuration::HuggleConfiguration->SystemConfig_SyslogPath = this->args.at(x + 1);
                ++x;
            }
            valid = true;
        }
        if (text == "--qd")
        {
            Configuration::HuggleConfiguration->QueryDebugging = true;
            if (this->args.count() > x + 1 && !this->args.at(x + 1).startsWith("-"))
            {
                ++x;
                Configuration::HuggleConfiguration->QueryDebugPath = this->args.at(x);
            }
            valid = true;
        }
        if (text == "--chroot")
        {
            if (this->args.count() > x + 1 && !this->args.at(x + 1).startsWith("-"))
            {
                Configuration::HuggleConfiguration->HomePath = this->args.at(x + 1);
                valid = true;
                ++x;
            } else
            {
                cerr << "Parameter --chroot requires an argument for it to work!" << endl;
                return true;
            }
        }
        if (text == "--login-file")
        {
            if (this->args.count() > x + 1 && !this->args.at(x + 1).startsWith("-"))
            {
                QString path = this->args.at(x + 1);
                QFile *lf = new QFile(path);
                if (!lf->open(QIODevice::ReadOnly))
                {
                    delete lf;
                    cerr << "Unable to open: " << path.toStdString() << endl;
                    return true;
                }
                QString credentials = lf->readAll();
                lf->close();
                delete lf;
                //remove the newlines, the file is meant to contain 1 line only
                credentials.replace("\n", "");
                if (!credentials.contains(":"))
                {
                    cerr << "Unable to read the credential file, expected colon as a separator" << endl;
                    return true;
                }
                // we need to split it by first colon now
                hcfg->SystemConfig_BotPassword = true;
                hcfg->SystemConfig_BotLogin = credentials.mid(0, credentials.indexOf(":"));
                if (!hcfg->SystemConfig_BotLogin.contains("@"))
                {
                    cerr << "Only bot logins are allowed using this method. Username didn't have valid format (no @ symbol found)." << endl;
                    return true;
                }
                hcfg->SystemConfig_Username = hcfg->SystemConfig_BotLogin.mid(0, hcfg->SystemConfig_BotLogin.indexOf("@"));
                hcfg->TemporaryConfig_Password = credentials.mid(credentials.indexOf(":") + 1);
                valid = true;
                ++x;
            } else
            {
                cerr << "Parameter --login-file requires an argument for it to work!" << endl;
                return true;
            }
        }
        if (text == "--login")
        {
            valid = true;
            hcfg->Login = true;
        }
        if (text == "--fuzzy")
        {
            valid = true;
            hcfg->Fuzzy = true;
        }
        if (text == "--pylibs-dump")
        {
            cout << Resources::GetResource("/huggle/text/Resources/Python/definitions.py").toStdString() << endl;
            return true;
        }
        if (!valid)
        {
            if (!this->Silent)
            {
                cerr << QString("This parameter isn't valid: " + text).toStdString() << endl;
            }
            return true;
        }
        ++x;
    }
    return false;
}

bool TerminalParser::ParseChar(QChar x)
{
    switch (x.toLatin1())
    {
        case 'h':
            //help
            DisplayHelp();
            //quit
            return true;
        case 'v':
            hcfg->Verbosity++;
            return false;
        case 'V':
            DisplayVersion();
            return true;
    }
    return false;
}

void TerminalParser::DisplayHelp()
{
    if (this->Silent)
    {
        return;
    }
    cout << "Huggle 3 QT-LX\n\n"\
            "You can use following arguments to change the runtime settings:\n"\
            "  -v:              Increases verbosity\n"\
            "  --fuzzy:         Enable fuzzy localizations (these which were translated in past\n"\
            "                   but which were not updated since the source text has changed).\n"\
            "  --safe:          Start huggle in special mode where lot of stuff is skipped\n"\
            "                   during load\n"\
            "  --chroot <path>: Changes the home path of huggle to a given folder, so that huggle\n"\
            "                   reads a different configuration file and uses different data.\n"\
            "  --syslog [file]: Will write a logs to a file\n"\
            "  --version | -V:  Display a version\n"\
            "  --login:         Can be used in combination of --login-file only, this will tell huggle\n"\
            "                   to start login process immediately without letting you to change any login\n"\
            "                   preferences on login form\n"\
            "  --login-file:    Read a username and password from plain text file, separated by a colon\n"\
            "  -h | --help:     Display this help\n\n"\
            "Debugging options:\n"\
            "  --language-test: Will perform CPU expensive language test on startup, which reports\n"\
            "                   warnings found in localization files. This option is useful for\n"\
            "                   developers and people who create localization files\n"\
            "  --dot:           Debug on terminal only mode\n"\
            "  --qd [file]:     Write all transferred data to a file\n\n"\
            "Python related:\n"\
            "  --pylibs-dump:   Dump all built-in python libraries to std out\n"\
            "\nNote: every argument in [brackets] is optional\n"\
            "      but argument in <brackets> is required\n\n"\
            "Huggle is open source, contribute at https://github.com/huggle/huggle3-qt-lx" << endl;
}
