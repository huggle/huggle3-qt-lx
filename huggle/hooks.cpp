//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hooks.h"


void Huggle::Hooks::EditPreProcess(Huggle::WikiEdit *Edit)
{
    int extension = 0;
    while (extension < Huggle::iExtension::Extensions.count())
    {
        Huggle::iExtension *e = Huggle::iExtension::Extensions.at(extension);
        if (e->IsWorking())
        {
            e->Hook_EditPreProcess((void*)Edit);
        }
        extension++;
    }
}

void Huggle::Hooks::EditPostProcess(Huggle::WikiEdit *Edit)
{
    int extension = 0;
    while (extension < Huggle::iExtension::Extensions.count())
    {
        Huggle::iExtension *e = Huggle::iExtension::Extensions.at(extension);
        if (e->IsWorking())
        {
            e->Hook_EditPostProcess((void*)Edit);
        }
        extension++;
    }
}

void Huggle::Hooks::OnGood(Huggle::WikiEdit *Edit)
{
    Core::Main->VandalDock->Good(Edit);
}

void Huggle::Hooks::OnRevert(Huggle::WikiEdit *Edit)
{
    Core::Main->VandalDock->Rollback(Edit);
}

void Huggle::Hooks::OnWarning(Huggle::WikiUser *User)
{

}

void Huggle::Hooks::BadnessScore(Huggle::WikiUser *User, int Score)
{

}
