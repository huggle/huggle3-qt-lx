//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WARNINGS_HPP
#define WARNINGS_HPP

#include "config.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include "revertquery.hpp"
#include "wikiedit.hpp"
#include "core.hpp"
#include "message.hpp"
#include "wikiuser.hpp"
#include "reportuser.hpp"
#include "apiquery.hpp"
#include "configuration.hpp"

namespace Huggle
{
    class WikiEdit;
    class Message;
    class WikiUser;
    class RevertQuery;
    class ApiQuery;

    /*!
     * \brief The PendingWarning class represent the warning that was requested but might not be delievered
     *
     * We can be using this to track all warnings that are still waiting for delivery and these which have failed
     * because the user talk page was changed meanwhile. In this case we need to parse it again and re-issue
     * the warning based on latest version of talk page. The current process is, that every warning that is sent
     * is stored in a list of these pending warnings and periodically checked using timer. If it's finished it's
     * removed, if it's not then it is checked and something is done with it.
     */
    class PendingWarning
    {
        public:
            //! Unique garbage collector id used to lock the edit related to this warning
            static int GCID;

            PendingWarning(Message *message, QString warning, WikiEdit *edit);
            ~PendingWarning();
            //! The message of this warning
            Message *Warning;
            //! The edit because of which this warning was sent
            WikiEdit *RelatedEdit;
            //! Template used in this warning so that we can use the same template for new attempt if any is needed
            QString Template;
            int gcid;
            ApiQuery *Query;
    };

    //! This NS contains functions that generate warnings to users
    namespace Warnings
    {
        /*!
         * \brief WarnUser Sends a warning to user
         * \param WarningType Type of warning
         * \param Dependency If the warnings is depending on some revert you can put a pointer to that here
         * \param Edit Pointer to edit this warning is related to
         * \param Report This is address of boolean to which value whether user should be reported intead is stored
         * \return Pointer to a warning object which contains information about the warning
         */
        PendingWarning *WarnUser(QString WarningType, RevertQuery *Dependency, WikiEdit *Edit, bool *Report);
        //! This sends a warning to user no matter if they should receive it or not
        void ForceWarn(int Level, WikiEdit *Edit);
        //! In case there is no shared IP
        QString UpdateSharedIPTemplate(WikiUser *User, QString Text);
    }
}

#endif // WARNINGS_HPP
