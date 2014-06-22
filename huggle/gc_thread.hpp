// IMPORTANT: this file has a different license than rest of huggle

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2014

#ifndef GC_THREAD_HPP
#define GC_THREAD_HPP

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif
#include <QThread>

namespace Huggle
{
    class GC_t : public QThread
    {
            Q_OBJECT
        public:
            GC_t(QObject *parent = nullptr);
            ~GC_t();
            void Stop();
            bool IsStopped() const;
            bool IsRunning() const;
        protected:
            void run();
        private:
            bool Running;
            bool Stopped;
    };
}

#endif // GC_THREAD_HPP
