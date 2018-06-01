//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "jshighlighter.hpp"

using namespace Huggle;

JSHighlighter::JSHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    this->keywordFormat.setForeground(Qt::darkGray);
    this->keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\bclass\\b" << "\\bconst\\b" << "\\bbreak\\b"
                    << "\\bcase\\b" << "\\bcatch\\b" << "\\bcontinue\\b" << "\\bdebugger\\b" << "\\bdefault\\b"
                    << "\\bdelete\\b" << "\\bdo\\b" << "\\belse\\b" << "\\bfinally\\b" << "\\bfor\\b" << "\\bfunction\\b"
                    << "\\bif\\b" << "\\bin\\b" << "\\binstanceof\\b" << "\\bnew\\b" << "\\breturn\\b" << "\\bswitch\\b"
                    << "\\bthis\\b" << "\\bthrow\\b" << "\\btry\\b" << "\\btypeof\\b" << "\\bvar\\b" << "\\bvoid\\b"
                    << "\\bwhile\\b" << "\\bwith\\b" << "\\benum\\b" << "\\bnull\\b" << "\\bundefined\\b" << "\\bstatic\\b"
                    << "\\bexport\\b" << "\\bextends\\b" << "\\blet\\b" << "\\bsuper\\b" << "\\bimport\\b"
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = this->keywordFormat;
        this->highlightingRules.append(rule);
    }

    this->classFormat.setFontWeight(QFont::Bold);
    this->classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = this->classFormat;
    this->highlightingRules.append(rule);

    this->singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = this->singleLineCommentFormat;
    this->highlightingRules.append(rule);

    this->multiLineCommentFormat.setForeground(Qt::red);

    this->quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = this->quotationFormat;
    this->highlightingRules.append(rule);

    this->functionFormat.setFontItalic(true);
    this->functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = this->functionFormat;
    this->highlightingRules.append(rule);

    this->commentStartExpression = QRegularExpression("/\\*");
    this->commentEndExpression = QRegularExpression("\\*/");
}

void JSHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, this->highlightingRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            this->setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    this->setCurrentBlockState(0);

    int startIndex = 0;
    if (this->previousBlockState() != 1)
        startIndex = text.indexOf(this->commentStartExpression);

    while (startIndex >= 0)
    {
        QRegularExpressionMatch match = this->commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1)
        {
            this->setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else
        {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        this->setFormat(startIndex, commentLength, this->multiLineCommentFormat);
        startIndex = text.indexOf(this->commentStartExpression, startIndex + commentLength);
    }
}
