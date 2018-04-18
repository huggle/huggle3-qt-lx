//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "version.hpp"
#include <QStringList>
#include "exception.hpp"

using namespace Huggle;
Version Version::SupportedMediawiki = Version(HUGGLE_SUPPORTED_MEDIAWIKI_VERSION);

bool Version::IsValid(QString version)
{
    if (!version.contains("."))
        return false;
    // there will be more checks in future

    return true;
}

static int IndexOfNonIntChar(QString string)
{
    int position = 0;
    char min = '0';
    char max = '9';
    while (position < string.length())
    {
        if (string[position] < min || string[position] > max)
            return position;
        position++;
    }
    return -1;
}

Version::Version()
{
    this->isValid = false;
}

Version::Version(QString version)
{
    if (!IsValid(version))
        return;

    this->original_string = version;
    QStringList subversions = this->original_string.split(".");
    bool ok = true;
    this->major = subversions[0].toInt(&ok);
    if (!ok)
        return;
    this->minor = this->getSuffixed(subversions[1]).toInt(&ok);
    if (!ok)
        return;
    if (subversions.count() > 2)
    {
        this->revision = this->getSuffixed(subversions[2]).toInt(&ok);
        if (!ok)
            return;
    }
    if (subversions.count() > 3)
    {
        this->patch = this->getSuffixed(subversions[3]).toInt(&ok);
        if (!ok)
            return;
    }

    this->isValid = true;
}

Version::~Version()
{

}

bool Version::IsEqual(const Version *b, bool ignore_suffix) const
{
    if (!this->isValid || !b->isValid)
        return false;
    if (this->major != b->major ||
        this->minor != b->minor ||
        this->patch != b->patch ||
        this->revision != b->revision ||
        this->versionType != b->versionType)
        return false;
    if (!ignore_suffix && this->suffix != b->suffix)
        return false;
    else
        return true;
}

bool Version::IsLower(const Version *b) const
{
    if (!this->IsValid() || !b->IsValid())
        return false;
    if (this->GetMajor() > b->GetMajor())
        return false;
    else if (this->GetMajor() < b->GetMajor())
        return true;
    if (this->GetMinor() > b->GetMinor())
        return false;
    else if (this->GetMinor() < b->GetMinor())
        return true;
    if (this->GetRevision() > b->GetRevision())
        return false;
    else if (this->GetRevision() < b->GetRevision())
        return true;

    return !this->IsEqual(b, true);
}

bool Version::IsGreater(const Version *b) const
{
    if (!this->IsValid() || !b->IsValid())
        return false;
    if (this->GetMajor() > b->GetMajor())
        return true;
    else if (this->GetMajor() < b->GetMajor())
        return false;
    if (this->GetMinor() > b->GetMinor())
        return true;
    else if (this->GetMinor() < b->GetMinor())
        return false;
    if (this->GetRevision() > b->GetRevision())
        return true;
    else if (this->GetRevision() < b->GetRevision())
        return false;

    return false;
}

QString Version::getSuffixed(QString number)
{
    int index = IndexOfNonIntChar(number);
    if (index != -1)
    {
        this->suffix = number.mid(index);
        number = number.mid(0, index);
    }
    return number;
}

bool Huggle::operator!=(const Version &a, const Version &b)
{
    return !a.IsEqual(&b);
}

bool Huggle::operator==(const Version &a, const Version &b)
{
    return a.IsEqual(&b);
}

bool Huggle::operator >=(const Version &a, const Version &b)
{
    return a.IsEqual(&b) || a.IsGreater(&b);
}

bool Huggle::operator >(const Version &a, const Version &b)
{
    return a.IsGreater(&b);
}

bool Huggle::operator <(const Version &a, const Version &b)
{
    return a.IsLower(&b);
}

bool Huggle::operator <=(const Version &a, const Version &b)
{
    return a.IsEqual(&b) || a.IsLower(&b);
}
