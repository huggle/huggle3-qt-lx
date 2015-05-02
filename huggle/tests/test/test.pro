#-------------------------------------------------
#
# Project created by QtCreator 2013-11-08T15:53:26
#
#-------------------------------------------------

CONFIG += c++11 qt
QT       += network opengl core gui webkit xml testlib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets webkitwidgets
}

QMAKE_CXXFLAGS += -std=c++11
TARGET = tst_testmain
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_testmain.cpp \
    ../../wikiuser.cpp \
    ../../mediawiki.cpp \
    ../../wikiutil.cpp \
    ../../querypool.cpp \
    ../../configuration.cpp \
    ../../localization.cpp \
    ../../wikisite.cpp \
    ../../wikipage.cpp \
    ../../resources.cpp \
    ../../core.cpp \
    ../../generic.cpp \
    ../../queryresult.cpp \
    ../../warninglist.cpp \
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
    ../../reloginform.cpp \
    ../../huggletool.cpp \
    ../../huggleweb.cpp \
    ../../hugglequeueitemlabel.cpp \
    ../../hugglequeuefilter.cpp \
    ../../hugglequeue.cpp \
    ../../warnings.cpp \
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
    ../../loadingform.cpp \
    ../../preferences.cpp \
    ../../oauthloginquery.cpp \
    ../../networkirc.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../wikiuser.hpp \
    ../../mediawiki.hpp \
    ../../configuration.hpp \
    ../../wikisite.hpp \
    ../../wikipage.hpp \
    ../../resources.hpp \
    ../../core.hpp \
    ../../queryresult.hpp \
    ../../query.hpp \
    ../../gc.hpp \
    ../../exception.hpp \
    ../../collectable.hpp \
    ../../huggleparser.hpp \
    ../../syslog.cpp \
    ../../deleteform.hpp \
    ../../warninglist.hpp \
    ../../localization.hpp \
    ../../blockuser.hpp \
    ../../apiquery.hpp \
    ../../message.hpp \
    ../../mainwindow.hpp \
    ../../login.hpp \
    ../../ignorelist.hpp \
    ../../sleeper.hpp \
    ../../reloginform.hpp \
    ../../iextension.hpp \
    ../../huggleweb.hpp \
    ../../huggletool.hpp \
    ../../wikiutil.hpp \
    ../../querypool.hpp \
    ../../hugglequeueitemlabel.hpp \
    ../../hugglequeuefilter.hpp \
    ../../hugglequeue.hpp \
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
    ../../generic.hpp \
    ../../vandalnw.hpp \
    ../../userinfoform.hpp \
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
    ../../warnings.hpp \
    ../../loadingform.hpp \
    ../../preferences.hpp \
    ../../oauthloginquery.hpp \
    ../../networkirc.hpp

RESOURCES += \
    Page.qrc
