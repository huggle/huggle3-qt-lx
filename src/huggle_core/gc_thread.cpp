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

#include "gc_thread.hpp"
#include "gc.hpp"

using namespace Huggle;

GC_t::GC_t(QObject *parent) : QThread(parent)
{
    this->Stopped = false;
    this->Running = true;
}

GC_t::~GC_t()
{

}

void GC_t::Stop()
{
    this->Running = false;
}

bool GC_t::IsStopped() const
{
    return this->Stopped;
}

bool GC_t::IsRunning() const
{
    return this->Running;
}

void GC_t::run()
{
    while(this->Running)
    {
        if (GC::gc != nullptr)
        {
            GC::gc->DeleteOld();
        }
        this->msleep(800);
    }
    this->Stopped = true;
}
