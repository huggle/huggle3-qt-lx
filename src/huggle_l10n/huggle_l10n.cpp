//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggle_l10n.hpp"

using namespace Huggle;

int Huggle_l10n::Init()
{
#ifdef HUGGLE_WIN
    extern int qInitResources_text();
    return qInitResources_text();
#else
    return -1;
#endif
}

QStringList Huggle_l10n::GetLocalizations()
{
    QStringList results;
    results << "ar" // Arabic
            << "bg" // Bulgarian
            << "bn" // Bengali
            << "br" // Brezhoneg
            << "cz" // Czech
            << "de" // Deutsch
            << "en" // English
            << "en-gb"
            << "es" // Spanish
            << "fa" // Persian
            << "fr" // French
            << "gu"
            << "he" // Hebrew
            << "hi" // Hindi
            << "hu" // Hungarian
            << "id" // Indonesian
            << "it" // Italian
            << "ja" // Japanese
            << "ka" // ?
            << "kk-cyrl"
    //      << "km" // Khmer
            << "kn" // Kannada
            << "ko" // Korean
            << "ksh"
            << "lb" // Lebanon
            << "lt" // Lithuanian
            << "lv" // Latvian
            << "mk" // Macedonian
            << "ml" // Malayalam
            << "mr" // Marathi
            << "ms"
            << "nl" // Dutch
            << "no" // Norwegian
            << "oc" // Occitan
            << "or" // Oriya
            << "pl" // Polish
            << "pt" // Portuguese
            << "pt-BR" // Portuguese (in Brazil)
            << "ro" // ??
            << "ru" // Russian
            << "sa"
            << "sv" // Swedish
            << "ta"
            << "tr" // Turkish
            << "uk" // Ukrainian
            << "ur" // Urdu
            << "zh" // Chinese
            << "zh-hant"; // Chinese hant
    return results;
}
