//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef JSHIGHLIGHTER_HPP
#define JSHIGHLIGHTER_HPP

#include "../definitions.hpp"
#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace Huggle
{
    class HUGGLE_EX_CORE JSHighlighter : public QSyntaxHighlighter
    {
            Q_OBJECT
        public:
            JSHighlighter(QTextDocument *parent = nullptr);
        protected:
            void highlightBlock(const QString &text) override;
        private:
            struct HighlightingRule
            {
                QRegularExpression pattern;
                QTextCharFormat format;
            };
            QVector<HighlightingRule> highlightingRules;

            QRegularExpression commentStartExpression;
            QRegularExpression commentEndExpression;

            QTextCharFormat keywordFormat;
            QTextCharFormat classFormat;
            QTextCharFormat singleLineCommentFormat;
            QTextCharFormat multiLineCommentFormat;
            QTextCharFormat quotationFormat;
            QTextCharFormat functionFormat;
    };
}

#endif // JSHIGHLIGHTER_HPP
