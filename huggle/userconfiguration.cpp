//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userconfiguration.hpp"
#include "generic.hpp"
#include "exception.hpp"
#include "huggleoption.hpp"
#include "huggleparser.hpp"

using namespace Huggle;
using namespace Huggle::HuggleParser;
Huggle::UserConfiguration::UserConfiguration()
{

}

Huggle::UserConfiguration::~UserConfiguration()
{
    QStringList ol = this->UserOptions.keys();
    while (ol.count())
    {
        HuggleOption *option = this->UserOptions[ol[0]];
        this->UserOptions.remove(ol[0]);
        delete option;
        ol.removeAt(0);
    }
}

HuggleOption *Huggle::UserConfiguration::GetOption(QString key)
{
    if (this->UserOptions.contains(key))
    {
        return this->UserOptions[key];
    }
    return nullptr;
}

QVariant Huggle::UserConfiguration::SetOption(QString key_, QString config_, QVariant default_)
{
    if (this->UserOptions.contains(key_))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it",
                                    "void Configuration::SetOption(QString key, QVariant data)");
    }
    QString d_ = default_.toString();
    QString value = ConfigurationParse(key_, config_, d_);
    HuggleOption *h;
    switch (default_.type())
    {
        case QVariant::Int:
            h = new HuggleOption(key_, value.toInt(), value == d_);
            break;
        case QVariant::Bool:
            h = new HuggleOption(key_, Generic::SafeBool(value), value == d_);
            break;
        default:
            h = new HuggleOption(key_, value, value == d_);
            break;
    }
    this->UserOptions.insert(key_, h);
    return h->GetVariant();
}

QStringList Huggle::UserConfiguration::SetUserOptionList(QString key_, QString config_, QStringList default_, bool CS)
{
    if (this->UserOptions.contains(key_))
    {
        // we must not add 2 same
        throw new Huggle::Exception("This option is already in a list you can't have multiple same keys in it",
                                    "void Configuration::SetUserOptionList(QString key, QVariant data)");
    }
    QStringList value = HuggleParser::ConfigurationParse_QL(key_, config_, default_, CS);
    HuggleOption *h = new HuggleOption(key_, value, value == default_);
    this->UserOptions.insert(key_, h);
    return value;
}

int Huggle::UserConfiguration::GetSafeUserInt(QString key_, int default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toInt();
    return default_value;
}

bool Huggle::UserConfiguration::GetSafeUserBool(QString key_, bool default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toBool();
    return default_value;
}

QString Huggle::UserConfiguration::GetSafeUserString(QString key_, QString default_value)
{
    HuggleOption *option = this->GetOption(key_);
    if (option != nullptr)
        return option->GetVariant().toString();

    return default_value;
}

