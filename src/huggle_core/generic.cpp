//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "generic.hpp"
#include <QUrl>
#include <QCryptographicHash>
#include "configuration.hpp"
#include "exception.hpp"
#include "hooks.hpp"
#include "localization.hpp"
#ifdef QT6_BUILD
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

using namespace Huggle;

QString Generic::Bool2String(bool b)
{
    if (b)
        return "true";
    return "false";
}

bool Generic::SafeBool(QString value, bool defaultvalue)
{
    value = value.toLower();
    if (value == "true")
    {
        return true;
    } else if (value == "false")
    {
        return false;
    }
    return defaultvalue;
}

QStringList Generic::CSV2QStringList(const QString& CSV, QChar separator)
{
    QStringList result;
    foreach(QString x, CSV.split(separator))
    {
        // Trim whitespace around the string, in case users used some extra spaces when separating CSV
        x = x.trimmed();
        // Remove the extra newline which could be present in last element
        x = x.replace("\n", "");
        if (!x.isEmpty())
        {
            // only use these items that contain something
           result.append(x);
        }
    }
    return result;
}

bool Generic::ReportPreFlightCheck()
{
    if (!Configuration::HuggleConfiguration->SystemConfig_AskUserBeforeReport)
        return true;
    return Hooks::ShowYesNoQuestion(_l("report-tu"), _l("report-warn"), false);
}

QString Generic::SanitizePath(QString name)
{
    QString new_name = name;
    while (new_name.contains("//"))
        new_name = new_name.replace("//", "/");
#ifdef HUGGLE_WIN
    return new_name.replace("/", "\\");
#else
    return name;
#endif // HUGGLE_WIN
}

QString Generic::SocketError2Str(QAbstractSocket::SocketError error)
{
    switch (error)
    {
        case QAbstractSocket::ConnectionRefusedError:
            return "Connection refused";
        case QAbstractSocket::RemoteHostClosedError:
            return "Remote host closed the connection unexpectedly";
        case QAbstractSocket::HostNotFoundError:
            return "Host not found";
        case QAbstractSocket::SocketAccessError:
            return "Socket access error";
        case QAbstractSocket::SocketResourceError:
            return "Socket resource error";
        case QAbstractSocket::SocketTimeoutError:
            return "Socket timeout error";
        case QAbstractSocket::DatagramTooLargeError:
            return "Datagram too large";
        case QAbstractSocket::NetworkError:
            return "Network error";
        case QAbstractSocket::AddressInUseError:
            return "AddressInUseError";
        case QAbstractSocket::SocketAddressNotAvailableError:
            return "SocketAdddressNotAvailableError";
        case QAbstractSocket::UnsupportedSocketOperationError:
            return "UnsupportedSocketOperationError";
        case QAbstractSocket::UnfinishedSocketOperationError:
            return "UnfinishedSocketOperationError";
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            return "ProxyAuthenticationRequiredError";
        case QAbstractSocket::SslHandshakeFailedError:
            return "SslHandshakeFailedError";
        case QAbstractSocket::ProxyConnectionRefusedError:
            return "ProxyConnectionRefusedError";
        case QAbstractSocket::ProxyConnectionClosedError:
            return "ProxyConnectionClosedError";
        case QAbstractSocket::ProxyConnectionTimeoutError:
            return "ProxyConnectionTimeoutError";
        case QAbstractSocket::ProxyNotFoundError:
            return "ProxyNotFoundError";
        case QAbstractSocket::ProxyProtocolError:
            return "ProxyProtocolError";
#ifdef HUGGLE_QTV5
        case QAbstractSocket::OperationError:
            return "OperationError";
        case QAbstractSocket::SslInternalError:
            return "SslInternalError";
        case QAbstractSocket::SslInvalidUserDataError:
            return "SslInvalidUserDataError";
        case QAbstractSocket::TemporaryError:
            return "TemporaryError";
#endif
        case QAbstractSocket::UnknownSocketError:
            return "UnknownError";
    }
    return "Unknown";
}

void Generic::DeveloperError()
{
    Hooks::ShowError("Function is restricted now", "You can't perform this action in"\
                     " developer mode, because you aren't logged into the wiki");
}

QString Generic::ShrinkText(const QString &text, int size, bool html, int minimum)
{
    if (size < 2 || size < minimum)
    {
        throw new Huggle::Exception("Parameter size must be more than 2", BOOST_CURRENT_FUNCTION);
    }
    // let's copy the text into new variable so that we don't break the original
    // who knows how these mutable strings are going to behave in qt :D
    QString text_ = text;
    int length = text_.length();
    if (length > size)
    {
        text_ = text_.mid(0, size - minimum);
        int cd = minimum;
        while (cd > 0)
        {
            text_ = text_ + ".";
            cd--;
        }
    } else while (text_.length() < size)
    {
        text_ += " ";
    }
    if (html)
    {
        text_.replace(" ", "&nbsp;");
    }
    return text_;
}

QString Generic::IRCQuitDefaultMessage()
{
    return "Huggle (" + hcfg->HuggleVersion + "), the anti vandalism software. See #huggle on irc://irc.libera.chat";
}

QString Generic::HtmlEncode(const QString& text)
{
    QString encoded;
    for(int i=0;i<text.size();++i)
    {
        QChar ch = text.at(i);
        if(ch.unicode() > 255)
            encoded += QString("&#%1;").arg(static_cast<int>(ch.unicode()));
        else
            encoded += ch;
    }
    encoded = encoded.replace("<", "&lt;");
    encoded = encoded.replace(">", "&gt;");
    return encoded;
}

QString Generic::MD5(const QString& data)
{
    return QString(QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md5).toHex());
}

bool Generic::SecondsToTimeSpan(int time, int *days, int *hours, int *minutes, int *seconds)
{
    if (time < 0)
        return false;

    int remaining_time = time;
    *days = remaining_time / (60 * 60 * 24);
    remaining_time -= *days * (60 * 60 * 24);
    *hours = remaining_time / (60 * 60);
    remaining_time -= *hours * (60 * 60);
    *minutes = remaining_time / 60;
    remaining_time -= *minutes * 60;
    *seconds = remaining_time;
    return true;
}

bool Generic::RegexExactMatch(const QString& regex, const QString& input_text)
{
#ifdef QT6_BUILD
    QRegularExpression re(regex);
    QRegularExpressionMatch match = re.match(input_text);
    return match.hasMatch() && (match.captured(0) == input_text);
#else
    QRegExp re(regex);
    return re.exactMatch(input_text);
#endif
}
