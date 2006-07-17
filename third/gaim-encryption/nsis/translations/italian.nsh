;;
;;  italian.nsh
;;
;;  Italian language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1252
;;  Author: Giacomo Succi <giacsu@sourceforge.net>
;;  Version 1, Sep 2004
;;


; Startup Gaim check
LangString GAIM_NEEDED ${LANG_ITALIAN} "Gaim-Encryption richiede che Gaim sia installato. Dovete installare Gaim prima di installare Gaim-Encryption."
LangString GAIM-ENCRYPTION_TITLE ${LANG_ITALIAN} "Gaim-Encryption, plugin per Gaim"
LangString BAD_GAIM_VERSION_1 ${LANG_ITALIAN} "Versione incompatibile.$\r$\n$\r$\nLa presente versione di Gaim-Encryption e' stata compilata per la versione di Gaim ${GAIM_VERSION}.  Sembra che avete una versione di Gaim incompatibile"
LangString BAD_GAIM_VERSION_2 ${LANG_ITALIAN} "installata.$\r$\n$\r$\nVisitate http://gaim-encryption.sourceforge.net per maggiori informazioni."
LangString UNKNOWN_GAIM_VERSION ${LANG_ITALIAN} "Non posso determinare quale versione di Gaim sia installata.  Siate certi che la versione sia la ${GAIM_VERSION}"

; Overrides for default text in windows:
LangString WELCOME_TITLE ${LANG_ITALIAN} "Installatore di Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"
LangString WELCOME_TEXT  ${LANG_ITALIAN} "Nota: Questo plugin e' stato pensato per Gaim ${GAIM_VERSION}, e non si installera' o funzionera' con altre versioni di Gaim.\r\n\r\nQuando aggiornerete la vostra versione di Gaim, dovete rimuovere o aggiornare questo plugin di conseguenza.\r\n\r\n"
LangString DIR_SUBTITLE ${LANG_ITALIAN} "Perfavore indicate la cartella dove Gaim e' installato"
LangString DIR_INNERTEXT ${LANG_ITALIAN} "Installa in questa cartella di Gaim:"
LangString FINISH_TITLE ${LANG_ITALIAN} "Installazione completata di Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"
LangString FINISH_TEXT ${LANG_ITALIAN} "Dovete chiudere e riavviare Gaim per far caricare il plugin, dopo di che andate nelle preferenze di Gaim e abilitate Gaim-Encryption Plugin."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_ITALIAN} "Il plugin encrypt.dll sta per essere rimosso dalla cartella Gaim/plugins.  Continuare?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_ITALIAN} "Gaim-Encryption Plugin (soltanto rimozione)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_ITALIAN} "L'uninstallatore non puo' trovare le entry nel registro per Gaim-Encryption.$\rE' probabile che un'altro utente abbia installato il plugin."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_ITALIAN} "Non avete i diritti necessari per rimuovere il plugin."
