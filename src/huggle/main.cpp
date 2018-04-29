//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// IMPORTANT: Since Huggle 3.4.0 this is just an entry point for dynamic libraries
//            that huggle consists of (huggle_core, huggle_ui etc).

#include <huggle_core/definitions.hpp>

#include <QApplication>
#include <QStringList>
#include <QString>
#include <huggle_core/configuration.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/terminalparser.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_res/huggle_res.hpp>
#include <huggle_l10n/huggle_l10n.hpp>
#include <huggle_ui/hgapplication.hpp>
#include <huggle_ui/uiexceptionhandler.hpp>
#include <huggle_ui/uiscript.hpp>
#include <huggle_ui/loginform.hpp>
#include <huggle_ui/updateform.hpp>

// This is needed for MSVC to turn this to Windows application, otherwise console window
// would be displayed during run of program
#ifdef _MSC_VER
#    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

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
        // We need to create terminal parser now and rest later, because rest of program depends on X11,
        // and that would result in weird errors on systems that don't have graphical frontend.
        // Using huggle on such is not supported yet, but there are plans for headless version
        Huggle::TerminalParser *parser = new Huggle::TerminalParser(argc, argv);
        if (parser->Init())
        {
            delete parser;
            Huggle::Exception::ExitBreakpad();
            return ReturnCode;
        }
        Huggle::HgApplication a(argc, argv);
        // Make it possible to select text from message boxens
        a.setStyleSheet("QMessageBox { messagebox-text-interaction-flags: 5; }");
        QApplication::setApplicationName("Huggle");
        QApplication::setOrganizationName("Wikimedia");
        #if QT_VERSION >= 0x050100
            // enable HiDPI support (available since Qt 5.1, but off by default)
            a.setAttribute(Qt::AA_UseHighDpiPixmaps);
        #endif

        // We must create config before we run terminal parser, because some config options may be altered using it
        Huggle::Configuration::HuggleConfiguration = new Huggle::Configuration();

        // Parse() returns true in case program should terminate
        if (parser->Parse())
        {
            Huggle::Exception::ExitBreakpad();
            delete Huggle::Configuration::HuggleConfiguration;
            delete parser;
            return ReturnCode;
        }
        // We don't need parser anymore so let's free the memory
        delete parser;

        if (hcfg->SystemConfig_UM)
        {
            // we start huggle in updater mode, so that it performs some updates which needs to be done in separate process
            Huggle::UpdateForm *update_form = new Huggle::UpdateForm();
            update_form->show();
            ReturnCode = a.exec();
            delete update_form;
            return ReturnCode;
        }

        // Load the external resource files, needed only on windows, because of shitties dynamic linker ever
        #ifdef HUGGLE_WIN
            Huggle::Huggle_l10n::Init();
            Huggle::Huggle_Res::Init();
        #endif

        // Load the core which manages lof of stuff like GC, exception handling and some internal stuff
        Huggle::Core::HuggleCore = new Huggle::Core();
        Huggle::Core::HuggleCore->Init();

        // Install graphical exception handler, so we get a nice window on exception, instead of some console error
        Huggle::Core::HuggleCore->InstallNewExceptionHandler(new Huggle::UiExceptionHandler());

        // Load all UI scripts
        Huggle::UiScript::Autostart();

        // Start the huggle by creating the login form, this form will delete itself after it's closed to save RAM, so it should be instantiated dynamically, and shouldn't be deleted
        Huggle::LoginForm *login_form = new Huggle::LoginForm();
        login_form->show();
        login_form->setAttribute(Qt::WA_DeleteOnClose);

        // Event loop
        ReturnCode = a.exec();
        if (Huggle::Core::HuggleCore && Huggle::Core::HuggleCore->Running)
            Huggle::Core::HuggleCore->Shutdown();
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
