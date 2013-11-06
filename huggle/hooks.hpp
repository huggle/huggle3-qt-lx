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
#include "wikipage.hpp"
#include "iextension.hpp"
#include "exception.hpp"
#include "wikiuser.hpp"
#include "wikiedit.hpp"

namespace Huggle
{
    class WikiUser;

    //! Hooks that can be used to attach some 3rd code to existing functions
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
