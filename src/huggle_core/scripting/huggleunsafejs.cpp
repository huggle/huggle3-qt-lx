//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "huggleunsafejs.hpp"
#include "script.hpp"
#include "../configuration.hpp"
#include "../syslog.hpp"
#include "../resources.hpp"
#include <QTextStream>
#include <QFile>

using namespace Huggle;

HuggleUnsafeJS::HuggleUnsafeJS(Script *s) : GenericJSClass(s)
{
    this->function_help.insert("read_file_to_string", "(string path): reads file into string");
    this->function_help.insert("read_file_to_list", "(string path): read a file as lines in a list");
    this->function_help.insert("write_string_to_file", "(string text, string file): writes text to a file");
    this->function_help.insert("append_string_to_file", "(string text, string file): appends text to file");
    this->function_help.insert("play_file", "(string file)");
    this->function_help.insert("sys_get_verbosity_level", "(): returns current verbosity level");
    this->function_help.insert("sys_set_verbosity_level", "(): changes current verbosity level");
    this->function_help.insert("set_cfg", "(string key, jsvalue data): changes system setting");
    this->function_help.insert("get_cfg", "(string key): return system setting");
}

bool HuggleUnsafeJS::append_string_to_file(QString text, QString file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly))
    {
        HUGGLE_ERROR(this->script->GetName() + ": append_string_to_file(text, file_path): unable to write to file: " + file_path);
        return false;
    }
    QTextStream st(&file);
    st << text;
    return true;
}

QHash<QString, QString> HuggleUnsafeJS::GetFunctions()
{
    return this->function_help;
}

QString HuggleUnsafeJS::read_file_to_string(QString file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        HUGGLE_ERROR(this->script->GetName() + ": read_file_to_string(file_path): Unable to read file: " + file_path);
        return QString();
    }
    return QString(file.readAll());
}

QList<QString> HuggleUnsafeJS::read_file_to_list(QString file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        HUGGLE_ERROR(this->script->GetName() + ": read_file_to_list(file_path): Unable to read file: " + file_path);
        return QList<QString>();
    }
    QString data = QString(file.readAll());
    return data.split("\n");
}

bool HuggleUnsafeJS::write_string_to_file(QString text, QString file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        HUGGLE_ERROR(this->script->GetName() + ": write_string_to_file(text, file_path): unable to write to file: " + file_path);
        return false;
    }
    QTextStream st(&file);
    st << text;
    return true;
}

void HuggleUnsafeJS::play_file(QString file)
{
    Resources::PlayExternalSoundFile(file);
}

int HuggleUnsafeJS::sys_get_verbosity_level()
{
    return static_cast<int>(hcfg->Verbosity);
}

void HuggleUnsafeJS::sys_set_verbosity_level(int verbosity_level)
{
    if (verbosity_level < 0)
        return;
    hcfg->Verbosity = static_cast<unsigned int>(verbosity_level);
}

QJSValue HuggleUnsafeJS::get_cfg(QString key)
{
    if (key == "ScoreDebug")
    {
        return QJSValue(hcfg->SystemConfig_ScoreDebug);
    }
    if (key == "Autorelog")
    {
        QJSValue(hcfg->SystemConfig_Autorelog);
    }
    if (key == "CacheHAN")
    {
        QJSValue(hcfg->SystemConfig_CacheHAN);
    }
    if (key == "CatScansAndWatched")
    {
        QJSValue(hcfg->SystemConfig_CatScansAndWatched);
    }
    if (key == "DelayVal")
    {
        QJSValue(hcfg->SystemConfig_DelayVal);
    }
    if (key == "HistorySize")
    {
        QJSValue(hcfg->SystemConfig_HistorySize);
    }
    return QJSValue();
}

bool HuggleUnsafeJS::set_cfg(QString key, QJSValue data)
{
    if (key == "ScoreDebug")
    {
        hcfg->SystemConfig_ScoreDebug = data.toBool();
        return true;
    }
    if (key == "Autorelog")
    {
        hcfg->SystemConfig_Autorelog = data.toBool();
        return true;
    }
    if (key == "CacheHAN")
    {
        hcfg->SystemConfig_CacheHAN = data.toInt();
        return true;
    }
    if (key == "CatScansAndWatched")
    {
        hcfg->SystemConfig_CatScansAndWatched = data.toBool();
        return true;
    }
    if (key == "DelayVal")
    {
        hcfg->SystemConfig_DelayVal = data.toUInt();
        return true;
    }
    if (key == "HistorySize")
    {
        hcfg->SystemConfig_HistorySize = data.toInt();
        return true;
    }
    return false;
}
