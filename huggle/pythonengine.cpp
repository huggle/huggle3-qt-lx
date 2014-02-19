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
    // define hooks
    this->Methods[0].ml_doc = "Returns the version of huggle.";
    this->Methods[0].ml_flags = METH_VARARGS;
    this->Methods[0].ml_meth = api_Version;
    this->Methods[0].ml_name = "huggle_version";
    Py_InitModule("huggle", Methods);
    PyRun_SimpleString(QString("import sys; sys.path.append('" + ExtensionsFolder_ + "')").toUtf8().data());
}

bool PythonEngine::LoadScript(QString path)
{
    PythonScript *p = new PythonScript(path);
    this->Scripts.append(p);
    if (p->Init())
    {
        p->SetEnabled(true);
        return true;
    }
    return false;
}

void PythonEngine::Hook_MainWindowIsLoaded()
{
    int x = 0;
    while (x < this->Scripts.count())
    {
        this->Scripts.at(x)->Hook_MainWindowIsLoaded();
        x++;
    }
}

PyObject *PythonEngine::api_Version(PyObject *self, PyObject *args)
{
    return PyString_FromString(Configuration::HuggleConfiguration->HuggleVersion.toUtf8().data());
}

PythonScript::PythonScript(QString name)
{
    this->SourceCode = "";
    this->Name = name;
    this->object = NULL;
    this->ptr_Hook_MainLoaded = NULL;
    this->Enabled = false;
}

PythonScript::~PythonScript()
{
    if (this->ptr_Hook_MainLoaded != NULL)
    {
        Py_DECREF(this->ptr_Hook_MainLoaded);
    }
    if (this->object != NULL)
    {
        Py_DECREF(this->object);
    }
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

void PythonScript::Hook_MainWindowIsLoaded()
{
    if (this->ptr_Hook_MainLoaded)
    {
        PyObject_CallObject(this->ptr_Hook_MainLoaded, NULL);
    }
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
        if (name == NULL)
        {
            PyErr_Print();
            return false;
        }
        this->object = PyImport_Import(name);
        // remove name
        Py_DECREF(name);
        if (this->object == NULL)
        {
            PyErr_Print();
            return false;
        }
        Syslog::HuggleLogs->DebugLog("Loading hook symbols for python " + this->Name);
        // load symbols for hooks now
        this->ptr_Hook_MainLoaded = PyObject_GetAttrString(this->object, "hook_main_window_is_loaded");
        if (this->ptr_Hook_MainLoaded != NULL && !PyCallable_Check(this->ptr_Hook_MainLoaded))
        {
            // we loaded the symbol but it's not callable function
            // so we remove it
            Py_DECREF(this->ptr_Hook_MainLoaded);
            Syslog::HuggleLogs->WarningLog("Function hook_main_window_is_loaded of " + this->Name
                                           + " isn't callable, hook is disabled now");
            this->ptr_Hook_MainLoaded = NULL;
        }
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
