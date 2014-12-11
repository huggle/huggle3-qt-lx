//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "iextension.hpp"
#include "exception.hpp"

using namespace Huggle;

void iExtension::huggle__internal_SetPath(QString path)
{
    if (!this->huggle__internal_ExtensionPath.isEmpty())
        throw new Exception("Extension path is read only", BOOST_CURRENT_FUNCTION);

    this->huggle__internal_ExtensionPath = path;
}

QString iExtension::GetExtensionFullPath()
{
    return this->huggle__internal_ExtensionPath;
}
