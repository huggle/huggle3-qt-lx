//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exception.hpp"

#ifdef __linux__
    //linux code goes here
QString Breakpad_DumpPath = "/tmp";
#elif _WIN32
    // windows code goes here
QString Breakpad_DumpPath = QDir::tempPath();
#endif

using namespace Huggle;

#ifdef HUGGLE_BREAKPAD
google_breakpad::MinidumpDescriptor *Exception::GoogleBP_descriptor = NULL;
google_breakpad::ExceptionHandler *Exception::GoogleBP_handler = NULL;
//! \todo Fix this
    #if _MSC_VER
        #pragma warning ( push )
        #pragma warning ( disable )
    #else
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-parameter"
    #endif

    static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
    {
        std::cout << "Dump path: " << descriptor.path() << std::endl;
        return succeeded;
    }

    #if _MSC_VER
        #pragma warning ( pop )
    #else
        #pragma GCC diagnostic pop
    #endif
#endif

Exception::Exception(QString Text, bool __IsRecoverable)
{
    std::cerr << "FATAL Exception thrown: " + Text.toStdString() << std::endl;
    this->Message = Text;
    this->ErrorCode = 2;
    this->Source = "{hidden}";
    this->_IsRecoverable = __IsRecoverable;
}

Exception::Exception(QString Text, QString _Source, bool __IsRecoverable)
{
    std::cerr << "FATAL Exception thrown: " + Text.toStdString() << std::endl;
    this->Source = _Source;
    this->Message = Text;
    this->ErrorCode = 2;
    this->_IsRecoverable = __IsRecoverable;
}

bool Exception::IsRecoverable() const
{
    return this->_IsRecoverable;
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
                                                                            NULL, true, -1);
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
