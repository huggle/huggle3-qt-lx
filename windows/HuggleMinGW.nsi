############################################################################################
#      NSIS Installation Script created by NSIS Quick Setup Script Generator v1.09.18
#               Entirely Edited with NullSoft Scriptable Installation System                
#              by Vlasis K. Barkas aka Red Wine red_wine@freemail.gr Sep 2006               
############################################################################################

!define MUI_ICON "huggle.ico"
!define MUI_UNICON "huggle.ico"
!define APP_NAME "Huggle"
!define COMP_NAME "Wikimedia Project"
!define WEB_SITE "http://en.wikipedia.org/wiki/Wikipedia:Huggle"
!define VERSION "3.1.15.0"
!define COPYRIGHT "GPL"
!define DESCRIPTION "Application"
!define LICENSE_TXT "gpl.txt"
!define INSTALLER_NAME "setup.exe"
!define MAIN_APP_EXE "huggle.exe"
!define INSTALL_TYPE "SetShellVarContext current"
!define REG_ROOT "HKCU"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

!define REG_START_MENU "Start Menu Folder"

var SM_Folder

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor ZLIB
Name "${APP_NAME}"
Caption "${APP_NAME}"
Icon "huggle.ico"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
XPStyle on
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
InstallDir "$PROGRAMFILES\Huggle"

######################################################################

!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_TXT
!insertmacro MUI_PAGE_LICENSE "${LICENSE_TXT}"
!endif

!insertmacro MUI_PAGE_DIRECTORY

!ifdef REG_START_MENU
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Huggle"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${REG_ROOT}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_PATH}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${REG_START_MENU}"
!insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_APP_EXE}"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

####################### UNINSTALL BEFORE UPGRADE #####################

Section "" SecUninstallPrevious

    Call UninstallPrevious

SectionEnd

Function UninstallPrevious

    ; Check for uninstaller.
    DetailPrint "Checking for previous huggle versions"    
    ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
  "UninstallString"

    ${If} $R0 == ""        
         ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
        "UninstallString"
        ${If} $R0 == ""        
            DetailPrint "No previous installation found"    
            Goto Done
        ${EndIf}
    ${EndIf}

    DetailPrint "Removing previous installation."    
    ; Run the uninstaller silently.
    ExecWait '"$R0" /S _?=$INSTDIR' $0
    DetailPrint "Uninstaller returned $0"

    Done:

FunctionEnd

######################################################################

Section -MainProgram
${INSTALL_TYPE}
SetOverwrite ifnewer
SetOutPath "$INSTDIR"
RmDir /r "$INSTDIR\extensions"
File "release\huggle.exe"
File "release\huggle.ico"
File "release\py_hug.exe"
File "release\libcore.dll.a"
File "release\libcore.dll"
File "release\deps\ssleay32.dll"
File "release\deps\libeay32.dll"
File "release\deps\icuin53.dll"
File "release\deps\Qt5Core.dll"
File "release\deps\Qt5Declarative.dll"
File "release\deps\Qt5Multimedia.dll"
File "release\deps\icudt53.dll"
File "release\deps\Qt5Qml.dll"
File "release\deps\icuuc53.dll"
File "release\deps\Qt5Network.dll"
File "release\deps\Qt5XmlPatterns.dll"
File "release\deps\libstdc++-6.dll"
File "release\deps\Qt5OpenGL.dll"
File "release\deps\Qt5Sql.dll"
File "release\deps\Qt5Quick.dll"
File "release\deps\Qt5MultimediaWidgets.dll"
File "release\deps\Qt5Widgets.dll"
File "release\deps\Qt5WebKitWidgets.dll"
File "release\deps\Qt5WebChannel.dll"
File "release\deps\Qt5Positioning.dll"
File "release\deps\Qt5PrintSupport.dll"
File "release\deps\Qt5WebKit.dll"
File "release\deps\Qt5Xml.dll"
File "release\deps\Qt5Sensors.dll"
File "release\deps\Qt5Script.dll"
File "release\deps\Qt5Gui.dll"
File "release\deps\libwinpthread-1.dll"
File "release\deps\libgcc_s_dw2-1.dll"
SetOutPath "$INSTDIR\platforms"
File "release\platforms\qminimal.dll"
File "release\platforms\qoffscreen.dll"
File "release\platforms\qwindows.dll"
SetOutPath "$INSTDIR\extensions"
File "release\libhuggle_thanks.dll"
File "release\libhuggle_md.dll"
File "release\libhuggle_sh.dll"
File "release\libhuggle_scoring.dll"
File "release\libhuggle_en.dll"
File "release\libhuggle_nuke.dll"
SectionEnd

