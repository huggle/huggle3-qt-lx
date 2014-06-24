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
#include <cstdint>
#include "core.hpp"
#include "configuration.hpp"
#include "wikiutil.hpp"
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
        static void TryCatch(PyObject *script)
        {
            if (script)
            {
                PythonScript *ptr = Core::HuggleCore->Python->PythonScriptObjFromPyObj(script);
                if (ptr != nullptr)
                {
                    HUGGLE_DEBUG1("Exception in " + ptr->GetName());
                }
            }
            if (PyErr_Occurred())
            {
                HUGGLE_DEBUG("Printing the exception to cout::stderr", 5);
                PyErr_PrintEx(1);
            }
        }

        static PyObject *ApiVersion(PyObject *self, PyObject *args)
        {
            PyObject *v = PyUnicode_FromString(Configuration::HuggleConfiguration->HuggleVersion.toUtf8().data());
            return v;
        }

        QString StringFromPyString(PyObject *text)
        {
            if (!text)
                return "";
            PyObject *tn = PyUnicode_AsASCIIString(text);
            if (!tn)
            {
                TryCatch(nullptr);
                return "";
            }
            QString result = QString(PyBytes_AsString(tn));
            Py_DECREF(tn);
            return result;
        }

        static PyObject *DebugLog(PyObject *self, PyObject *args)
        {
            PyObject *py_verbosity_ = nullptr;
            PyObject *text_ = nullptr;
            PyObject *result_ = PyBool_FromLong(0);
            if (PyArg_UnpackTuple(args, "debug_log", 1, 2, &text_, &py_verbosity_))
            {
                Py_DECREF(result_);
                result_ = PyBool_FromLong(1);
                unsigned int verbosity = 1;
                if (py_verbosity_ != nullptr && PyLong_Check(py_verbosity_))
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
                if (uni_ == nullptr || !PyBytes_Check(uni_))
                {
                    Syslog::HuggleLogs->DebugLog("Log@unkown: parameter text must be of a string type");
                } else
                {
                    QString qs_text_(PyBytes_AsString(uni_));
                    Syslog::HuggleLogs->DebugLog(qs_text_, verbosity);
                    Py_DECREF(uni_);
                }
            } else
            {
                HUGGLE_DEBUG("Failed to unpack tuple @DebugLog", 2);
                TryCatch(nullptr);
            }
            return result_;
        }

        static PyObject *Log_(HuggleLogType log_type, PyObject *self, PyObject *args)
        {
            PyObject *text_ = nullptr;
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
                if (uni_ == nullptr || !PyBytes_Check(uni_))
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
            } else
            {
                HUGGLE_DEBUG1("Failed to unpack tuple in Log_ fc");
                TryCatch(nullptr);
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

        static PyObject *Configuration_GetUser(PyObject *self, PyObject *args)
        {
            PyObject *result_ = PyUnicode_FromString(Configuration::HuggleConfiguration->SystemConfig_Username.toUtf8().data());
            return result_;
        }

        static PyObject *Configuration_GetWikiURL(PyObject *self, PyObject *args)
        {
            PyObject *result_ = PyUnicode_FromString(Configuration::HuggleConfiguration->GetProjectWikiURL().toUtf8().data());
            return result_;
        }

        static PyObject *Configuration_GetScript(PyObject *self, PyObject *args)
        {
            PyObject *result_ = PyUnicode_FromString(Configuration::HuggleConfiguration->GetProjectScriptURL().toUtf8().data());
            return result_;
        }

        static PyObject *Config_Set(PyObject *self, PyObject *args)
        {
            /*PythonScript *script = Core::HuggleCore->Python->PythonScriptObjFromPyObj(self);
            if (script == nullptr)
                return nullptr;
                */
            PyObject *name, *vals, *extension;
            if (!PyArg_UnpackTuple(args, "Config_Set", 1, 3, &extension, &name, &vals))
            {
                //HUGGLE_DEBUG1("Failed to run Config_Set - arguments could not be processed @" + script->GetName());
                TryCatch(self);
                return nullptr;
            }
            QString option = StringFromPyString(name);
            QString ex = StringFromPyString(extension);
            QString data = StringFromPyString(vals);
            // remove the temp objects we created
            Py_DECREF(name);
            Py_DECREF(vals);
            if (!Configuration::HuggleConfiguration->ExtensionData.contains(ex))
            {
                // it seems this extension didn't store any data so far
                // let's create a new storage
                Configuration::HuggleConfiguration->ExtensionData.insert(ex, new ExtensionConfig());
            }
            Configuration::HuggleConfiguration->ExtensionData[ex]->SetOption(option, data);
            PyObject *result_ = PyBool_FromLong(1);
            return result_;
        }

        static PyObject *Config_Get(PyObject *self, PyObject *args)
        {
            //PythonScript *script = Core::HuggleCore->Python->PythonScriptObjFromPyObj(self);
            //if (script == nullptr)
            //    return nullptr;
            PyObject *name, *extension;
            if (!PyArg_UnpackTuple(args, "Config_Set", 1, 2, &extension, &name))
            {
                //HUGGLE_DEBUG1("Failed to run Config_Get - arguments could not be processed @" + script->GetName());
                TryCatch(self);
                return nullptr;
            }
            QString option = StringFromPyString(name);
            QString module = StringFromPyString(extension);
            // remove the temp objects we created
            Py_DECREF(name);
            Py_DECREF(extension);
            PyObject *result_ = PyUnicode_FromString(Configuration::HuggleConfiguration->GetExtensionConfig(module, option, "").toUtf8().data());
            return result_;
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

        static PyObject *Wikipage_append(PyObject *self, PyObject *args)
        {
            HUGGLE_DEBUG("Running Wikipage_append", 4);
            PyObject *result;
            PyObject *page, *text, *summary;
            if (!PyArg_UnpackTuple(args, "Wikipage_Append", 1, 3, &page, &text, &summary))
            {
                HUGGLE_DEBUG1("Failed to run Wikipage_append - arguments could not be processed");
                return nullptr;
            }
            PyObject *tpage = PyUnicode_AsASCIIString(page);
            PyObject *tsuma = PyUnicode_AsASCIIString(summary);
            PyObject *ttext = PyUnicode_AsASCIIString(text);
            if (!tpage || !tsuma || !ttext)
            {
                TryCatch(self);
                return nullptr;
            }
            QString page_name = QString(PyBytes_AsString(tpage));
            QString page_text = QString(PyBytes_AsString(ttext));
            QString page_su = QString(PyBytes_AsString(tsuma));
            // remove the temp objects we created
            Py_DECREF(ttext);
            Py_DECREF(tsuma);
            Py_DECREF(tpage);
            // now we need to append the text to page
            WikiUtil::AppendTextToPage(page_name, page_text, page_su);
            Py_DECREF(page);
            Py_DECREF(summary);
            Py_DECREF(text);

            result = PyBool_FromLong(1);
            return result;
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
            {"wikipage_append", Wikipage_append, METH_VARARGS, "Append a text to a page"},
            {"configuration_get_user", Configuration_GetUser, METH_VARARGS, "Request a name of user who is currently used on selected wiki"},
            {"configuration_get_project_script_url", Configuration_GetScript, METH_VARARGS, "Return a script url"},
            {"configuration_get_project_wiki_url", Configuration_GetWikiURL, METH_VARARGS, "Return an URL of current project"},
            {"configuration_get", Config_Get, METH_VARARGS, "Get a private configuration option"},
            {"configuration_set", Config_Set, METH_VARARGS, "Set a private configuration option"},
            {nullptr, nullptr, 0, nullptr}
        };

        static PyModuleDef Module = {
            PyModuleDef_HEAD_INIT, "huggle", nullptr, -1, Methods, nullptr, nullptr, nullptr, nullptr
        };

        static PyObject *PyInit_emb()
        {
            return PyModule_Create(&Module);
        }
    }
}

