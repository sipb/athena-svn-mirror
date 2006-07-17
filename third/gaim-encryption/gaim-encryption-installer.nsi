;NSIS Script For Gaim-Encryption Plugin (MUI version)
; Based on installers by Mike Campell and Daniel Atallah, and on the Gaim installer by
; Herman Bloggs.  Many thanks!
; probably will not work with older versions of NSIS
; Requires NSIS 2.0 or greater

Name "Gaim-Encryption ${GAIM-ENCRYPTION_VERSION}"
!define MY_NAME Name

; Registry keys:
!define GAIM_ENCRYPTION_REG_KEY         "SOFTWARE\gaim-encryption"
!define GAIM_ENCRYPTION_UNINSTALL_KEY   "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\gaim-encryption"
!define GAIM_ENCRYPTION_UNINST_EXE      "gaim-encryption-uninst.exe"

!define ALL_LINGUAS "cs da de es fr hu it ja nl pl pt_BR ru sl uk zh_TW"

!include "MUI.nsh"

;Do A CRC Check
CRCCheck On

;Output File Name
OutFile "gaim-encryption-${GAIM-ENCRYPTION_VERSION}.exe"

ShowInstDetails show
ShowUnInstDetails show
SetCompressor lzma

; Translations


; Modern UI Configuration

!define MUI_ICON .\nsis\install.ico
!define MUI_UNICON .\nsis\install.ico
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "nsis\header.bmp"

; Gaim Plugin installer helper stuff
!addincludedir "..\..\src\win32\nsis"
!include "gaim-plugin.nsh"

; Pages
; !insertmacro MUI_LANGDLL_DISPLAY

!define MUI_WELCOMEPAGE_TITLE $(WELCOME_TITLE)
!define MUI_WELCOMEPAGE_TEXT $(WELCOME_TEXT)
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE  "./COPYING"

!define MUI_DIRECTORYPAGE_TEXT_TOP $(DIR_SUBTITLE)
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION $(DIR_INNERTEXT)
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE $(FINISH_TITLE)
!define MUI_FINISHPAGE_TEXT $(FINISH_TEXT)
!insertmacro MUI_PAGE_FINISH

; MUI Config

!define MUI_CUSTOMFUNCTION_GUIINIT encrypt_checkGaimVersion
!define MUI_ABORTWARNING
!define MUI_UNINSTALLER
!define MUI_PROGRESSBAR smooth
!define MUI_INSTALLCOLORS /windows
;  !define MUI_FINISHPAGE_TEXT $(G-E_INSTALL_FINISHED)
;  !define MUI_FINISHPAGE_NOAUTOCLOSE
;!define MUI_TEXT_WELCOME_INFO_TEXT $(WELCOME_TEXT)
;!define MUI_TEXT_DIRECTORY_SUBTITLE $(DIR_SUBTITLE)
;!define MUI_INNERTEXT_DIRECTORY_TOP $(DIR_INNERTEXT)


;; Here in alphabetical order in native language
;; i.e. Danish, Deutsch, Espanol, Francais... , not Danish, French, German, Spanish...
;; English first as a default

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Ukrainian"

!include "nsis\translations\czech.nsh"
!include "nsis\translations\danish.nsh"
!include "nsis\translations\dutch.nsh"
!include "nsis\translations\english.nsh"
!include "nsis\translations\french.nsh"
!include "nsis\translations\hungarian.nsh"
!include "nsis\translations\german.nsh"
!include "nsis\translations\italian.nsh"
!include "nsis\translations\japanese.nsh"
!include "nsis\translations\polish.nsh"
!include "nsis\translations\portugueseBR.nsh"
!include "nsis\translations\russian.nsh"
!include "nsis\translations\spanish.nsh"
!include "nsis\translations\slovenian.nsh"
!include "nsis\translations\ukrainian.nsh"
!include "nsis\translations\trad-chinese.nsh"


!define MUI_LICENSEPAGE_RADIOBUTTONS


;The Default Installation Directory
InstallDir "$PROGRAMFILES\gaim"
InstallDirRegKey HKLM SOFTWARE\gaim ""

