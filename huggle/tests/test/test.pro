#-------------------------------------------------
#
# Project created by QtCreator 2013-11-08T15:53:26
#
#-------------------------------------------------

CONFIG += qt
QT       += network opengl core gui webkit xml testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = tst_testmain
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_testmain.cpp \
    ../../wikiuser.cpp \
    ../../configuration.cpp \
    ../../localization.cpp \
    ../../wikisite.cpp \
    ../../wikipage.cpp \
    ../../core.cpp \
    ../../queryresult.cpp \
    ../../huggleparser.cpp \
    ../../query.cpp \
    ../../syslog.cpp \
    ../../gc.cpp \
    ../../exception.cpp \
    ../../deleteform.cpp \
    ../../updateform.cpp \
    ../../collectable.cpp \
    ../../blockuser.cpp \
    ../../apiquery.cpp \
    ../../message.cpp \
    ../../mainwindow.cpp \
    ../../login.cpp \
    ../../ignorelist.cpp \
    ../../iextension.cpp \
    ../../huggleweb.cpp \
    ../../huggletool.cpp \
    ../../hugglequeueitemlabel.cpp \
    ../../hugglequeuefilter.cpp \
    ../../hugglequeue.cpp \
    ../../hugglenuke.cpp \
    ../../hugglemassrollback.cpp \
    ../../hugglelog.cpp \
    ../../hugglefeedproviderwiki.cpp \
    ../../hugglefeedproviderirc.cpp \
    ../../hugglefeed.cpp \
    ../../hooks.cpp \
    ../../historyform.cpp \
    ../../history.cpp \
    ../../exceptionwindow.cpp \
    ../../editquery.cpp \
    ../../sleeper.cpp \
    ../../wlquery.cpp \
    ../../aboutform.cpp \
    ../../wikiedit.cpp \
    ../../waitingform.cpp \
    ../../whitelistform.cpp \
    ../../vandalnw.cpp \
    ../../userinfoform.cpp \
    ../../uaareport.cpp \
    ../../terminalparser.cpp \
    ../../speedyform.cpp \
    ../../sessionform.cpp \
    ../../scorewordsdbform.cpp \
    ../../revertquery.cpp \
    ../../requestprotect.cpp \
    ../../reportuser.cpp \
    ../../pythonengine.cpp \
    ../../webserverquery.cpp \
    ../../proxy.cpp \
    ../../protectpage.cpp \
    ../../processlist.cpp \
    ../../preferences.cpp \
    ../../oauthloginquery.cpp \
    ../../networkirc.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../wikiuser.hpp \
    ../../configuration.hpp \
    ../../wikisite.hpp \
    ../../wikipage.hpp \
    ../../core.hpp \
    ../../queryresult.hpp \
    ../../query.hpp \
    ../../gc.hpp \
    ../../exception.hpp \
    ../../collectable.hpp \
    ../../huggleparser.hpp \
    ../../syslog.cpp \
    ../../deleteform.hpp \
    ../../localization.hpp \
    ../../blockuser.hpp \
    ../../apiquery.hpp \
    ../../message.hpp \
    ../../mainwindow.hpp \
    ../../login.hpp \
    ../../ignorelist.hpp \
    ../../sleeper.hpp \
    ../../iextension.hpp \
    ../../huggleweb.hpp \
    ../../huggletool.hpp \
    ../../hugglequeueitemlabel.hpp \
    ../../hugglequeuefilter.hpp \
    ../../hugglequeue.hpp \
    ../../hugglenuke.hpp \
    ../../hugglemassrollback.hpp \
    ../../hugglelog.hpp \
    ../../hugglefeedproviderwiki.hpp \
    ../../hugglefeedproviderirc.hpp \
    ../../hugglefeed.hpp \
    ../../hooks.hpp \
    ../../historyform.hpp \
    ../../history.hpp \
    ../../whitelistform.hpp \
    ../../exceptionwindow.hpp \
    ../../editquery.hpp \
    ../../aboutform.hpp \
    ../../wlquery.hpp \
    ../../wikiedit.hpp \
    ../../waitingform.hpp \
    ../../vandalnw.hpp \
    ../../userinfoform.hpp \
    ../../uaareport.hpp \
    ../../terminalparser.hpp \
    ../../speedyform.hpp \
    ../../sessionform.hpp \
    ../../updateform.hpp \
    ../../scorewordsdbform.hpp \
    ../../revertquery.hpp \
    ../../requestprotect.hpp \
    ../../reportuser.hpp \
    ../../pythonengine.hpp \
    ../../webserverquery.hpp \
    ../../proxy.hpp \
    ../../protectpage.hpp \
    ../../processlist.hpp \
    ../../preferences.hpp \
    ../../oauthloginquery.hpp \
    ../../networkirc.hpp