QString DoubleBack(QString path)
{
    QString result = QString(path);
    return result.replace("\\", "\\\\");
}

PythonEngine::PythonEngine(QString ExtensionsFolder_)
{
    // define hooks
    PyImport_AppendInittab("huggle", &PyInit_emb);
    // load it
    Py_Initialize();
    Syslog::HuggleLogs->DebugLog("Inserting extensions folder to path: " + DoubleBack(ExtensionsFolder_));
    if (PyRun_SimpleString(QString("import sys; sys.path.append('" + DoubleBack(ExtensionsFolder_) + "')").toUtf8().data()))
        TryCatch(nullptr);
#ifdef HUGGLE_GLOBAL_EXTENSION_PATH
    PyRun_SimpleString(QString(QString("sys.path.append('") + HUGGLE_GLOBAL_EXTENSION_PATH + "')").toUtf8().data());
#endif
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

PythonScript *PythonEngine::PythonScriptObjFromPyObj(PyObject *object)
{
    foreach (PythonScript *script, this->Scripts)
    {
        if (object == script->PythonObject())
        {
            return script;
        }
    }
    HUGGLE_DEBUG("Unable to resolve script from script table, id: 0x" + QString::number((intptr_t)object, 16), 4);
    return nullptr;
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

void PythonEngine::Hook_HuggleShutdown()
{
    foreach (PythonScript *c, this->Scripts)
    {
        if (c->IsEnabled())
        {
            c->Hook_Shutdown();
        }
    }
}

void PythonEngine::Hook_MainWindowIsLoaded()
{
    foreach (PythonScript *c, this->Scripts)
    {
        if (c->IsEnabled())
        {
            c->Hook_MainWindowIsLoaded();
        }
    }
}

void PythonEngine::Hook_SpeedyFinished(WikiEdit *edit, QString tags, bool successfull)
{
    foreach (PythonScript *c, this->Scripts)
    {
        if (c->IsEnabled())
        {
            c->Hook_SpeedyFinished(edit, tags, successfull);
        }
    }
}

PythonScript::PythonScript(QString name)
{
    this->SourceCode = "";
    this->Name = name;
    this->object = nullptr;
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
    this->ptr_Hook_MainLoaded = nullptr;
    this->Description = "<unknown>";
    this->Author = "<unknown>";
    this->Version = "<unknown>";
    this->Enabled = false;
}

PythonScript::~PythonScript()
{
    if (this->ptr_Hook_MainLoaded != nullptr)
    {
        Py_DECREF(this->ptr_Hook_MainLoaded);
    }
    if (this->object != nullptr)
    {
        Py_DECREF(this->object);
    }
}

PyObject *PythonScript::Hook(QString function)
{
    PyObject *ptr_python_ = nullptr;
    if (Huggle::Python::ContainsFunction(function, this->SourceCode))
    {
        Syslog::HuggleLogs->DebugLog("Loading hook symbols of " + function + " " + this->Name, 2);
        ptr_python_ = PyObject_GetAttrString(this->object, function.toUtf8().data());
        if (ptr_python_ != nullptr && !PyCallable_Check(ptr_python_))
        {
            if (Configuration::HuggleConfiguration->Verbosity > 0)
                TryCatch(nullptr);
            // we loaded the symbol but it's not callable function
            // so we remove it
            Py_DECREF(ptr_python_);
            Syslog::HuggleLogs->WarningLog("Function " + function + "@" + this->Name
                                           + " isn't callable, hook is disabled now");
            ptr_python_ = nullptr;
        } else if (ptr_python_ == nullptr)
        {
            Syslog::HuggleLogs->DebugLog("There is no override for " + function);
            if (Configuration::HuggleConfiguration->Verbosity > 0)
                TryCatch(nullptr);
        }
    }
    return ptr_python_;
}

QString PythonScript::CallInternal(QString function)
{
    PyObject *ptr_python_ = nullptr;
    if (Huggle::Python::ContainsFunction(function, this->SourceCode))
    {
        Syslog::HuggleLogs->DebugLog("Loading symbols of " + function + " " + this->Name, 2);
        ptr_python_ = PyObject_GetAttrString(this->object, function.toUtf8().data());
        if (ptr_python_ != nullptr && !PyCallable_Check(ptr_python_))
        {
            // we loaded the symbol but it's not callable function
            // so we remove it
            Py_DECREF(ptr_python_);
            Syslog::HuggleLogs->WarningLog("Function " + function + "@" + this->Name
                                           + " isn't callable, unable to retrieve");
            TryCatch(nullptr);
            ptr_python_ = nullptr;
        } else if (ptr_python_ == nullptr)
        {
            Syslog::HuggleLogs->DebugLog("There is no override for " + function);
            TryCatch(nullptr);
        }
    }
    if (ptr_python_ == nullptr)
    {
        return "<unknown>";
    }
    PyObject *value = PyObject_CallObject(ptr_python_, nullptr);
    PyObject *text_ = PyUnicode_AsUTF8String(value);
    Py_DECREF(value);
    if (text_ == nullptr || !PyBytes_Check(text_))
    {
        TryCatch(nullptr);
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

/////////////////////////////////////////////////
// HOOKS
/////////////////////////////////////////////////

void PythonScript::Hook_Shutdown()
{
    if (this->ptr_Hook_Shutdown != nullptr)
    {
        HUGGLE_DEBUG("Calling hook Hook_Shutdown @" + this->Name, 2);
        if (!PyObject_CallObject(this->ptr_Hook_Shutdown, nullptr))
            TryCatch(nullptr);
    }
}

void PythonScript::Hook_SpeedyFinished(WikiEdit *edit, QString tags, bool successfull)
{
    if (edit == nullptr)
        return;
    if (this->ptr_Hook_SpeedyFinished != nullptr)
    {
        HUGGLE_DEBUG("Calling hook Hook_SpeedyFinished @" + this->Name, 2);
        // let's make a new list of params
        PyObject *page_name = PyUnicode_FromString(edit->Page->PageName.toUtf8().data());
        if (!page_name)
            goto error;
        PyObject *page_t_ = PyUnicode_FromString(tags.toUtf8().data());
        if (!page_t_)
            goto error;
        PyObject *user_name = PyUnicode_FromString(edit->User->Username.toUtf8().data());
        if (!user_name)
            goto error;
        PyObject *success;
        if (!successfull)
            success = PyUnicode_FromString("fail");
        else
            success = PyUnicode_FromString("success");
        if (!success)
            goto error;
        PyObject *args = PyTuple_Pack(4, page_name, user_name, page_t_, success);
        if (!args)
            goto error;
        if (!PyObject_CallObject(this->ptr_Hook_SpeedyFinished, args))
            goto error;
    }
    return;

    error:
        HUGGLE_DEBUG("Error in: " + this->Name, 2);
        TryCatch(nullptr);

}

void PythonScript::Hook_MainWindowIsLoaded()
{
    if (this->ptr_Hook_MainLoaded != nullptr)
    {
        Syslog::HuggleLogs->DebugLog("Calling hook Hook_MainWindowIsLoaded @" + this->Name, 2);
        if(!PyObject_CallObject(this->ptr_Hook_MainLoaded, nullptr))
            TryCatch(nullptr);
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
        if (name == nullptr)
        {
            TryCatch(nullptr);
            return false;
        }
        this->object = PyImport_Import(name);
        // remove name
        Py_DECREF(name);
        if (this->object == nullptr)
        {
            HUGGLE_DEBUG("Failed to load module " + this->ModuleID, 2);
            TryCatch(nullptr);
            return false;
        }
        Syslog::HuggleLogs->DebugLog("Loading hook symbols for python " + this->Name);
        // load symbols for hooks now
        this->ptr_Hook_Shutdown = this->Hook("hook_shutdown");
        this->ptr_Hook_SpeedyFinished = this->Hook("hook_speedy_finished");
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