Section -SecUninstallOldPlugin
  ; Check install rights..
  Call CheckUserInstallRights
  Pop $R0

  StrCmp $R0 "HKLM" rights_hklm
  StrCmp $R0 "HKCU" rights_hkcu done

  rights_hkcu:
      ReadRegStr $R1 HKCU ${GAIM_ENCRYPTION_REG_KEY} ""
      ReadRegStr $R2 HKCU ${GAIM_ENCRYPTION_REG_KEY} "Version"
      ReadRegStr $R3 HKCU "${GAIM_ENCRYPTION_UNINSTALL_KEY}" "UninstallString"
      Goto try_uninstall

  rights_hklm:
      ReadRegStr $R1 HKLM ${GAIM_ENCRYPTION_REG_KEY} ""
      ReadRegStr $R2 HKLM ${GAIM_ENCRYPTION_REG_KEY} "Version"
      ReadRegStr $R3 HKLM "${GAIM_ENCRYPTION_UNINSTALL_KEY}" "UninstallString"

  ; If previous version exists .. remove
  try_uninstall:
    StrCmp $R1 "" done
      StrCmp $R2 "" uninstall_problem
        IfFileExists $R3 0 uninstall_problem
          ; Have uninstall string.. go ahead and uninstall.
          SetOverwrite on
          ; Need to copy uninstaller outside of the install dir
          ClearErrors
          CopyFiles /SILENT $R3 "$TEMP\${GAIM_ENCRYPTION_UNINST_EXE}"
          SetOverwrite off
          IfErrors uninstall_problem
            ; Ready to uninstall..
            ClearErrors
            ExecWait '"$TEMP\${GAIM_ENCRYPTION_UNINST_EXE}" /S _?=$R1'
            IfErrors exec_error
              Delete "$TEMP\${GAIM_ENCRYPTION_UNINST_EXE}"
              Goto done

            exec_error:
              Delete "$TEMP\${GAIM_ENCRYPTION_UNINST_EXE}"
              Goto uninstall_problem

        uninstall_problem:
            ; Just delete the plugin and uninstaller, and remove Registry key
             MessageBox MB_YESNO $(GAIM_ENCRYPTION_PROMPT_WIPEOUT) IDYES do_wipeout IDNO cancel_install
          cancel_install:
            Quit

          do_wipeout:
            StrCmp $R0 "HKLM" del_lm_reg del_cu_reg
            del_cu_reg:
              DeleteRegKey HKCU ${GAIM_ENCRYPTION_REG_KEY}
              Goto uninstall_prob_cont
            del_lm_reg:
              DeleteRegKey HKLM ${GAIM_ENCRYPTION_REG_KEY}

            uninstall_prob_cont:
              Delete "$R1\plugins\encrypt.dll"
              Delete "$R3"

  done:

SectionEnd


Section "Install"

  Call CheckUserInstallRights
  Pop $R0

  StrCmp $R0 "NONE" instrights_none
  StrCmp $R0 "HKLM" instrights_hklm instrights_hkcu

  instrights_hklm:
    ; Write the version registry keys:
    WriteRegStr HKLM ${GAIM_ENCRYPTION_REG_KEY} "" "$INSTDIR"
    WriteRegStr HKLM ${GAIM_ENCRYPTION_REG_KEY} "Version" "${GAIM-ENCRYPTION_VERSION}"

    ; Write the uninstall keys for Windows
    WriteRegStr HKLM ${GAIM_ENCRYPTION_UNINSTALL_KEY} "DisplayName" "$(GAIM_ENCRYPTION_UNINSTALL_DESC)"
    WriteRegStr HKLM ${GAIM_ENCRYPTION_UNINSTALL_KEY} "UninstallString" "$INSTDIR\${GAIM_ENCRYPTION_UNINST_EXE}"
    SetShellVarContext "all"
    Goto install_files

  instrights_hkcu:
    ; Write the version registry keys:
    WriteRegStr HKCU ${GAIM_ENCRYPTION_REG_KEY} "" "$INSTDIR"
    WriteRegStr HKCU ${GAIM_ENCRYPTION_REG_KEY} "Version" "${GAIM-ENCRYPTION_VERSION}"

    ; Write the uninstall keys for Windows
    WriteRegStr HKCU ${GAIM_ENCRYPTION_UNINSTALL_KEY} "DisplayName" "$(GAIM_ENCRYPTION_UNINSTALL_DESC)"
    WriteRegStr HKCU ${GAIM_ENCRYPTION_UNINSTALL_KEY} "UninstallString" "$INSTDIR\${GAIM_ENCRYPTION_UNINST_EXE}"
    Goto install_files
  
  instrights_none:
    ; No registry keys for us...
    
  install_files:
    SetOutPath $INSTDIR
    SetCompress Auto
    SetOverwrite on
    File /r ".\win32-install-dir\*.*"

    StrCmp $R0 "NONE" done
    CreateShortCut "$SMPROGRAMS\Gaim\Gaim-Encryption Uninstall.lnk" "$INSTDIR\${GAIM_ENCRYPTION_UNINST_EXE}"
    WriteUninstaller "$INSTDIR\${GAIM_ENCRYPTION_UNINST_EXE}"
    SetOverWrite off

  done:
SectionEnd

