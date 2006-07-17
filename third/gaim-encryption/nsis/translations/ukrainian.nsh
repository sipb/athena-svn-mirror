;;
;;  ukrainian.nsh
;;
;;  Ukrainian language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1251
;;  Author: Roman Sosenko <coloc75@yahoo.com>
;;  Version 1, Jan 2005
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_UKRAINIAN} "Перед встановленням Gaim encryption необхідно встановити Gaim. Встановіть Gaim."

LangString GAIM-ENCRYPTION_TITLE ${LANG_UKRAINIAN} "Модуль Gaim-Encryption для Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_UKRAINIAN} "Несумісна версія.$\r$\n$\r$\nЦя версія модуля Gaim-Encryption була створена для версії Gaim ${GAIM_VERSION}. На Вашому комп'ютері встановлена версія Gaim"

LangString BAD_GAIM_VERSION_2 ${LANG_UKRAINIAN} "Встановлено.$\r$\n$\r$\nДивіться http://gaim-encryption.sourceforge.net для більш детальної інформації."

LangString UNKNOWN_GAIM_VERSION ${LANG_UKRAINIAN} "Нам невідомо, яка версія Gaim встановлена на Вашому комп'ютері. Переконайтеся, що це версія ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_UKRAINIAN} "Інсталятор Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} "
LangString WELCOME_TEXT  ${LANG_UKRAINIAN} "Увага: Ця версія модуля створена для Gaim ${GAIM_VERSION}, і не може бути встановленою і працювати з іншими версіями Gaim.\r\n\r\nУ випадку оновлення версії Gaim Вам необхідно дезінсталювати чи обновити також і модуль.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_UKRAINIAN} "Будь ласка, знайдіть каталог, у якому встановлено Gaim"
LangString DIR_INNERTEXT ${LANG_UKRAINIAN} "Встановити у цб папку Gaim:"

LangString FINISH_TITLE ${LANG_UKRAINIAN} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Встановлення завершено"
LangString FINISH_TEXT ${LANG_UKRAINIAN} "Для завантаження модуля Вам буде необхідно запустити Gaim повторно, після чого в опціях Gaim активувати модуль Gaim-Encryption."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_UKRAINIAN} "Модуль encrypt.dll буде видалений з Вашого каталога Gaim/модуль. Продовжити?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_UKRAINIAN} "Модуль Gaim-Encryption  (лише видалення)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_UKRAINIAN} "Деінсталятор не може знайти елементи реєстра Gaim-Encryption.$\rЙмовірно, модуль був установлений іншим користувачем."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_UKRAINIAN} "У Вас немає прав для деінсталяції модуля."



