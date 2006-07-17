;;
;;  dutch.nsh
;;
;;  Default language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1252
;;  Author: Menno Jonkers <gaim-encryption@jonkers.com>
;;  Version 1, September 5, 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_DUTCH} "Gaim-encryptie vereist dat Gaim geïnstalleerd is.  U moet Gaim installeren voordat u Gaim-encryptie installeert."

LangString GAIM-ENCRYPTION_TITLE ${LANG_DUTCH} "Gaim-encryptie plugin voor Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_DUTCH} "Incompatibele versie.$\r$\n$\r$\nDeze versie van de Gaim-encryptie plugin is gemaakt voor Gaim versie ${GAIM_VERSION}.  Het lijkt erop dat u Gaim versie"

LangString BAD_GAIM_VERSION_2 ${LANG_DUTCH} "geïnstalleerd heeft.$\r$\n$\r$\nZie http://gaim-encryption.sourceforge.net voor meer informatie."

LangString UNKNOWN_GAIM_VERSION ${LANG_DUTCH} "Er kan niet worden vastgesteld welke versie van Gaim u geïnstalleerd heeft.  Controleert u dat dit versie ${GAIM_VERSION} is"


; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_DUTCH} "Gaim-encryptie v${GAIM-ENCRYPTION_VERSION} Installatie"
LangString WELCOME_TEXT  ${LANG_DUTCH} "Let op: deze versie van de plugin is gemaakt voor Gaim ${GAIM_VERSION} en zal niet installeren of werken met andere versies van Gaim.\r\n\r\nWanneer u uw versie van Gaim opwaardeert, dient u deze plugin ook te verwijderen of op te waarderen.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_DUTCH} "Blader naar de map waarin Gaim is geïnstalleerd"
LangString DIR_INNERTEXT ${LANG_DUTCH} "Installeer in deze Gaim map:"

LangString FINISH_TITLE ${LANG_DUTCH} "Gaim-encryptie v${GAIM-ENCRYPTION_VERSION} Installatie Voltooid"
LangString FINISH_TEXT ${LANG_DUTCH} "U dient Gaim te herstarten om de plugin beschikbaar te maken.  Ga dan in Gaim naar Voorkeuren en stel bij Plugins in dat Gaim-encryptie geladen wordt."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_DUTCH} "De encrypt.dll plugin staat op het punt verwijderd te worden uit uw Gaim/plugins map.  Doorgaan?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_DUTCH} "Gaim-encryptie plugin (alleen verwijderen)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_DUTCH} "Het verwijderpogramma kon in het register geen onderdelen vinden van Gaim-encryptie.$\rWaarschijnlijk is de plugin door een andere gebruiker geïnstalleerd."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_DUTCH} "U heeft niet de noodzakelijke rechten om de plugin te verwijderen."



