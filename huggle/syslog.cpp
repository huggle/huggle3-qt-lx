//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "syslog.hpp"
#include <iostream>
#include <QMutex>
#include <QDateTime>
#include <QFile>
#include "configuration.hpp"

using namespace Huggle;

Syslog *Syslog::HuggleLogs = new Syslog();

Syslog::Syslog()
{
    this->WriterLock = new QMutex(QMutex::Recursive);
    this->lUnwrittenLogs = new QMutex(QMutex::Recursive);
#ifndef HUGGLE_SDK
    this->EnableLogWriteBuffer = true;
#else
    this->EnableLogWriteBuffer = false;
#endif
}

Syslog::~Syslog()
{
    delete this->lUnwrittenLogs;
    delete this->WriterLock;
}

void Syslog::Log(QString Message, bool TerminalOnly, HuggleLogType Type)
{
    QString d = QDateTime::currentDateTime().toString();
    QString message = d + "   " + Message;
    if (Type == HuggleLogType_Error)
    {
        std::cerr << message.toStdString() << std::endl;
    } else
    {
        std::cout << message.toStdString() << std::endl;
    }
    HuggleLog_Line line(Message, d);
    line.Type = Type;
    if (!TerminalOnly)
    {
        this->InsertToRingLog(line);
        if (this->EnableLogWriteBuffer)
        {
            this->lUnwrittenLogs->lock();
            this->UnwrittenLogs.append(line);
            this->lUnwrittenLogs->unlock();
        }
        if (Configuration::HuggleConfiguration->SystemConfig_Log2File)
        {
            this->WriterLock->lock();
            QFile *file = new QFile(Configuration::HuggleConfiguration->SystemConfig_SyslogPath);
            if (file->open(QIODevice::Append))
            {
                file->write(QString(message + "\n").toUtf8());
                file->close();
            }
            delete file;
            this->WriterLock->unlock();
        }
    }
}

void Syslog::ErrorLog(QString Message, bool TerminalOnly)
{
    this->Log("ERROR: " + Message, TerminalOnly, HuggleLogType_Error);
}

void Syslog::WarningLog(QString Message, bool TerminalOnly)
{
    this->Log("WARNING: " + Message, TerminalOnly, HuggleLogType_Warn);
}

QString Syslog::RingLogToText()
{
    int i = 0;
    QString text = "";
    while (i<this->RingLog.size())
    {
        text = this->RingLog.at(i).Date + ": " + this->RingLog.at(i).Text + "\n" + text;
        i++;
    }
    return text;
}

QStringList Syslog::RingLogToQStringList()
{
    QStringList list;
    int i = 0;
    while (i<this->RingLog.size())
    {
        QString text = this->RingLog.at(i).Date + ": " + this->RingLog.at(i).Text;
        list.append(text);
        i++;
    }
    return list;
}

void Syslog::InsertToRingLog(HuggleLog_Line line)
{
    if (this->RingLog.size()+1 > Huggle::Configuration::HuggleConfiguration->SystemConfig_RingLogMaxSize)
    {
        this->RingLog.removeAt(0);
    }
    this->RingLog.append(line);
}

QList<HuggleLog_Line> Syslog::RingLogToList()
{
    QList<HuggleLog_Line> list;
    list << this->RingLog;
    return list;
}

void Syslog::DebugLog(QString Message, unsigned int Verbosity)
{
    if (Huggle::Configuration::HuggleConfiguration->Verbosity >= Verbosity)
    {
        this->Log("DEBUG[" + QString::number(Verbosity) + "]: " + Message,
                  Huggle::Configuration::HuggleConfiguration->SystemConfig_Dot,
                  Huggle::HuggleLogType_Debug);
    }
}

HuggleLog_Line::HuggleLog_Line(HuggleLog_Line *line)
{
    this->Type = line->Type;
    this->Date = line->Date;
    this->Text = line->Text;
}

HuggleLog_Line::HuggleLog_Line(const HuggleLog_Line &line)
{
    this->Type = line.Type;
    this->Date = line.Date;
    this->Text = line.Text;
}

HuggleLog_Line::HuggleLog_Line(QString text, QString date)
{
    this->Type = HuggleLogType_Normal;
    this->Text = text;
    this->Date = date;
}

