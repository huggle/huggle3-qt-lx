//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <QObject>
#include <QString>

namespace Huggle
{
    class WikiUser;
    class Hooks;
    class Events : public QObject
    {
            Q_OBJECT
        public:
            static Events *Global;
            Events();
            virtual ~Events();

        signals:
            void WikiUser_Updated(WikiUser *wiki_user);

        private:
            friend class Hooks;

            void on_UpdateUser(WikiUser *wiki_user);
    };
}

#endif // EVENTS_HPP
