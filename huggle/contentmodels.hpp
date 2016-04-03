//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef CONTENTMODELS_H
#define CONTENTMODELS_H

#include "definitions.hpp"

namespace Huggle
{
    /*!
     * \brief Represents the content of a page. This class needs to be overriden with the specific content model.
     */
    class HUGGLE_EX WikiPageContent
    {
        public:
            WikiPageContent();
            virtual ~WikiPageContent();
            virtual QString GetContentAsString()=0;
            virtual QString GetContentModel()=0;
    };

    class WikiPageContentModel_WikiText : public WikiPageContent
    {
        public:
            WikiPageContentModel_WikiText(QString text);
            QString GetContentModel();
            QString GetContentAsString();
        private:
            QString Text;
    };
}

#endif // CONTENTMODELS_H
