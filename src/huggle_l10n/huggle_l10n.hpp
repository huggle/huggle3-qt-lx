//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLE_L10N_HPP
#define HUGGLE_L10N_HPP

#include <huggle_core/definitions.hpp>
#include <QStringList>

namespace Huggle
{
	class HUGGLE_EX_L10N Huggle_l10n
	{
    	public:
		    static int Init();
            static QStringList GetLocalizations();
	};
}

#endif
