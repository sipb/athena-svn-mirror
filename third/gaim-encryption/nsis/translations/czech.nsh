;;
;;  czech.nsh
;;
;;  Czech language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1250
;;  Author: Luboš Stanìk <lubek@users.sourceforge.net>
;;  Version 1, Nov 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_CZECH} "Doplnìk Gaim-Encryption vyžaduje nainstalovaný Gaim. Musíte nainstalovat Gaim pøed instalací doplòku Gaim-Encryption."

LangString GAIM-ENCRYPTION_TITLE ${LANG_CZECH} "Doplnìk Gaim-Encryption pro Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_CZECH} "Nekompatibilní verze.$\r$\n$\r$\nTato verze doplòku Gaim-Encryption byla vytvoøena pro Gaim ve verzi ${GAIM_VERSION}. Zdá se, že máte nainstalovánu verzi"

LangString BAD_GAIM_VERSION_2 ${LANG_CZECH} "programu Gaim.$\r$\n$\r$\nVíce informací získáte návštìvou http://gaim-encryption.sourceforge.net."

LangString UNKNOWN_GAIM_VERSION ${LANG_CZECH} "Nelze zjistit, jaká verze programu Gaim je nainstalována. Ujistìte se, že je to verze ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_CZECH} "Instalace doplòku Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"
LangString WELCOME_TEXT  ${LANG_CZECH} "Poznámka: Tato verze doplòku je navržena pro Gaim ${GAIM_VERSION}, nenainstaluje se ani nebude fungovat s jinými verzemi programu Gaim.\r\n\r\nKdyž aktualizujete svou verzi programu Gaim, musíte odinstalovat nebo aktualizovat také tento doplnìk.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_CZECH} "Lokalizujte prosím složku, kam je nainstalován program Gaim"
LangString DIR_INNERTEXT ${LANG_CZECH} "Instalovat do této složky programu Gaim:"

LangString FINISH_TITLE ${LANG_CZECH} "Instalace doplòku Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} je dokonèena"
LangString FINISH_TEXT ${LANG_CZECH} "Je tøeba restartovat Gaim, aby se doplnìk naèetl. Pak jdìte do nastavení programu Gaim a povolte doplnìk Gaim-Encryption."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_CZECH} "Doplnìk encrypt.dll má být vymazán z vaší složky doplòkù programu Gaim. Pokraèovat?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_CZECH} "Gaim-Encryption doplnìk (pouze odebrat)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_CZECH} "Odinstalátor nemùže najít položky registru pro Gaim-Encryption.$\rNejspíše instaloval doplnìk jiný uživatel."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_CZECH} "Nemáte dostateèná oprávnìní pro odinstalaci doplòku."



