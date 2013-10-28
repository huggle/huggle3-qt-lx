//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#ifndef HOOKS_H
#define HOOKS_H

#include <QString>
#include "wikipage.h"
#include "iextension.h"
#include "wikiuser.h"
#include "wikiedit.h"

namespace Huggle
{
    class WikiUser;

    class Hooks
    {
    public:
        static void EditPreProcess(WikiEdit *Edit);
        static void EditPostProcess(WikiEdit *Edit);
        static void OnGood(WikiEdit *Edit);
        static void OnRevert(WikiEdit *Edit);
        static void OnWarning(WikiUser *User);
        static void Suspicious(WikiEdit *Edit);
        static void BadnessScore(WikiUser *User, int Score);
    };
}

#endif // HOOKS_H
