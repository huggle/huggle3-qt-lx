//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef SYSLOG_HPP
#define SYSLOG_HPP

#include "definitions.hpp"

#include <QStringList>

// This macro should be used for all debug messages which are frequently called, so that we don't call DebugLog(QString, uint)
// when we aren't in debug mode, this saves some CPU resources as these calls are very expensive sometimes (lot of conversions
// of numbers and text)
//! Sends a debug to system (text of a log, verbosity)
#define HUGGLE_DEBUG(debug, verbosity) if (Huggle::Configuration::HuggleConfiguration->Verbosity >= verbosity) \
                                           Huggle::Syslog::HuggleLogs->DebugLog(debug, verbosity)

#define HUGGLE_DEBUG1(debug) if (Huggle::Configuration::HuggleConfiguration->Verbosity >= 1) \
                                           Huggle::Syslog::HuggleLogs->DebugLog(debug, 1)

#ifdef HUGGLE_EXTENSION
//! Sends a debug to system as extension, this can only be used in scope of extension
#define HUGGLE_EXDEBUG(debug, verbosity) if (Huggle::Configuration::HuggleConfiguration->Verbosity >= verbosity) \
                       Huggle::Syslog::HuggleLogs->DebugLog(this->GetExtensionName() + ": " + debug, verbosity)
#define HUGGLE_EXDEBUG1(debug) if (Huggle::Configuration::HuggleConfiguration->Verbosity >= 1) \
                         Huggle::Syslog::HuggleLogs->DebugLog(this->GetExtensionName() + ": " + debug, 1)
#endif

#define HUGGLE_WARNING(text)   Huggle::Syslog::HuggleLogs->WarningLog(text);
#define HUGGLE_LOG(text)       Huggle::Syslog::HuggleLogs->Log(text);
#define HUGGLE_ERROR(text)     Huggle::Syslog::HuggleLogs->ErrorLog(text);

class QMutex;

namespace Huggle
{
    enum HuggleLogType
    {
        HuggleLogType_Normal,
        HuggleLogType_Error,
        HuggleLogType_Debug,
        HuggleLogType_Warn
    };

    //! Line of log
    class HUGGLE_EX HuggleLog_Line
    {
        public:
            HuggleLog_Line(HuggleLog_Line *line);
            HuggleLog_Line(const HuggleLog_Line &line);
            HuggleLog_Line(QString text, QString date);
            QString Text;
            QString Date;
            HuggleLogType Type;
    };

    //! Provides a logging to various places

    //! There is an instance of this class that can be used to log from external modules using same facility
    class HUGGLE_EX Syslog
    {
        public:
            static Syslog *HuggleLogs;

            Syslog();
            virtual ~Syslog();
            //! Write text to terminal as well as ring log
            /*!
             * \param Message Message to log
             */
            virtual void Log(QString Message, bool TerminalOnly = false, HuggleLogType Type = HuggleLogType_Normal);
            virtual void ErrorLog(QString Message, bool TerminalOnly = false);
            virtual void WarningLog(QString Message, bool TerminalOnly = false);
            //! This log is only shown if verbosity is same or larger than requested verbosity
            virtual void DebugLog(QString Message, unsigned int Verbosity = 1);
            //! Return a ring log represented as 1 huge string
            QString RingLogToText();
            /*!
             * \brief Return a ring log as qstring list
             * \return QStringList
             */
            QStringList RingLogToQStringList();
            void InsertToRingLog(HuggleLog_Line line);
            QList<HuggleLog_Line> RingLogToList();
            //! This is a list of logs that needs to be written, it exist so that logs can be written from
            //! other threads as well, writing to syslog from other thread would crash huggle
            QList<HuggleLog_Line> UnwrittenLogs;
            //! Mutex we lock unwritten logs with so that only 1 thread can write to it
            QMutex *lUnwrittenLogs;
            bool EnableLogWriteBuffer;
        protected:
            //! Ring log is a buffer that contains system messages
            QList<HuggleLog_Line> RingLog;
            //! Everytime we write to a file we need to lock this
            QMutex *WriterLock;
    };
}

#endif // SYSLOG_HPP
