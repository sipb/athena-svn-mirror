;;
;;  english.nsh
;;
;;  English language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1252
;;  Author: Bill Tompkins <tompkins@icarion.com>
;;  Version 1, Nov 2003
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_ENGLISH} "Gaim-Encryption requires that Gaim be installed. You must install Gaim before installing Gaim-Encryption."

LangString GAIM-ENCRYPTION_TITLE ${LANG_ENGLISH} "Gaim-Encryption plugin for Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_ENGLISH} "Incompatible version.$\r$\n$\r$\nThis version of the Gaim-Encryption plugin was built for Gaim version ${GAIM_VERSION}.  It appears that you have Gaim version"

LangString BAD_GAIM_VERSION_2 ${LANG_ENGLISH} "installed.$\r$\n$\r$\nSee http://gaim-encryption.sourceforge.net for more information."

LangString UNKNOWN_GAIM_VERSION ${LANG_ENGLISH} "I can't tell what version of Gaim is installed.  Make sure that it is version ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_ENGLISH} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Installer"
LangString WELCOME_TEXT  ${LANG_ENGLISH} "Note: This version of the plugin is designed for Gaim ${GAIM_VERSION}, and will not install or function with other versions of Gaim.\r\n\r\nWhen you upgrade your version of Gaim, you must uninstall or upgrade this plugin as well.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_ENGLISH} "Please locate the directory where Gaim is installed"
LangString DIR_INNERTEXT ${LANG_ENGLISH} "Install in this Gaim folder:"

LangString FINISH_TITLE ${LANG_ENGLISH} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Install Complete"
LangString FINISH_TEXT ${LANG_ENGLISH} "You will need to restart Gaim for the plugin to be loaded, then go the Gaim preferences and enable the Gaim-Encryption Plugin."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_ENGLISH} "The encrypt.dll plugin is about to be deleted from your Gaim/plugins directory.  Continue?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_ENGLISH} "Gaim-Encryption Plugin (remove only)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_ENGLISH} "The uninstaller could not find registry entries for Gaim-Encryption.$\rIt is likely that another user installed the plugin."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_ENGLISH} "You do not have the permissions necessary to uninstall the plugin."



