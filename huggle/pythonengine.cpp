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

PythonEngine::PythonEngine()
{
    Py_Initialize();
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
    this->Text = "";
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
            /// \todo here we probably want to write this to some log so that we know what is going on
            delete file;
            return false;
        }
        this->Text = QString(file->readAll());
        file->close();
        delete file;
        PyObject *name = PyString_FromString(this->Name.toUtf8().data());
        this->object = PyImport_Import(name);
        Py_DECREF(name);
        return true;
    }
    return false;
}

QString PythonScript::RetrieveText() const
{
    return this->Text;
}

#endif
