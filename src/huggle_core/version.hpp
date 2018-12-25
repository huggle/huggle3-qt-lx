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
    class HUGGLE_EX_CORE Version
    {
        public:
            /*!
             * \brief Returns whether or not a string is a valid version number
             * \param String to check
             */
            static bool IsValid(const QString& version);
            /*!
             * \brief The version of mediawiki supported by huggle
             */
            static Version SupportedMediawiki;
            Version();
            Version(const QString& version);
            ~Version()=default;
            /*!
             * \brief Returns the major section of the version number
             */
            int GetMajor() const;
            /*!
             * \brief Returns the minor section of the version number
             */
            int GetMinor() const;
            /*!
             * \brief Returns the revision section of the version number
             */
            int GetRevision() const;
            /*!
             * \brief Returns whether or not 2 versions are equal
             * \param The version to check against
             * \param If the suffix of the version should be ignored
             */
            bool IsEqual(const Version *b, bool ignore_suffix = false) const;
            /*!
            * \brief Returns whether or not  the current version is lower than another version
            * \param The version to check against
            */
            bool IsLower(const Version *b) const;
            /*!
             * \brief Returns whether or not  the current version is higher than another version
             * \param The version to check against
             */
            bool IsGreater(const Version *b) const;
            //! Returns if the instance's version is valid
            bool IsValid() const;
            //! Version as string
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

    inline int Version::GetMajor() const
    {
        return this->major;
    }

    inline int Version::GetMinor() const
    {
        return this->minor;
    }

    inline int Version::GetRevision() const
    {
        return this->revision;
    }

    inline bool Version::IsValid() const
    {
        return this->isValid;
    }

    inline QString Version::ToString()
    {
        return this->original_string;
    }

	HUGGLE_EX_CORE bool operator !=(const Version &a, const Version &b);
	HUGGLE_EX_CORE bool operator ==(const Version &a, const Version &b);
	HUGGLE_EX_CORE bool operator <=(const Version &a, const Version &b);
	HUGGLE_EX_CORE bool operator >=(const Version &a, const Version &b);
	HUGGLE_EX_CORE bool operator >(const Version &a, const Version &b);
	HUGGLE_EX_CORE bool operator <(const Version &a, const Version &b);
}

#endif // VERSION_HPP
