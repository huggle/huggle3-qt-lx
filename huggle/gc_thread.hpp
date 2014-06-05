//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef GC_THREAD_HPP
#define GC_THREAD_HPP

#include <QThread>

namespace Huggle
{
    class GC_t : public QThread
    {
            Q_OBJECT
        public:
            GC_t(QObject *parent = nullptr);
            ~GC_t();
            void Stop();
            bool IsStopped() const;
            bool IsRunning() const;
        protected:
            void run();
        private:
            bool Running;
            bool Stopped;
    };
}

#endif // GC_THREAD_HPP
