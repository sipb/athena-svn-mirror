;;
;;  slovenian.nsh
;;
;;  Slovenian language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1250
;;  Author: Martin Srebotnjak <miles@filmsi.net>
;;  Version 1, oct 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_SLOVENIAN} "Šifriranje Gaim zahteva namešèeni Gaim. Pred namestitvijo Šifriranja Gaim morate namestiti Gaim."

LangString GAIM-ENCRYPTION_TITLE ${LANG_SLOVENIAN} "Vstavek Šifriranje Gaim za Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_SLOVENIAN} "Incompatible version.$\r$\n$\r$\nTa razlièica vstavka Šifriranja Gaim je prirejena za Gaim razlièice ${GAIM_VERSION}.  Videti je, da imate namešèeno Gaim razlièice "

LangString BAD_GAIM_VERSION_2 ${LANG_SLOVENIAN} ".$\r$\n$\r$\nZa veè informacij si poglejte stran http://gaim-encryption.sourceforge.net."

LangString UNKNOWN_GAIM_VERSION ${LANG_SLOVENIAN} "Ni mogoèe ugotoviti, katera razlièica Gaima je namešèena.  Preprièajte se, da je to razlièica ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_SLOVENIAN} "Namestitev Šifriranja Gaim v${GAIM-ENCRYPTION_VERSION}"
LangString WELCOME_TEXT  ${LANG_SLOVENIAN} "Opomba: Ta razlièica vstavka je prirejena za Gaim ${GAIM_VERSION} in ne bo namešèena ali delovala z drugimi razlièicami Gaima.\r\n\r\nKo nadgradite razlièico Gaima, ga morate odstraniti ali prav tako nadgraditi ta vstavek.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_SLOVENIAN} "Prosimo poišèite imenik, kjer je namešèen Gaim"
LangString DIR_INNERTEXT ${LANG_SLOVENIAN} "Namesti v ta imenik Gaim:"

LangString FINISH_TITLE ${LANG_SLOVENIAN} "Namestitev Šifriranja Gaim v${GAIM-ENCRYPTION_VERSION} dokonèana"
LangString FINISH_TEXT ${LANG_SLOVENIAN} "Za nalaganje vstavka morate ponovno zagnati Gaim ter v Možnostih Gaima omogoèiti vstavek Šifriranje Gaim."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_SLOVENIAN} "Datoteka encrypt.dll bo zbrisana iz imenika Gaim/plugins.  Želite nadaljevati?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_SLOVENIAN} "Vstavek Šifriranje Gaim (samo odstrani)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_SLOVENIAN} "Program za odstranitev programa v registru ne najde vnosov za Šifriranje Gaim.$\rVerjetno je, da je vstavek namestil drug uporabnik."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_SLOVENIAN} "Nimate potrebnih pravic za odstranitev vstavka."



