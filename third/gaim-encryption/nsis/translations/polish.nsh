;;
;;  polish.nsh
;;
;;  Polish language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1250
;;  Author: Marek Habersack <grendel@caudium.net>
;;  Version 1, sept 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_POLISH} "Gaim-Encryption wymaga by Gaim by³ zainstalowany. Nale¿y zainstalowaæ Gaim przed instalacj¹ Gaim-Encryption."

LangString GAIM-ENCRYPTION_TITLE ${LANG_POLISH} "Wtyczka Gaim-Encryption dla Gaim" 

LangString BAD_GAIM_VERSION_1 ${LANG_POLISH} "Nieodpowiednia wersja.$\r$\n$\r$\nTa wersja wtyczki Gaim-Encryption zosta³a skompilowana dla wersji ${GAIM_VERSION} Gaim. Wydaje siê, ¿e zainstalowana wersja Gaim to" 

LangString BAD_GAIM_VERSION_2 ${LANG_POLISH} "$\r$\n$\r$\nOdwiedŸ http://gaim-encryption.sourceforge.net by uzyskaæ wiêcej informacji."

LangString UNKNOWN_GAIM_VERSION ${LANG_POLISH} "Nie potrafiê okreœliæ wersji zainstalowanego Gaim'a. Upewnij siê, ¿e jest to wersja ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_POLISH} "Instalator Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"

LangString WELCOME_TEXT  ${LANG_POLISH} "Uwaga: ta wersja wtyczki zosta³a zaprojektowana dla Gaim ${GAIM_VERSION} i nie bêdzie dzia³a³a z innymi wersjami Gaim.\r\n\r\nPrzy ka¿dej aktualizacji Gaim nale¿y równie¿ zaktualizowaæ tê wtyczkê.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_POLISH} "Proszê wskazaæ katalog w którym zainstalowano Gaim"

LangString DIR_INNERTEXT ${LANG_POLISH} "Instaluj w poni¿szym katalogu Gaim:"

LangString FINISH_TITLE ${LANG_POLISH} "Instalacja Gaim-Encryption v{GAIM_ENCRYPTION_VERSION} zakoñczona"

LangString FINISH_TEXT ${LANG_POLISH} "Aby Gaim móg³ u¿ywaæ nowej wtyczki nale¿y go zrestartowaæ a nastêpnie uaktywniæ wtyczkê w okienku konfiguracyjnym Gaim."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_POLISH} "Wtyczka encrypt.dll zostanie usuniêta z katalogu wtyczek Gaim. Kontynuowaæ?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_POLISH} "Wtyczka Gaim-Encryption (tylko usuwanie)"

LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_POLISH} "Skrypt deinstalacyjny nie móg³ usun¹æ wpisów w rejestrze dotycz¹cych wtyczki Gaim-Encryption.$\rJest prawdopodobne, ¿e inny u¿ytkownik równie¿ zainstalowa³ wtyczkê."

LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_POLISH} "Nie posiadasz wystarczaj¹cych uprawnien aby zainstalowaæ wtyczkê."

