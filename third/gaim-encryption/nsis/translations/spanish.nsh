;;
;;  spanish.nsh
;;
;;  Spanish language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1252
;;  Author: Javier Fernández-Sanguino Peña <jfs@computer.org>
;;  Version 1, sept 2004 
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_SPANISH} "Gaim-Encryption necesita que Gaim esté instalado. Debe instalar Gaim antes de instalar Gaim-Encryption."

LangString GAIM-ENCRYPTION_TITLE ${LANG_SPANISH} "Complemento de Cifrado de Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_SPANISH} "Versión incompatible.$\r$\n$\r$\nEsta versión del complemento de Cifrado de Gaim se preparó para la versión ${GAIM_VERSION} de Gaim.  Parece que vd. tiene la versión de Gaim"

LangString BAD_GAIM_VERSION_2 ${LANG_SPANISH} "instalada.$\r$\n$\r$\nPara más información consulte http://gaim-encryption.sourceforge.net"

LangString UNKNOWN_GAIM_VERSION ${LANG_SPANISH} "No puedo determinar la versión de Gaim que tiene instalada. Asegúrese de que es la versión ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_SPANISH} "Instalador de Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"
LangString WELCOME_TEXT  ${LANG_SPANISH} "Aviso: Esta versión del complemento fue diseñada para la versión ${GAIM_VERSION} de Gaim y no se podrá instalar ni funcionará con otras versiones.\r\n\r\nCuando actualice su versión de Gaim debe desinstalar o actualizar este complemento.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_SPANISH} "Por favor, localice el directorio donde está instalado Gaim"
LangString DIR_INNERTEXT ${LANG_SPANISH} "Instalar en este directorio de Gaim:"

LangString FINISH_TITLE ${LANG_SPANISH} "Se ha completado la instalación de Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"
LangString FINISH_TEXT ${LANG_SPANISH} "Deberá reiniciar Gaim para que se cargue el complemento, después vaya a las preferencias de Gaim y active el complemento de Cifrado de Gaim."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_SPANISH} "Se va a borrar el complemento encrypt.dll de su directorio de complementos de Gaim. ¿Desea continuar?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_SPANISH} "Complemento Gaim-Encryption Plugin (sólo desinstalación)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_SPANISH} "El desinstalador no pudo encontrar las entradas de registro de Gaim-Encryption.$\rEs posible que otro usuario haya instalado el complemento."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_SPANISH} "No tiene los permisos necesarios para desinstalar el complemento."



