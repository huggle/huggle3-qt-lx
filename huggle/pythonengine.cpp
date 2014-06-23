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
#include "configuration.hpp"
#include "exception.hpp"
#include "syslog.hpp"

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

        static PyObject *DebugLog(PyObject *self, PyObject *args)
        {
            PyObject *py_verbosity_ = NULL;
            PyObject *text_ = NULL;
            PyObject *result_ = PyBool_FromLong(0);
            if (PyArg_UnpackTuple(args, "debug_log", 1, 2, &text_, &py_verbosity_) && PyBytes_Check(text_))
            {
                Py_DECREF(result_);
                result_ = PyBool_FromLong(1);
                unsigned int verbosity = 1;
                if (py_verbosity_ != NULL && PyLong_Check(py_verbosity_))
                {
                    verbosity = (unsigned int)PyLong_AsLong(py_verbosity_);
                    Py_DECREF(py_verbosity_);
                }
                if (verbosity < 1)
                {
                    verbosity = 1;
                }
                PyObject *uni_ = PyUnicode_AsUTF8String(text_);
                Py_DECREF(text_);
                if (uni_ == NULL || !PyBytes_Check(uni_))
                {
                    Syslog::HuggleLogs->DebugLog("Log@unkown: parameter text must be of a string type");
                } else
                {
                    QString qs_text_(PyBytes_AsString(uni_));
                    Syslog::HuggleLogs->DebugLog(qs_text_, verbosity);
                    Py_DECREF(uni_);
                }
            }
            return result_;
        }

        static PyObject *Log_(HuggleLogType log_type, PyObject *self, PyObject *args)
        {
            PyObject *text_ = NULL;
            PyObject *result_ = PyBool_FromLong(0);
            QString fc_name_;
            switch (log_type)
            {
                case HuggleLogType_Error:
                    fc_name_ = "error_log";
                    break;
                case HuggleLogType_Warn:
                    fc_name_ = "warn_log";
                    break;
                default:
                    fc_name_ = "log";
                    break;
            }
            if (PyArg_UnpackTuple(args, "log", 1, 1, &text_))
            {
                PyObject *uni_ = PyUnicode_AsUTF8String(text_);
                Py_DECREF(text_);
                if (uni_ == NULL || !PyBytes_Check(uni_))
                {
                    Syslog::HuggleLogs->DebugLog("Python::$" + fc_name_ + "@unkown: parameter text must be of a string type");
                } else
                {
                    Py_DECREF(result_);
                    result_ = PyBool_FromLong(1);
                    QString qs_text_(PyBytes_AsString(uni_));
                    Py_DECREF(uni_);
                    switch (log_type)
                    {
                        case HuggleLogType_Error:
                            Syslog::HuggleLogs->ErrorLog(qs_text_);
                            break;
                        case HuggleLogType_Warn:
                            Syslog::HuggleLogs->WarningLog(qs_text_);
                            break;
                        default:
                            Syslog::HuggleLogs->Log(qs_text_);
                            break;
                    }
                }
            }
            return result_;
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

        static PyObject *Log(PyObject *self, PyObject *args)
        {
            return Log_(HuggleLogType_Normal, self, args);
        }

        static PyObject *WarningLog(PyObject *self, PyObject *args)
        {
            return Log_(HuggleLogType_Warn, self, args);
        }

        static PyObject *ErrorLog(PyObject *self, PyObject *args)
        {
            return Log_(HuggleLogType_Error, self, args);
        }
#if _MSC_VER
#pragma warning ( pop )
#else
#pragma GCC diagnostic pop
#endif

        static PyMethodDef Methods[] = {
            {"huggle_version", ApiVersion, METH_VARARGS, "Return a huggle version"},
            {"warning_log", WarningLog, METH_VARARGS, "Write to warning log"},
            {"log", Log, METH_VARARGS, "Write to a log"},
            {"debug_log", DebugLog, METH_VARARGS, "Write to debug log"},
            {"error_log", ErrorLog, METH_VARARGS, "Write to error log using stderr to output"},
            {NULL, NULL, 0, NULL}
        };

        static PyModuleDef Module = {
            PyModuleDef_HEAD_INIT, "huggle", NULL, -1, Methods, NULL, NULL, NULL, NULL
        };

        static PyObject *PyInit_emb()
        {
            return PyModule_Create(&Module);
        }
    }
}

