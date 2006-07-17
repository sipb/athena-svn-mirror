;;
;;  japanese.nsh
;;
;;  Default language strings for the Windows Gaim-encryption NSIS installer.
;;  Windows Code page: 932
;;
;;  Author: Takeshi AIHANA <aihana@gnome.gr.jp>
;;  Version 1, sept 2004
;;

; Startup Gaim check
LangString GAIM_NEEDED ${LANG_JAPANESE} "Gaim-Encryption を利用するには Gaim がインストールされている必要があります。Gaim-Encryption をインストールする前に Gaim をインストールして下さい。"

LangString GAIM-ENCRYPTION_TITLE ${LANG_JAPANESE} "Gaim 向けの Gaim-Encryption プラグイン"

LangString BAD_GAIM_VERSION_1 ${LANG_JAPANESE} "バージョンが合っていません。$\r$\n$\r$\nこのバージョンの Gaim-Encryption プラグインは Gaim バージョン ${GAIM_VERSION} 向けに開発されたものです。次の Gaim バージョンをインストールしていると表示されます:"

LangString BAD_GAIM_VERSION_2 ${LANG_JAPANESE} "$\r$\n$\r$\nさらに詳細な情報な情報については http://gaim-encryption.sourceforge.net をご覧下さい。"

LangString UNKNOWN_GAIM_VERSION ${LANG_JAPANESE} "インストールされている Gaim のバージョンを取得できませんでした。バージョン ${GAIM_VERSION} であることを確認して下さい。"

; Overrides for default text in windows:

LangString WELCOME_TITLE ${LANG_JAPANESE} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} インストーラ"
LangString WELCOME_TEXT  ${LANG_JAPANESE} "注意: このバージョンのプラグインは、Gaim バージョン ${GAIM_VERSION} 向けに設計されたもので、それ以外のバージョンではインストール、または動作しないかもしれません。\r\n\r\nお使いの Gaim をアップグレードする際は同様に、このプラグインもアンインストール、またはアップグレードして下さい。\r\n\r\n"

LangString DIR_SUBTITLE ${LANG_JAPANESE} "Gaim がインストールされているフォルダを指定して下さい"
LangString DIR_INNERTEXT ${LANG_JAPANESE} "この Gaim フォルダの中にインストールする:"

LangString FINISH_TITLE ${LANG_JAPANESE} "Gaim-Encryption v${GAIM-ENCRYPTION_VERSION} のインストールが完了しました"
LangString FINISH_TEXT ${LANG_JAPANESE} "プラグインを読み込むために Gaim を再起動し、Gaim の設定ダイアログから Gaim-Encryption プラグインを有効にして下さい。"

; during install uninstaller
LangString GAIM_ENCRYPTION_PROMPT_WIPEOUT ${LANG_JAPANESE} "お使いの Gaim/プラグイン・フォルダからファイル encrypt.dll を削除します。続行してもよろしいですか?"

; for windows uninstall
LangString GAIM_ENCRYPTION_UNINSTALL_DESC ${LANG_JAPANESE} "Gaim-Encryption プラグイン (削除専用)"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_1 ${LANG_JAPANESE} "アンインストーラは Gaim-Encryption に対するレジストリのエントリを見つけることができませんでした。$\rおそらく誰か他のユーザがプラグインをインストールしたと思われます。"
LangString un.GAIM_ENCRYPTION_UNINSTALL_ERROR_2 ${LANG_JAPANESE} "このプラグインをアンインストールするのに必要な権限がありません。"



