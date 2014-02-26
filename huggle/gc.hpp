//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef GC_H
#define GC_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QList>
#include <QMutex>
#include "collectable.hpp"

#define HUGGLECONSUMER_WIKIEDIT                 0
#define HUGGLECONSUMER_PROVIDERIRC              1
#define HUGGLECONSUMER_QUEUE                    2
#define HUGGLECONSUMER_CORE_POSTPROCESS         3
#define HUGGLECONSUMER_DELETEFORM               4
#define HUGGLECONSUMER_PROCESSLIST              5
#define HUGGLECONSUMER_HUGGLETOOL               6
#define HUGGLECONSUMER_EDITQUERY                7
#define HUGGLECONSUMER_REVERTQUERY              8
#define HUGGLECONSUMER_MAINFORM                 9
#define HUGGLECONSUMER_LOGINFORM                10
#define HUGGLECONSUMER_MESSAGE_SEND             11
#define HUGGLECONSUMER_PROTECTPAGE              12
#define HUGGLECONSUMER_HISTORYWIDGET            13
#define HUGGLECONSUMER_USERINFO                 14
#define HUGGLECONSUMER_DELETIONLOCK             15
#define HUGGLECONSUMER_BLOCKFORM                16
//! This is used to lock the message resource before it's passed to parent object
#define HUGGLECONSUMER_CORE_MESSAGE             17
#define HUGGLECONSUMER_REPORTFORM               22
#define HUGGLECONSUMER_CORE                     800

namespace Huggle
{
    class Collectable;

    //! Garbage collector that can be used to collect some objects

    //! Every object must be derived from Collectable, otherwise it
    //! must not be handled by garbage collector
    class GC
    {
        public:
            //! Garbage collector
            static GC *gc;

            GC();
            ~GC();
            //! Function that walks through the list and delete these that can be deleted
            void DeleteOld();
            //! List of all managed queries that qgc keeps track of
            QList<Collectable*> list;
            //! QMutex that is used to lock the GC::list object

            //! This lock needs to be aquired every time when you need to access this list
            //! from any thread during runtime
            QMutex * Lock;
    };
}

#endif // GC_H
