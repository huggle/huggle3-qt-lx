//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiutil.hpp"

using namespace Huggle;

EditQuery *WikiUtil::EditPage(QString page, QString text, QString summary, bool minor, QString BaseTimestamp, unsigned int section)
{
    // retrieve a token
    EditQuery *eq = new EditQuery();
    if (!summary.endsWith(Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle))
    {
        summary = summary + " " + Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle;
    }
    eq->RegisterConsumer("WikiUtil::EditPage");
    eq->Page = page;
    eq->BaseTimestamp = BaseTimestamp;
    QueryPool::HugglePool->PendingMods.append(eq);
    eq->text = text;
    eq->Section = section;
    eq->Summary = summary;
    eq->Minor = minor;
    eq->Process();
    return eq;
}

EditQuery *WikiUtil::EditPage(WikiPage *page, QString text, QString summary, bool minor, QString BaseTimestamp)
{
    if (page == NULL)
    {
        return NULL;
    }
    return EditPage(page->PageName, text, summary, minor, BaseTimestamp);
}
