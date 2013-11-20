//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "syslog.hpp"

using namespace Huggle;

Syslog *Syslog::HuggleLogs = new Syslog();

Syslog::Syslog()
{
    this->WriterLock = new QMutex(QMutex::Recursive);
}

Syslog::~Syslog()
{
    delete this->WriterLock;
}

void Syslog::Log(QString Message)
{
    Message = QDateTime::currentDateTime().toString() + "   " + Message;
    std::cout << Message.toStdString() << std::endl;
    this->InsertToRingLog(Message);
    this->lUnwrittenLogs.lock();
    this->UnwrittenLogs.append(Message);
    this->lUnwrittenLogs.unlock();
    if (Configuration::HuggleConfiguration->Log2File)
    {
        this->WriterLock->lock();
        QFile *file = new QFile(Configuration::HuggleConfiguration->SyslogPath);
        if (file->open(QIODevice::Append))
        {
            file->write(QString(Message + "\n").toUtf8());
            file->close();
        }
        delete file;
        this->WriterLock->unlock();
    }
}

QString Syslog::RingLogToText()
{
    int i = 0;
    QString text = "";
    while (i<this->RingLog.size())
    {
        text = this->RingLog.at(i) + "\n" + text;
        i++;
    }
    return text;
}

QStringList Syslog::RingLogToQStringList()
{
    return QStringList(this->RingLog);
}

void Syslog::InsertToRingLog(QString text)
{
    if (this->RingLog.size()+1 > Configuration::HuggleConfiguration->RingLogMaxSize)
    {
        this->RingLog.removeAt(0);
    }
    this->RingLog.append(text);
}

void Syslog::DebugLog(QString Message, unsigned int Verbosity)
{
    if (Configuration::HuggleConfiguration->Verbosity >= Verbosity)
    {
        this->Log("DEBUG[" + QString::number(Verbosity) + "]: " + Message);
    }
}