PythonEngine::PythonEngine(QString ExtensionsFolder_)
{
    // define hooks
    PyImport_AppendInittab("huggle", &PyInit_emb);
    // load it
    Py_Initialize();
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

QList<PythonScript *> PythonEngine::ScriptsList()
{
    return QList<PythonScript*>(this->Scripts);
}

unsigned int PythonEngine::Count()
{
    unsigned int s = 0;
    foreach (PythonScript *x, this->Scripts)
    {
        // we only want to know how many enabled scripts we have
        if (x->IsEnabled())
        {
            s++;
        }
    }
    return s;
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
    this->ModuleID = name;
    if (name.endsWith(".py"))
    {
        this->ModuleID = this->ModuleID.mid(0, this->ModuleID.length() - 3);
    }
    if (this->ModuleID.contains("/"))
    {
        this->ModuleID = this->ModuleID.mid(this->ModuleID.lastIndexOf("/") + 1);
    }
    if (this->ModuleID.contains("\\"))
    {
        this->ModuleID = this->ModuleID.mid(this->ModuleID.lastIndexOf("\\") + 1);
    }
    this->ptr_Hook_MainLoaded = NULL;
    this->Description = "<unknown>";
    this->Author = "<unknown>";
    this->Version = "<unknown>";
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
            if (Configuration::HuggleConfiguration->Verbosity > 0)
                PyErr_Print();
            // we loaded the symbol but it's not callable function
            // so we remove it
            Py_DECREF(ptr_python_);
            Syslog::HuggleLogs->WarningLog("Function " + function + "@" + this->Name
                                           + " isn't callable, hook is disabled now");
            ptr_python_ = NULL;
        } else if (ptr_python_ == NULL)
        {
            Syslog::HuggleLogs->DebugLog("There is no override for " + function);
            if (Configuration::HuggleConfiguration->Verbosity > 0)
                PyErr_Print();
        }
    }
    return ptr_python_;
}

QString PythonScript::CallInternal(QString function)
{
    PyObject *ptr_python_ = NULL;
    if (Huggle::Python::ContainsFunction(function, this->SourceCode))
    {
        Syslog::HuggleLogs->DebugLog("Loading symbols of " + function + " " + this->Name, 2);
        ptr_python_ = PyObject_GetAttrString(this->object, function.toUtf8().data());
        if (ptr_python_ != NULL && !PyCallable_Check(ptr_python_))
        {
            // we loaded the symbol but it's not callable function
            // so we remove it
            Py_DECREF(ptr_python_);
            Syslog::HuggleLogs->WarningLog("Function " + function + "@" + this->Name
                                           + " isn't callable, unable to retrieve");
            if (Configuration::HuggleConfiguration->Verbosity > 0)
                PyErr_Print();
            ptr_python_ = NULL;
        } else if (ptr_python_ == NULL)
        {
            Syslog::HuggleLogs->DebugLog("There is no override for " + function);
            if (Configuration::HuggleConfiguration->Verbosity > 0)
                PyErr_Print();
        }
    }
    if (ptr_python_ == NULL)
    {
        return "<unknown>";
    }
    PyObject *value = PyObject_CallObject(ptr_python_, NULL);
    PyObject *text_ = PyUnicode_AsUTF8String(value);
    Py_DECREF(value);
    if (text_ == NULL || !PyBytes_Check(text_))
    {
        if (Configuration::HuggleConfiguration->Verbosity > 0)
            PyErr_Print();
        Syslog::HuggleLogs->DebugLog("Python::$" + function + "@" + this->Name + ": return value must be of a string type");
        return "<unknown>";
    } else
    {
        QString qs_text_(PyBytes_AsString(text_));
        Py_DECREF(text_);
        return qs_text_;
    }
}

QString PythonScript::GetName() const
{
    return this->Name;
}

QString PythonScript::GetModule() const
{
    return this->ModuleID;
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
    if (this->ptr_Hook_MainLoaded != NULL)
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
        HUGGLE_DEBUG("Importing module " + this->ModuleID, 4);
        PyObject *name = PyUnicode_FromString(this->ModuleID.toUtf8().data());
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
            HUGGLE_DEBUG("Failed to load module " + this->ModuleID, 2);
            PyErr_Print();
            return false;
        }
        Syslog::HuggleLogs->DebugLog("Loading hook symbols for python " + this->Name);
        // load symbols for hooks now
        this->ptr_Hook_MainLoaded = this->Hook("hook_main_window_is_loaded");

        // load the information about the plugin
        this->Author = this->CallInternal("get_author");
        this->Description = this->CallInternal("get_description");
        this->Version = this->CallInternal("get_version");
        return true;
    }
    delete file;
    return false;
}

QString PythonScript::RetrieveSourceCode() const
{
    return this->SourceCode;
}

QString PythonScript::GetVersion() const
{
    return this->Version;
}

QString PythonScript::GetAuthor()
{
    return this->Author;
}

QString PythonScript::GetDescription() const
{
    return this->Description;
}

#endif
