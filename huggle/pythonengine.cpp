//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"

#ifdef PYTHONENGINE

#include "pythonengine.hpp"

using namespace Huggle;

PythonEngine::PythonEngine(QString ExtensionsFolder_)
{
    Py_Initialize();
    PyRun_SimpleString(QString("import sys; sys.path.append('" + ExtensionsFolder_ + "')").toUtf8().data());
}

bool PythonEngine::LoadScript(QString path)
{
    PythonScript *p = new PythonScript(path);
    this->Scripts.append(p);
    if (p->Init())
    {
        return true;
    }
    return false;
}

PythonScript::PythonScript(QString name)
{
    this->SourceCode = "";
    this->Name = name;
    this->object = NULL;
    this->Enabled = false;
}

QString PythonScript::GetName() const
{
    return this->Name;
}

bool PythonScript::GetEnabled() const
{
    return this->Enabled;
}

void PythonScript::SetEnabled(bool value)
{
    this->Enabled = value;
}

bool PythonScript::Init()
{
    QFile *file = new QFile(this->Name);
    if (QFile().exists(this->Name))
    {
        if (!file->open(QIODevice::ReadOnly))
        {
            Huggle::Syslog::HuggleLogs->ErrorLog("Unable to open " + this->Name);
            delete file;
            return false;
        }
        this->SourceCode = QString(file->readAll());
        file->close();
        delete file;
        QString ModuleName = (this->Name.toUtf8().data());
        if (ModuleName.contains("/"))
        {
            ModuleName = ModuleName.mid(ModuleName.indexOf("/") + 1);
            ModuleName = ModuleName.replace(".py", "");
        }
        PyObject *name = PyString_FromString(ModuleName.toUtf8().data());
        this->object = PyImport_Import(name);
        if (this->object == NULL)
        {
            PyErr_Print();
            Syslog::HuggleLogs->WarningLog("Unable to load " + this->Name);
            return false;
        }
        Py_DECREF(name);
        return true;
    }
    delete file;
    return false;
}

QString PythonScript::RetrieveSourceCode() const
{
    return this->SourceCode;
}

#endif
