//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

// This class is only available if you compile huggle with python support
// see definitions.hpp in order to enable it

#include "definitions.hpp"

#ifdef HUGGLE_PYTHON

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#define HUGGLE_EINVALIDQUERY 1

#include <QString>
#include <QThread>
#include <QHash>
#include <QMutex>
#include <QList>

namespace Huggle
{
    class Query;
    class WikiEdit;

    //! Python code goes here, this namespace doesn't exist when huggle isn't compiled in python mode so wrap in direct.
    namespace Python
    {
        //! Every python script that is used in huggle is described by instance of this thing
        class PythonScript
        {
            public:
                //! Creates a new instance of python script
                PythonScript(QString name);
                ~PythonScript();
                //! Return a path of a script
                QString GetName() const;
                QString GetModule() const;
                bool IsEnabled() const;
                void SetEnabled(bool value);
                void Hook_SpeedyFinished(WikiEdit *edit, QString tags, bool successfull);
                void Hook_MainWindowIsLoaded();
                void Hook_Shutdown(); 
                void Hook_OnEditPreProcess(WikiEdit *edit);
                void Hook_OnEditPostProcess(WikiEdit *edit);
                bool Hook_OnEditLoadToQueue(WikiEdit *edit);
                void Hook_GoodEdit(WikiEdit *edit);
                PyObject *PythonObject();
                //! Initialize the script
                bool Init();
                QString RetrieveSourceCode() const;
                QString GetVersion() const;
                QString GetAuthor();
                QString GetDescription() const;
            private:
                PyObject *Hook(QString function);
                //! Pointer to a python object that represent this script
                PyObject *object;
                QString Name;
                QString Version;
                QString Author;
                QString Description;
                QString ModuleID;
                QString CallInternal(QString function);
                bool Enabled;
                PyObject *ptr_Hook_SpeedyFinished = nullptr;
                PyObject *ptr_Hook_MainLoaded = nullptr;
                PyObject *ptr_Hook_OnEditPreProcess = nullptr;
                PyObject *ptr_Hook_Shutdown = nullptr;
                PyObject *ptr_Hook_OnEditPostProcess = nullptr;
                PyObject *ptr_Hook_OnEditLoadToQueue = nullptr;
                PyObject *ptr_Hook_GoodEdit = nullptr;
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
                ~PythonEngine();
                unsigned int Count();
                bool LoadScript(QString path);
                void Hook_SpeedyFinished(WikiEdit *edit, QString tags, bool successfull);
                void Hook_MainWindowIsLoaded();
                void Hook_HuggleShutdown();
                void Hook_OnEditPreProcess(WikiEdit *edit);
                void Hook_OnEditPostProcess(WikiEdit *edit);
                //! Called before the edit is inserted into queue of huggle, if returns false, the edit is not inserted into queue
                bool Hook_OnEditLoadToQueue(WikiEdit *edit);
                void Hook_GoodEdit(WikiEdit *edit);
                QString HugglePyLibSource();
                PythonScript *PythonScriptObjFromPyObj(PyObject *object);
                QList<PythonScript*> ScriptsList();
                Query *GetQuery(unsigned long ID);
                /*!
                 * \brief InsertQuery put a query to list of all queries that are referenced by python objects
                 * \param query
                 * \return reference ID of a query managed by python
                 */
                unsigned long InsertQuery(Query *query);
                unsigned long RemoveQuery(unsigned long ID);
            private:
                QHash<unsigned long, Query*> Queries;
                unsigned long LastQuery = 0;
                QString hugglePyLib;
                friend class PythonScript;
                QList<PythonScript*> Scripts;
        };

        inline PyObject * PythonScript::PythonObject()
        {
            return this->object;
        }
    }
}

#endif // PYTHONENGINE_H
#endif
