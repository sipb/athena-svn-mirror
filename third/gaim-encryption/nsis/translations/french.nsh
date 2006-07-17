;;
;;  french.nsh
;;
;;  French language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 1252
;;  Author: Davy Defaud <davy.defaud@free.fr>
;;  Version 1, sept 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_FRENCH} "Gaim-Encryption est un greffon (plugin) pour Gaim. Vous devez d'abord installer Gaim avant d'installer Gaim-Encryption."

LangString GAIM-ENCRYPTION_TITLE ${LANG_FRENCH} "Gaim-Encryption, greffon de chiffrement pour Gaim"

LangString BAD_GAIM_VERSION_1 ${LANG_FRENCH} "Version incompatible.$\r$\n$\r$\nCette version du greffon Gaim-Encryption a été compilée pour la version ${GAIM_VERSION} de Gaim. Vous semblez posséder la version"

LangString BAD_GAIM_VERSION_2 ${LANG_FRENCH} "de Gaim.$\r$\n$\r$\nPour plus d'information, veuillez consulter le site internet http://gaim-encryption.sourceforge.net."

LangString UNKNOWN_GAIM_VERSION ${LANG_FRENCH} "Impossible de détecter la version de Gaim installée.  Veuillez vous assurer qu'il s'agit de la version ${GAIM_VERSION}."

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_FRENCH} "Installateur de Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} "
LangString WELCOME_TEXT  ${LANG_FRENCH} "Note: Cette version de Gaim-Encryption est conçue pour Gaim ${GAIM_VERSION}, elle ne s'installera et ne fonctionnera pas avec d'autres versions de Gaim.\r\n\r\nQuand vous mettez à jour votre version de Gaim, vous devez désinstaller ou mettre également à jour Gaim-Encryption.\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_FRENCH} "Veuillez indiquer le répertoire d'installation de Gaim."
LangString DIR_INNERTEXT ${LANG_FRENCH} "Installer dans ce dossier Gaim:"

LangString FINISH_TITLE ${LANG_FRENCH} "Installation de Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} terminée."
LangString FINISH_TEXT ${LANG_FRENCH} "Vous devez redémarrer Gaim pour charger le greffon, ensuite vous rendre dans les préférences de Gaim et activer le greffon (plugin) Gaim-Encryption."

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_FRENCH} "Le fichier encrypt.dll est sur le point d'être effacé de votre sous-répertoire Gaim/plugins. Voulez-vous continuer?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_FRENCH} "Désinstallation de Gaim-Encryption"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_FRENCH} "Le désinstallateur ne peut trouver les entrées de la base de registres concernant Gaim-Encryption.$\rIl semble qu'un autre utilisateur a installé le greffon."
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_FRENCH} "Vous ne possédez pas les privilèges nécessaires pour désinstaller Gaim-Encryption."
