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

#include <Python.h>

namespace Huggle
{
    //! This python engine should allow users to create python modules for huggle
    class PythonEngine
    {
    public:
        PythonEngine();
    };
}

#endif // PYTHONENGINE_H

#endif
