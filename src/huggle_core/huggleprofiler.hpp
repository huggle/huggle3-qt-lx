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
#include <QHash>

#define HUGGLE_PROFILER_RESET Huggle::Profiler::Reset()
#define HUGGLE_PROFILER_INCRCALL(function) Huggle::Profiler::IncrementCall(function)
#define HUGGLE_PROFILER_TIME  Huggle::Profiler::GetTime()
#define HUGGLE_PROFILER_PRINT_TIME(function) Huggle::Syslog::HuggleLogs->DebugLog(QString("PROFILER: ") \
                                             + function + " finished in " + QString::number(Huggle::Profiler::GetTime()) \
                                             + "ms");\
                                             Huggle::Profiler::Reset()

namespace Huggle
{
    class HUGGLE_EX_CORE Profiler
    {
        public:
            static void Reset();
            static qint64 GetTime();
            static void IncrementCall(QString function);
            static unsigned long long GetCallsForFunction(QString function);
            static QList<QString> GetRegisteredCounterFunctions();
        private:
            static QHash<QString, unsigned long long> callCounter;
            static QDateTime ts;
    };
}
#else

#define HUGGLE_PROFILER_PRINT_TIME(function)
#define HUGGLE_PROFILER_RESET
constexpr int HUGGLE_PROFILER_TIME = 0;
#define HUGGLE_PROFILER_INCRCALL

#endif

#endif // HUGGLEPROFILER_HPP
