;;
;;  portugueseBR.nsh
;;
;;  Portuguese language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1252
;;  Author: Aury Fink Filho <nuny@aury.com.br>
;;  Version 1, oct 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_PORTUGUESEBR} "Gaim-Encryption requer que o Gaim esteja instalado. Você deve instalar o Gaim antes de instalar o Gaim-Encryption."

LangString GAIM-ENCRYPTION_TITLE ${LANG_PORTUGUESEBR} "Gaim-Encryption plugin para Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_PORTUGUESEBR} "Versão incompatível.$\r$\n$\r$\nEsta versão do plugin do Gaim-Encryption foi gerada para o Gaim versão ${GAIM_VERSION}.  Aparentemente, você tem o Gaim versão"

LangString BAD_GAIM_VERSION_2 ${LANG_PORTUGUESEBR} "instalado.$\r$\n$\r$\nVeja http://gaim-encryption.sourceforge.net para mais informações."

LangString UNKNOWN_GAIM_VERSION ${LANG_PORTUGUESEBR} "Eu não posso dizer qual versão do Gaim está instalada. Verifique se é a versão ${GAIM_VERSION}"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_PORTUGUESEBR} "Instalador do Gaim-Encryption v${GAIM-ENCRYPTION_VERSION}"
LangString WELCOME_TEXT  ${LANG_PORTUGUESEBR} "Nota: Essa versão do plugin foi feita para o Gaim ${GAIM_VERSION}, e não irá instalar ou funcionar com outras versões do Gaim.\r\n\r\nQuando você atualizar sua versão do Gaim, você deve desinstalar ou atualizar esse plugin também.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_PORTUGUESEBR} "Por favor, localize o diretório aonde o Gaim está instalado"
LangString DIR_INNERTEXT ${LANG_PORTUGUESEBR} "Instale nessa pasta do Gaim:"

LangString FINISH_TITLE ${LANG_PORTUGUESEBR} "Instalação do Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} Finalizada"
LangString FINISH_TEXT ${LANG_PORTUGUESEBR} "Você necessita reiniciar o Gaim para o plugin ser carregado, então vá para as preferências do Gaim e habilite o plugin do Gaim-Encryption."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_PORTUGUESEBR} "O plugin encrypt.dll está para ser deletado de seu diretório Gaim/plugins.  Continuar?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_PORTUGUESEBR} "Gaim-Encryption Plugin (apenas remover)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_PORTUGUESEBR} "O desintalador não pode encontrar as entradas no registro para o Gaim-Encryption.$\rAparentemente, outro usuário instalou o plugin."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_PORTUGUESEBR} "Você não tem as permissões necessárias para desinstalar o plugin."
