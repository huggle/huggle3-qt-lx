#-------------------------------------------------
#
# Project created by QtCreator 2013-09-11T13:41:34
#
#-------------------------------------------------

CONFIG += qt

QT       += xml webkit core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = huggle
TEMPLATE = app
INCLUDEPATH += "C:\\python33\\include"
INCLUDEPATH += "/usr/include/python2.7"
#LIBS += "-lpython2.7"


SOURCES += main.cpp\
        mainwindow.cpp \
    login.cpp \
    core.cpp \
    configuration.cpp \
    preferences.cpp \
    oauth.cpp \
    query.cpp \
    apiquery.cpp \
    queryresult.cpp \
    exception.cpp \
    wikisite.cpp \
    oauthlogin.cpp \
    oauthloginquery.cpp \
    aboutform.cpp \
    hugglequeue.cpp \
    hugglelog.cpp \
    huggletool.cpp \
    huggleweb.cpp \
    terminalparser.cpp \
    wikiuser.cpp \
    wikipage.cpp \
    proxy.cpp \
    pythonengine.cpp \
    hugglequeuefilter.cpp \
    hugglefeed.cpp \
    hugglefeedproviderirc.cpp \
    hugglequeueitemlabel.cpp \
    wikiedit.cpp \
    hugglefeedproviderwiki.cpp \
    wlquery.cpp \
    processlist.cpp \
    ignorelist.cpp \
    message.cpp \
    scorewordsdbform.cpp \
    history.cpp \
    editquery.cpp \
    iextension.cpp \
    reportuser.cpp \
    blockuser.cpp \
    exceptionwindow.cpp \
    waitingform.cpp \
    speedyform.cpp \
    deleteform.cpp \
    webserverquery.cpp \
    historyform.cpp \
    userinfoform.cpp \
    hooks.cpp \
    revertquery.cpp \
    protectpage.cpp \
    vandalnw.cpp \
    networkirc.cpp \
    updateform.cpp \
    uaareport.cpp \
    collectable.cpp \
    gc.cpp \
    hugglemassrollback.cpp \
    hugglenuke.cpp \
    requestprotect.cpp \
    sessionform.cpp \
    whitelistform.cpp \
    localization.cpp \
    syslog.cpp \
    huggleparser.cpp

HEADERS  += mainwindow.hpp \
    login.hpp \
    core.hpp \
    configuration.hpp \
    preferences.hpp \
    oauth.hpp \
    query.hpp \
    apiquery.hpp \
    queryresult.hpp \
    exception.hpp \
    wikisite.hpp \
    oauthlogin.hpp \
    oauthloginquery.hpp \
    aboutform.hpp \
    hugglequeue.hpp \
    hugglelog.hpp \
    huggletool.hpp \
    huggleweb.hpp \
    terminalparser.hpp \
    wikiuser.hpp \
    wikipage.hpp \
    proxy.hpp \
    pythonengine.hpp \
    hugglequeuefilter.hpp \
    hugglefeed.hpp \
    hugglefeedproviderirc.hpp \
    hugglequeueitemlabel.hpp \
    wikiedit.hpp \
    hugglefeedproviderwiki.hpp \
    wlquery.hpp \
    processlist.hpp \
    ignorelist.hpp \
    message.hpp \
    scorewordsdbform.hpp \
    history.hpp \
    editquery.hpp \
    iextension.hpp \
    reportuser.hpp \
    blockuser.hpp \
    exceptionwindow.hpp \
    waitingform.hpp \
    speedyform.hpp \
    deleteform.hpp \
    webserverquery.hpp \
    historyform.hpp \
    userinfoform.hpp \
    hooks.hpp \
    revertquery.hpp \
    protectpage.hpp \
    vandalnw.hpp \
    networkirc.hpp \
    updateform.hpp \
    uaareport.hpp \
    collectable.hpp \
    gc.hpp \
    hugglemassrollback.hpp \
    hugglenuke.hpp \
    requestprotect.hpp \
    sessionform.hpp \
    whitelistform.hpp \
    localization.hpp \
    syslog.hpp \
    huggleparser.hpp

FORMS    += mainwindow.ui \
    login.ui \
    preferences.ui \
    oauthlogin.ui \
    aboutform.ui \
    hugglequeue.ui \
    hugglelog.ui \
    huggletool.ui \
    huggleweb.ui \
    proxy.ui \
    hugglequeueitemlabel.ui \
    processlist.ui \
    ignorelist.ui \
    scorewordsdbform.ui \
    history.ui \
    reportuser.ui \
    blockuser.ui \
    exceptionwindow.ui \
    waitingform.ui \
    speedyform.ui \
    deleteform.ui \
    historyform.ui \
    userinfoform.ui \
    protectpage.ui \
    vandalnw.ui \
    updateform.ui \
    uaareport.ui \
    hugglemassrollback.ui \
    hugglenuke.ui \
    requestprotect.ui \
    sessionform.ui \
    whitelistform.ui

RESOURCES += \
    pictures.qrc \
    Version.qrc \
    html.qrc \
    text.qrc

OTHER_FILES += \
    Resources/Header.txt \
    Resources/Definitions.txt
