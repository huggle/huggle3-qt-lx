//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef VERSION_HPP
#define VERSION_HPP

#include "definitions.hpp"
#include <QString>

namespace Huggle
{
    enum VersionType
    {
        VersionType_Alpha,
        VersionType_Beta,
        VersionType_Unknown
    };

    //! This class can be used to store various versions and compare them
    class HUGGLE_EX Version
    {
        public:
            static bool IsValid(QString version);
            static Version SupportedMediawiki;

            Version();
            Version(QString version);
            ~Version();
            int GetMajor();
            int GetMinor();
            int GetRevision();
            bool IsEqual(Version *b, bool ignore_suffix = false);
            bool IsLower(Version *b);
            bool IsGreater(Version *b);
            bool IsValid();
            QString ToString();
        private:
            QString getSuffixed(QString number);
            int major = 0;
            int minor = 0;
            int revision = 0;
            int patch = 0;
            bool isValid = false;
            QString original_string;
            VersionType versionType = VersionType_Unknown;
            QString suffix;
    };

    inline int Version::GetMajor()
    {
        return this->major;
    }

    inline int Version::GetMinor()
    {
        return this->minor;
    }

    inline int Version::GetRevision()
    {
        return this->revision;
    }

    inline bool Version::IsValid()
    {
        return this->isValid;
    }

    inline QString Version::ToString()
    {
        return this->original_string;
    }

    bool operator !=(Version &a, Version &b);
    bool operator ==(Version &a, Version &b);
    bool operator <=(Version &a, Version &b);
    bool operator >=(Version &a, Version &b);
    bool operator >(Version &a, Version &b);
    bool operator <(Version &a, Version &b);
}

#endif // VERSION_HPP
