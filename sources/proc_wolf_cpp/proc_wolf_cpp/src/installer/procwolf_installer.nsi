; Proc-Wolf Installer NSIS Script
!define APPNAME "Proc-Wolf"
!define COMPANY "whispr.dev"
!define VERSION "3.0.0"
!define INSTALLDIR "$PROGRAMFILES\${APPNAME}"

; Include Modern UI
!include "MUI2.nsh"

; General Settings
Name "${APPNAME} ${VERSION}"
OutFile "ProcWolf_Installer.exe"
InstallDir "${INSTALLDIR}"
RequestExecutionLevel admin
SetCompressor /SOLID lzma

; Icon settings (fixed path syntax)
Icon "resources\wolf.ico"
UninstallIcon "resources\wolf.ico"

; Version Information (fixed syntax)
VIProductVersion "3.0.0.0"
VIAddVersionKey "ProductName" "${APPNAME}"
VIAddVersionKey "CompanyName" "${COMPANY}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileDescription" "${APPNAME} Installer"
VIAddVersionKey "LegalCopyright" "© 2025 ${COMPANY}"

; Modern UI Configuration
!define MUI_ABORTWARNING
!define MUI_ICON "resources\wolf.ico"
!define MUI_UNICON "resources\wolf.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU "Application" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

; Variables
Var StartMenuFolder

; Main Installation Section
Section "Proc-Wolf Core" SecCore
  SectionIn RO  ; Read-only section (required)
  
  ; Set output path to the installation directory
  SetOutPath "$INSTDIR"
  
  ; Install files (fixed syntax)
  File "build\ProcWolfTray.exe"
  File "build\ProcWolfCLI.exe"
  File "build\ProcWolfService.exe"
  File "build\WhitelistManager.exe"
  File "resources\wolf.ico"
  
  ; Install whitelist directory
  SetOutPath "$INSTDIR\whitelist"
  File /r "whitelist\*.*"
  
  ; Create start menu shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk" "$INSTDIR\ProcWolfTray.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Whitelist Manager.lnk" "$INSTDIR\WhitelistManager.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall ${APPNAME}.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
  
  ; Create startup shortcut (fixed variable name)
  CreateShortCut "$SMSTARTUP\ProcWolf.lnk" "$INSTDIR\ProcWolfTray.exe"
  
  ; Create desktop shortcut (optional)
  CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\ProcWolfTray.exe"
  
  ; Write the uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Add uninstall information to Add/Remove Programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "${COMPANY}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$INSTDIR\wolf.ico"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
  
  ; Estimate installation size (in KB)
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" 5120
SectionEnd

; Optional section for Windows Service
Section "Install as Windows Service" SecService
  ExecWait '"$INSTDIR\ProcWolfService.exe" /install'
SectionEnd

; Section descriptions
LangString DESC_SecCore ${LANG_ENGLISH} "Core Proc-Wolf application files and tray utility."
LangString DESC_SecService ${LANG_ENGLISH} "Install Proc-Wolf as a Windows service for automatic startup."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecService} $(DESC_SecService)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller section
Section "Uninstall"
  ; Stop and remove service if installed
  ExecWait '"$INSTDIR\ProcWolfService.exe" /uninstall'
  
  ; Remove files
  Delete "$INSTDIR\ProcWolfTray.exe"
  Delete "$INSTDIR\ProcWolfCLI.exe"
  Delete "$INSTDIR\ProcWolfService.exe"
  Delete "$INSTDIR\WhitelistManager.exe"
  Delete "$INSTDIR\wolf.ico"
  Delete "$INSTDIR\Uninstall.exe"
  
  ; Remove whitelist directory
  RMDir /r "$INSTDIR\whitelist"
  
  ; Remove shortcuts
  Delete "$SMSTARTUP\ProcWolf.lnk"
  Delete "$DESKTOP\${APPNAME}.lnk"
  
  ; Remove start menu entries
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Whitelist Manager.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall ${APPNAME}.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  ; Remove installation directory
  RMDir "$INSTDIR"
  
  ; Remove registry entries
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
SectionEnd