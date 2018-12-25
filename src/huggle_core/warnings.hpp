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

#include "definitions.hpp"

#include <QString>
#include <QList>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "wikiedit.hpp"

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
    class HUGGLE_EX_CORE PendingWarning
    {
        public:
            static QList<PendingWarning*> PendingWarnings;

            PendingWarning(Message *message, const QString &warning, WikiEdit *edit);
            ~PendingWarning();
            //! The message of this warning
            Message *Warning;
            //! The edit because of which this warning was sent
            Collectable_SmartPtr<WikiEdit> RelatedEdit;
            //! Template used in this warning so that we can use the same template for new attempt if any is needed
            QString Template;
            Collectable_SmartPtr<ApiQuery> Query;
    };

    //! This NS contains functions that generate warnings to users
    namespace Warnings
    {
        /*!
         * \brief WarnUser Sends a warning to user
         * \param warning_type Type of warning
         * \param dependency If the warnings is depending on some revert you can put a pointer to that here
         * \param edit Pointer to edit this warning is related to
         * \param report This is address of boolean to which value whether user should be reported instead is stored
         * \return Pointer to a warning object which contains information about the warning
         */
        HUGGLE_EX_CORE PendingWarning *WarnUser(const QString& warning_type, RevertQuery *dependency, WikiEdit *edit, bool *report);
        //! This sends a warning to user no matter if they should receive it or not
        HUGGLE_EX_CORE void ForceWarn(int level, WikiEdit *edit);
        //! Checks all warnings that weren't sent and try to send them

        //! This is used on talk pages of users which changed while we tried to send them a warning
		HUGGLE_EX_CORE void ResendWarnings();
        HUGGLE_EX_CORE QString RetrieveTemplateToWarn(const QString& type, WikiSite *site, bool force = false);
        //! In case there is no shared IP
        HUGGLE_EX_CORE QString UpdateSharedIPTemplate(WikiUser *user, QString text, WikiSite *site);
    }
}

#endif // WARNINGS_HPP
