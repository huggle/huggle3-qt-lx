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
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QStringList>
#include <QMutex>

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
    class HuggleLog_Line
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
    class Syslog
    {
        public:
            static Syslog *HuggleLogs;

            Syslog();
            ~Syslog();
            //! Write text to terminal as well as ring log
            /*!
             * \param Message Message to log
             */
            void Log(QString Message, bool TerminalOnly = false, HuggleLogType Type = HuggleLogType_Normal);
            void ErrorLog(QString Message, bool TerminalOnly = false);
            void WarningLog(QString Message, bool TerminalOnly = false);
            //! This log is only shown if verbosity is same or larger than requested verbosity
            void DebugLog(QString Message, unsigned int Verbosity = 1);
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
            QMutex lUnwrittenLogs;
        private:
            //! Ring log is a buffer that contains system messages
            QList<HuggleLog_Line> RingLog;
            //! Everytime we write to a file we need to log this
            QMutex *WriterLock;
    };
}

#endif // SYSLOG_HPP
