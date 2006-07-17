;;
;;  hungarian.nsh
;;
;;  Hungarian language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1250
;;  Author: Peter Tutervai <mrbay@csevego.net>
;;  Version 1, nov 2004 
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_HUNGARIAN} "A Gaim-Encryption telepítéséhez szükség van a Gaimra. Fel kell telepítened a Gaim-ot a Gaim-Encryption telepítése elõtt."

LangString GAIM-ENCRYPTION_TITLE ${LANG_HUNGARIAN} "Gaim-Encryption plugin a Gaimhoz"

LangString BAD_GAIM_VERSION_1 ${LANG_HUNGARIAN} "Nem kompatibilis verzió.$\r$\n$\r$\nA Gaim-Encryption plugin ezen verziója a Gaim ${GAIM_VERSION} verziójához lett lefordítva. Neked a "

LangString BAD_GAIM_VERSION_2 ${LANG_HUNGARIAN} "verziójú Gaim van feltelepítve.$\r$\n$\r$\nNézd meg a http://gaim-encryption.sourceforge.net webhelyet további információkért."

LangString UNKNOWN_GAIM_VERSION ${LANG_HUNGARIAN} "A feltelepített Gaim verziója ismeretlen. Bizonyosodjon meg róla, hogy a verziója ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_HUNGARIAN} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Telepítõ"
LangString WELCOME_TEXT  ${LANG_HUNGARIAN} "Fontos: A plugin ezen verziója a Gaim ${GAIM_VERSION} verziójához lett lefordítva, és nem lesz telepítve vagy nem fog futni a Gaim más verzióival.\r\n\r\nHa frissíti a Gaimot, törölje vagy frissítse a plugint is.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_HUNGARIAN} "Kérlek, add meg a Gaim helyét"
LangString DIR_INNERTEXT ${LANG_HUNGARIAN} "Telepítés ebbe a Gaim könyvtárba:"

LangString FINISH_TITLE ${LANG_HUNGARIAN} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Telepítése befejezõdött"
LangString FINISH_TEXT ${LANG_HUNGARIAN} "Újra kell indítanod a Gaimot, hogy betöltsön a plugin, majd a Gaim beállításokban be kell kapcsolnod Gaim-Encryption Plugint."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_HUNGARIAN} "Az encrypt.dll plugin törölve lesz a Gaim/plugins könyvtárból. Folytassam?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_HUNGARIAN} "Gaim-Encryption Plugin (csak törölhetõ)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_HUNGARIAN} "Az uninstaller nem talált bejegyzéseket a registryben a Gaim-Encryptionhöz.$\rValószínüleg másik felhasználó telepítette a Gaim-Encryptiont."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_HUNGARIAN} "Nincs jogod a Gaim-Encryption törléséhez."



