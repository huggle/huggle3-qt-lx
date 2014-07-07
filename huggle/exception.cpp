//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exception.hpp"
#include <iostream>
#include <QDir>
#include "configuration.hpp"
#include "syslog.hpp"

#ifdef __linux__
    //linux code goes here
QString Breakpad_DumpPath = "/tmp";
#elif _WIN32
    // windows code goes here
QString Breakpad_DumpPath = QDir::tempPath();
#endif

using namespace Huggle;

#ifdef HUGGLE_BREAKPAD
google_breakpad::ExceptionHandler *Exception::GoogleBP_handler = NULL;
    #if _MSC_VER
        #pragma warning ( push )
        #pragma warning ( disable : 4100 )
    #else
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-parameter"
    #endif

#if HUGGLE_BREAKPAD == 0
    google_breakpad::MinidumpDescriptor *Exception::GoogleBP_descriptor = NULL;
    static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
    {
        std::cout << "Dump path: " << descriptor.path() << std::endl;
        return succeeded;
    }
#endif

#if HUGGLE_BREAKPAD == 1
    // windows
    const wchar_t kPipeName[] = L"\\\\.\\pipe\\BreakpadCrashServices\\Huggle";
    bool dumpCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo,
                      MDRawAssertionInfo* assertion, bool succeeded) {

        if (succeeded)
        {
            std::cout << "Dump generated in " + Breakpad_DumpPath.toStdString() << std::endl;
        } else
        {
            std::cout << "Failed to generate dump in " + Breakpad_DumpPath.toStdString() << std::endl;
        }
      return succeeded;
    }
#endif

    #if _MSC_VER
        #pragma warning ( pop )
    #else
        #pragma GCC diagnostic pop
    #endif
#endif

Exception::Exception(QString text, bool isRecoverable)
{
    std::cerr << "FATAL Exception thrown: " + text.toStdString() << std::endl;
    this->Message = text;
    this->ErrorCode = 2;
    this->Source = "{hidden}";
    this->_IsRecoverable = isRecoverable;
}

Exception::Exception(QString text, QString source, bool isRecoverable)
{
    this->construct(text, source, isRecoverable);
}

Exception::Exception(QString text, const char *source)
{
    this->construct(text, QString(source), true);
}

bool Exception::IsRecoverable() const
{
    return this->_IsRecoverable;
}

void Exception::construct(QString text, QString source, bool isRecoverable)
{
    std::cerr << "FATAL Exception thrown: " + text.toStdString() << std::endl;
    this->Source = source;
    this->Message = text;
    this->ErrorCode = 2;
    this->_IsRecoverable = isRecoverable;
}

void Exception::ThrowSoftException(QString Text, QString Source)
{
    if (Configuration::HuggleConfiguration->Verbosity > 0)
    {
        throw new Huggle::Exception(Text, Source);
    } else
    {
        Syslog::HuggleLogs->WarningLog("Soft exception: " + Text + " source: " + Source);
    }
}

void Exception::InitBreakpad()
{
#ifdef HUGGLE_BREAKPAD
    #if HUGGLE_BREAKPAD == 0
        // linux code
        Exception::GoogleBP_descriptor = new google_breakpad::MinidumpDescriptor("/tmp");
        Exception::GoogleBP_handler = new google_breakpad::ExceptionHandler(*Exception::GoogleBP_descriptor,  NULL,
                                                                             dumpCallback, NULL, true, -1);
    #endif
    #if HUGGLE_BREAKPAD == 1
        // windows code
        Exception::GoogleBP_handler = new google_breakpad::ExceptionHandler(Breakpad_DumpPath.toStdWString(), NULL, dumpCallback,
                                                                            NULL, google_breakpad::ExceptionHandler::HANDLER_ALL,
                                                                            MiniDumpNormal, kPipeName, NULL);
    #endif
#endif
}

void Exception::ExitBreakpad()
{
#ifdef HUGGLE_BREAKPAD
    #if HUGGLE_BREAKPAD == 0
        delete Exception::GoogleBP_descriptor;
    #endif
    delete Exception::GoogleBP_handler;
#endif
}


NullPointerException::NullPointerException(QString name, QString source) : Exception("", source)
{
    this->ErrorCode = 1;
    this->Message = QString("Null pointer exception. The variable you referenced (") + name + ") had null value.";
}
