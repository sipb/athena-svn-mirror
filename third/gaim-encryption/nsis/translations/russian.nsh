;;
;;  russian.nsh
;;
;;  Russian language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1251
;;  Author: Roman Sosenko <coloc75@yahoo.com>
;;  Version 1, Dec 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_RUSSIAN} "Перед установкой Gaim encryption необходимо установить Gaim. Установите Gaim."

LangString GAIM-ENCRYPTION_TITLE ${LANG_RUSSIAN} "Модуль Gaim-Encryption для Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_RUSSIAN} "Несовместимая версия.$\r$\n$\r$\nЭта версия модуля Gaim-Encryption была создана для версии Gaim ${GAIM_VERSION}. На Вашем компьютере установлена версия Gaim"

LangString BAD_GAIM_VERSION_2 ${LANG_RUSSIAN} "установлено.$\r$\n$\r$\nСмотрите http://gaim-encryption.sourceforge.net для более подробной информации."

LangString UNKNOWN_GAIM_VERSION ${LANG_RUSSIAN} "Нам неизвестно, какая версия Gaim установлена на Вашем компьютере. Убедитесь, что это версия ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_RUSSIAN} "Инсталлятор Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} "
LangString WELCOME_TEXT  ${LANG_RUSSIAN} "Внимание: Настоящая версия модуля создана для Gaim ${GAIM_VERSION}, и не может быть установлена и функционировать с другими версиями Gaim.\r\n\r\nВ случае обновления версии Gaim Вам необходимо дезинсталлировать или обновить также и модуль.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_RUSSIAN} "Пожалуйста, найдите каталог, в котором установлен Gaim"
LangString DIR_INNERTEXT ${LANG_RUSSIAN} "Установить в эту папку Gaim:"

LangString FINISH_TITLE ${LANG_RUSSIAN} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Установка завершена"
LangString FINISH_TEXT ${LANG_RUSSIAN} "Для загрузки модуля Вам будет необходимо запустить Gaim заново, затем в опциях Gaim активировать модуль Gaim-Encryption."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_RUSSIAN} "Модуль encrypt.dll будет удалён с Вашего каталога Gaim/модуль. Продолжить?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_RUSSIAN} "Модуль Gaim-Encryption  (только удаление)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_RUSSIAN} "Деинсталлятор не может найти элементы реестра Gaim-Encryption.$\rВероятно, модуль был установлен другим пользователем."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_RUSSIAN} "У Вас нет прав для деинсталляции модуля."



