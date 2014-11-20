//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEPROFILER_HPP
#define HUGGLEPROFILER_HPP

#include "definitions.hpp"
#ifdef HUGGLE_PROFILING
#include <QDateTime>
#include <QString>

#define HUGGLE_PROFILER_RESET Huggle::Profiler::Reset()
#define HUGGLE_PROFILER_TIME  Huggle::Profiler::GetTime()
#define HUGGLE_PROFILER_PRINT_TIME(function) Huggle::Syslog::HuggleLogs->DebugLog(QString("PROFILER: ") \
                                             + function + " finished in " + QString::number(Huggle::Profiler::GetTime()) \
                                             + "ms");\
                                             Huggle::Profiler::Reset()

namespace Huggle
{
    class HUGGLE_EX Profiler
    {
        public:
            static void Reset();
            static qint64 GetTime();
        private:
            static QDateTime ts;
    };
}
#else

#define HUGGLE_PROFILER_PRINT_TIME(function)
#define HUGGLE_PROFILER_RESET
#define HUGGLE_PROFILER_TIME 0

#endif

#endif // HUGGLEPROFILER_HPP
