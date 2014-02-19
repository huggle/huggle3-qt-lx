//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// This class is only available if you compile huggle with python support

#include "configuration.hpp"

#ifdef PYTHONENGINE

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QList>
#include <Python.h>
#include "configuration.hpp"
#include "exception.hpp"
#include "syslog.hpp"

namespace Huggle
{
    //! DOCUMENT ME
    class PythonScript
    {
        public:
            //! Creates a new instance of python script
            PythonScript(QString name);
            ~PythonScript();
            //! Return a path of a script
            QString GetName() const;
            bool GetEnabled() const;
            void SetEnabled(bool value);
            void Hook_MainWindowIsLoaded();
            //! Initialize the script
            bool Init();
            QString RetrieveSourceCode() const;
        private:
            //! Pointer to a python object that represent this script
            PyObject *object;
            QString Name;
            bool Enabled;
            PyObject *ptr_Hook_MainLoaded;
            QString SourceCode;
    };

    //! This python engine should allow users to create python modules for huggle

    //! This interface will create a python interpretor that can load any .py
    //! scripts using the LoadScript(path) function. Modules should only contain
    //! functions and hooks. If there is any execution body it will be executed
    //! immediately after the script is loaded.
    class PythonEngine
    {
        public:
            PythonEngine(QString ExtensionsFolder_);
            bool LoadScript(QString path);
            void Hook_MainWindowIsLoaded();
        private:
            static PyObject* api_Version(PyObject *self, PyObject *args);
            PyMethodDef Methods[1];
            QList<PythonScript*> Scripts;
    };
}

#endif // PYTHONENGINE_H
#endif
