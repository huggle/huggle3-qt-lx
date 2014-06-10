//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggleoption.hpp"

using namespace Huggle;

HuggleOption::HuggleOption(QString name, QVariant value, bool isdefault)
{
    this->Name = name;
    this->isDefault = isdefault;
    this->Value = value;
}

void HuggleOption::SetVariant(QVariant value)
{
    this->isDefault = false;
    this->Value = value;
}


