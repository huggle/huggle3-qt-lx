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
#include "syslog.hpp"

namespace Huggle
{
    //! DOCUMENT ME
    class PythonScript
    {
        public:
            PythonScript(QString name);
            QString GetName() const;
            bool GetEnabled() const;
            void SetEnabled(bool value);
            bool Init();
            QString RetrieveText() const;
        private:
            //! Pointer to a python object that represent this script
            PyObject *object;
            QString Name;
            bool Enabled;
            QString Text;
    };

    //! This python engine should allow users to create python modules for huggle
    class PythonEngine
    {
        public:
            PythonEngine(QString ExtensionsFolder_);
            bool LoadScript(QString path);
        private:
            QList<PythonScript*> Scripts;
    };
}

#endif // PYTHONENGINE_H
#endif