######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\uninstall.exe"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
CreateDirectory "$SMPROGRAMS\$SM_Folder"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "$INSTDIR\huggle.ico" 0
CreateShortCut "$SMPROGRAMS\$SM_Folder\PyHuggle.lnk" "$INSTDIR\py_hug.exe" "" "$INSTDIR\huggle.ico" 0
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "$INSTDIR\huggle.ico" 0
CreateShortCut "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!insertmacro MUI_STARTMENU_WRITE_END
!endif

!ifndef REG_START_MENU
CreateDirectory "$SMPROGRAMS\Huggle"
CreateShortCut "$SMPROGRAMS\Huggle\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\Huggle\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\Huggle\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!endif

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

!ifdef WEB_SITE
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "URLInfoAbout" "${WEB_SITE}"
!endif
SectionEnd

######################################################################

Section Uninstall
${INSTALL_TYPE}
Delete "$INSTDIR\${MAIN_APP_EXE}"
Delete "$INSTDIR\icuin53.dll"
Delete "$INSTDIR\libcore.dll.a"
Delete "$INSTDIR\libcore.dll"
Delete "$INSTDIR\Qt5Core.dll"
Delete "$INSTDIR\Qt5Multimedia.dll"
Delete "$INSTDIR\icudt53.dll"
Delete "$INSTDIR\Qt5Qml.dll"
Delete "$INSTDIR\icuuc53.dll"
Delete "$INSTDIR\Qt5XmlPatterns.dll"
Delete "$INSTDIR\Qt5Declarative.dll"
Delete "$INSTDIR\Qt5Network.dll"
Delete "$INSTDIR\extensions\libhuggle_thanks.dll"
Delete "$INSTDIR\libstdc++-6.dll"
Delete "$INSTDIR\extensions\libhuggle_md.dll"
Delete "$INSTDIR\extensions\libhuggle_sh.dll"
Delete "$INSTDIR\extensions\libhuggle_en.dll"
Delete "$INSTDIR\Qt5OpenGL.dll"
Delete "$INSTDIR\Qt5Sql.dll"
Delete "$INSTDIR\Qt5Quick.dll"
Delete "$INSTDIR\Qt5Positioning.dll"
Delete "$INSTDIR\Qt5MultimediaWidgets.dll"
Delete "$INSTDIR\Qt5Widgets.dll"
Delete "$INSTDIR\Qt5WebChannel.dll"
Delete "$INSTDIR\Qt5WebKitWidgets.dll"
Delete "$INSTDIR\Qt5Script.dll"
Delete "$INSTDIR\Qt5PrintSupport.dll"
Delete "$INSTDIR\Qt5WebKit.dll"
Delete "$INSTDIR\Qt5Sensors.dll"
Delete "$INSTDIR\Qt5Xml.dll"
Delete "$INSTDIR\ssleay32.dll"
Delete "$INSTDIR\libeay32.dll"
Delete "$INSTDIR\Qt5Gui.dll"
Delete "$INSTDIR\libwinpthread-1.dll"
Delete "$INSTDIR\py_hug.exe"
Delete "$INSTDIR\huggle.ico"
Delete "$INSTDIR\libgcc_s_dw2-1.dll"
Delete "$INSTDIR\platforms\qminimal.dll"
Delete "$INSTDIR\platforms\qoffscreen.dll"
Delete "$INSTDIR\platforms\qwindows.dll"
Delete "$INSTDIR\uninstall.exe"
!ifdef WEB_SITE
Delete "$INSTDIR\${APP_NAME} website.url"
!endif
RmDir /r "$INSTDIR\extensions"
RmDir /r "$INSTDIR\platforms"
RmDir "$INSTDIR"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_GETFOLDER "Application" $SM_Folder
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\$SM_Folder"
!endif

!ifndef REG_START_MENU
Delete "$SMPROGRAMS\Huggle\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\Huggle\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\Huggle\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\Huggle"
!endif

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
SectionEnd

######################################################################