Section Uninstall
  Call un.CheckUserInstallRights
  Pop $R0
  StrCmp $R0 "NONE" no_rights
  StrCmp $R0 "HKCU" try_hkcu try_hklm

  try_hkcu:
    ReadRegStr $R0 HKCU ${GAIM_ENCRYPTION_REG_KEY} ""
    StrCmp $R0 $INSTDIR 0 cant_uninstall
      ; HKCU install path matches our INSTDIR.. so uninstall
      DeleteRegKey HKCU ${GAIM_ENCRYPTION_REG_KEY}
      DeleteRegKey HKCU "${GAIM_ENCRYPTION_UNINSTALL_KEY}"
      Goto cont_uninstall

  try_hklm:
    ReadRegStr $R0 HKLM ${GAIM_ENCRYPTION_REG_KEY} ""
    StrCmp $R0 $INSTDIR 0 try_hkcu
      ; HKLM install path matches our INSTDIR.. so uninstall
      DeleteRegKey HKLM ${GAIM_ENCRYPTION_REG_KEY}
      DeleteRegKey HKLM "${GAIM_ENCRYPTION_UNINSTALL_KEY}"
      ; Sets start menu and desktop scope to all users..
      SetShellVarContext "all"

  cont_uninstall:
    ; plugin 
    Delete "$INSTDIR\plugins\encrypt.dll"

     ; all locales
     Push $R0 ;save old values
     Push $R1
     Push $R2
     Push "${ALL_LINGUAS}"
     Pop $R0 ;initialize input array
     alllinguas:
     StrLen $R1 $R0 ;length of input array
     IntCmp $R1 0 alldone
     IntOp $R1 $R1 + 1
     uptodelimiter:
         IntOp $R1 $R1 - 1
         IntCmp $R1 0 fullof fullof
         StrCpy $R2 $R0 1 -$R1
         StrCmp $R2 "" fullof
         StrCmp $R2 " " partof
     Goto uptodelimiter
     partof:
         StrCpy $R2 $R0 -$R1
         IntOp $R1 $R1 - 1
         StrCpy $R0 $R0 "" -$R1
         Goto dodelwork
     fullof:
         StrCpy $R2 $R0
         StrCpy $R0 ""
     dodelwork:
     StrCpy $R1 "$INSTDIR\locale"
     Push $R1
     StrCpy $R1 "$R1\$R2"
     Push $R1
     StrCpy $R1 "$R1\LC_MESSAGES"
     Delete "$R1\gaim-encryption.mo"
     RMDir $R1
     Pop $R1
     RMDir $R1
     Pop $R1
     RMDir $R1
     Goto alllinguas
     alldone:
     Pop $R2 ;restore old values
     Pop $R1
     Pop $R0

    ; uninstaller shortcut
    Delete "$SMPROGRAMS\Gaim\Gaim-Encryption Uninstall.lnk"

    ; uninstaller
    Delete "$INSTDIR\${GAIM_ENCRYPTION_UNINST_EXE}"
    
    ; try to delete the Gaim directories, in case it has already uninstalled
    RMDir "$INSTDIR\plugins"
    RMDIR "$INSTDIR\locale"
    RMDir "$INSTDIR"
    RMDir "$SMPROGRAMS\Gaim"

    Goto done

  cant_uninstall:
    MessageBox MB_OK $(un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1) IDOK
    Quit

  no_rights:
    MessageBox MB_OK $(un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2) IDOK
    Quit

  done:
SectionEnd

Function .onVerifyInstDir
  IfFileExists $INSTDIR\gaim.exe Good1
    Abort
  Good1:
FunctionEnd

Function encrypt_checkGaimVersion
  Push $R0
  Push ${GAIM_VERSION}
  Call CheckGaimVersion
  Pop $R0

  StrCmp $R0 ${GAIM_VERSION_OK} encrypt_checkGaimVersion_OK

    StrCmp $R0 ${GAIM_VERSION_INCOMPATIBLE} +1 +6
    Call GetGaimVersion
    IfErrors +3
    Pop $R0
    MessageBox MB_OK|MB_ICONSTOP "$(BAD_GAIM_VERSION_1) $R0 $(BAD_GAIM_VERSION_2)"
    goto +2
    MessageBox MB_OK|MB_ICONSTOP "$(UNKNOWN_GAIM_VERSION)"
    Quit

  encrypt_checkGaimVersion_OK:
  Pop $R0

FunctionEnd

Function CheckUserInstallRights
        ClearErrors
        UserInfo::GetName
        IfErrors Win9x
        Pop $0
        UserInfo::GetAccountType
        Pop $1

        StrCmp $1 "Admin" 0 +3
                StrCpy $1 "HKLM"
                Goto done
        StrCmp $1 "Power" 0 +3
                StrCpy $1 "HKLM"
                Goto done
        StrCmp $1 "User" 0 +3
                StrCpy $1 "HKCU"
                Goto done
        StrCmp $1 "Guest" 0 +3
                StrCpy $1 "NONE"
                Goto done
        ; Unknown error
        StrCpy $1 "NONE"
        Goto done

        Win9x:
                StrCpy $1 "HKLM"

        done:
        Push $1
FunctionEnd

Function un.CheckUserInstallRights
        ClearErrors
        UserInfo::GetName
        IfErrors Win9x
        Pop $0
        UserInfo::GetAccountType
        Pop $1
        StrCmp $1 "Admin" 0 +3
                StrCpy $1 "HKLM"
                Goto done
        StrCmp $1 "Power" 0 +3
                StrCpy $1 "HKLM"
                Goto done
        StrCmp $1 "User" 0 +3
                StrCpy $1 "HKCU"
                Goto done
        StrCmp $1 "Guest" 0 +3
                StrCpy $1 "NONE"
                Goto done
        ; Unknown error
        StrCpy $1 "NONE"
        Goto done

        Win9x:
                StrCpy $1 "HKLM"

        done:
        Push $1
FunctionEnd

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
