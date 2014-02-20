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
using namespace Huggle::Python;

// let's define huggle api for python here
namespace Huggle
{
    namespace Python
    {
// Disable compiler warnings because we always need to use unused parameters when we use python
#if _MSC_VER
#pragma warning ( push )
#pragma warning ( disable )
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
        static PyObject *ApiVersion(PyObject *self, PyObject *args)
        {
            PyObject *v = PyUnicode_FromString(Configuration::HuggleConfiguration->HuggleVersion.toUtf8().data());
            return v;
        }
#if _MSC_VER
#pragma warning ( pop )
#else
#pragma GCC diagnostic pop
#endif

        static PyMethodDef Methods[] = {
            {"huggle_version", ApiVersion, METH_VARARGS,
             "Return a huggle version"},
            {NULL, NULL, 0, NULL}
        };

        static PyModuleDef Module = {
            PyModuleDef_HEAD_INIT, "huggle", NULL, -1, Methods,
            NULL, NULL, NULL, NULL
        };

        static PyObject *PyInit_emb()
        {
            return PyModule_Create(&Module);
        }

        static bool ContainsFunction(QString function, QString string)
        {
            QStringList lines = string.split('\n');
            QRegExp *regex_ = new QRegExp("^[\\s\\t]*def " + function + ".*:[\\s\\t]*$");
            int ln = 0;
            while (ln < lines.count())
            {
                if (lines.at(ln).contains(*regex_))
                {
                    delete regex_;
                    return true;
                }
                ln++;
            }

            delete regex_;
            return false;
        }
    }
}

PythonEngine::PythonEngine(QString ExtensionsFolder_)
{
    Py_Initialize();
    // define hooks
    PyImport_AppendInittab("huggle", &PyInit_emb);
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

PyObject *PythonScript::Hook(QString function)
{
    PyObject *ptr_python_ = NULL;
    if (Huggle::Python::ContainsFunction(function, this->SourceCode))
    {
        Syslog::HuggleLogs->DebugLog("Loading hook symbols of " + function + " " + this->Name, 2);
        ptr_python_ = PyObject_GetAttrString(this->object, function.toUtf8().data());
        if (ptr_python_ != NULL && !PyCallable_Check(ptr_python_))
        {
            // we loaded the symbol but it's not callable function
            // so we remove it
            Py_DECREF(ptr_python_);
            Syslog::HuggleLogs->WarningLog("Function " + function + "@" + this->Name
                                           + " isn't callable, hook is disabled now");
            ptr_python_ = NULL;
        } else if (ptr_python_ == NULL)
        {
            Syslog::HuggleLogs->DebugLog("There is no override for " + function);
        }
    }
    return ptr_python_;
}

QString PythonScript::GetName() const
{
    return this->Name;
}

bool PythonScript::IsEnabled() const
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
        Syslog::HuggleLogs->DebugLog("Calling hook Hook_MainWindowIsLoaded @" + this->Name, 2);
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
        PyObject *name = PyUnicode_FromString(ModuleName.toUtf8().data());
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
        this->ptr_Hook_MainLoaded = this->Hook("hook_main_window_is_loaded");

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
