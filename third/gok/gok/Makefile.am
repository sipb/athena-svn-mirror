bin_PROGRAMS = gok create-branching-keyboard

noinst_PROGRAMS = test-gok-action test-gok-feedback test-gok-sound test-gok-bounds test-gok-wordcomplete keyboard-geometry 

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_Gok.server.in.in
server_DATA = $(server_in_files:.server.in.in=.server)
$(server_in_files:.server.in.in=.server.in): $(server_in_files)
	sed -e "s|\@BINDIR\@|$(bindir)|" $< > $@
@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
	test-gok-log.pl    \
	$(server_in_files)

CLEANFILES = $(server_DATA) $(server_DATA).in

INCLUDES = \
	-DGOK_LOCALEDIR=\""$(datadir)/locale"\"  \
	-DPREFIX=\""$(prefix)"\"                 \
	-DSYSCONFDIR=\""$(sysconfdir)"\"         \
	-DDATADIR=\""$(datadir)"\"               \
	-DLIBDIR=\""$(libdir)"\"                 \
	-I$(top_srcdir)                          \
	-I$(top_builddir)                        \
	$(GOK_CFLAGS)

LDADD = $(GOK_LIBS)\
	$(X_LIBS) \
	@LT_VERSION_INFO@

gok_LDFLAGS = -export-dynamic

TESTS = test-gok-action test-gok-feedback test-gok-bounds 

gok_SOURCES = \
	callbacks.c callbacks.h                          \
	gok-branchback-stack.c gok-branchback-stack.h    \
	gok-button.c gok-button.h                        \
	gok-chunker.c gok-chunker.h                      \
	gok-data.c gok-data.h                            \
	gok-repeat.c gok-repeat.h                        \
	gok-mousecontrol.c gok-mousecontrol.h            \
	gok-keyboard.c gok-keyboard.h                    \
	gok-key.c gok-key.h                              \
	gok-input.c gok-input.h				 \
	gok-scanner.c gok-scanner.h                      \
	gok-settings-dialog.c gok-settings-dialog.h      \
	gok-page-keysizespace.c gok-page-keysizespace.h  \
	gok-page-actions.c gok-page-actions.h            \
	gok-page-keyboard.c gok-page-keyboard.h          \
	gok-page-accessmethod.c gok-page-accessmethod.h  \
	gok-page-wordcomplete.c gok-page-wordcomplete.h  \
	gok-page-feedbacks.h gok-page-feedbacks.c        \
	gok-spy.c gok-spy.h gok-spy-priv.h               \
	gok.h                                            \
	main.c main.h                                    \
	switchapi.c switchapi.h                          \
	gtkjoyswitch.h                                   \
	gok-keyslotter.c gok-keyslotter.h                \
	gok-word-complete.c gok-word-complete.h          \
	word-complete.c word-complete.h                  \
	gok-utf8-word-complete.c			 \
	gok-utf8-word-complete.h			 \
	gok-sliding-window-word-complete.c		 \
	gok-sliding-window-word-complete.h		 \
	gok-sound.c gok-sound.h                          \
	gok-speech.c gok-speech.h                        \
	gok-modifier.c gok-modifier.h                    \
	gok-output.c gok-output.h                        \
	gok-action.c gok-action.h                        \
	gok-control.c gok-control.h                      \
	gok-log.c gok-log.h gok-log-priv.h               \
	gok-gconf-keys.h                                 \
	gok-editor.c gok-editor.h                        \
	global.h                                         \
	global.c                                         \
	arraylist.h                                      \
	arraylist.c                                      \
	command-common.h                                 \
	command-vertex-data.h                            \
	command-vertex-data.c                            \
	command-edge-data.h                              \
	command-edge-data.c                              \
	command-vertex.h                                 \
	command-vertex.c                                 \
	command-edge.h                                   \
	command-edge.c                                   \
	command-graph.h                                  \
	command-graph.c                                  \
	command-predictor.h                              \
	command-predictor.c                              \
	gok-log-reader.h                                 \
	gok-log-reader.c                                 \
	gok-predictor.h                                  \
	gok-predictor.c                                  \
	gok-gconf.c gok-gconf.h                          \
	gok-bounds.c gok-bounds.h                       \
	gok-feedback.c gok-feedback.h			\
	gok-composer.c	gok-composer.h			\
	gok-windowlister.c gok-windowlister.h		\
	gok-modifier-keymasks.c gok-modifier-keymasks.h \
	keysym2ucs.c					\
	gok-glade-helpers.c gok-glade-helpers.h

test_gok_action_SOURCES = \
	test-gok-action.c                                \
	gok-action.c gok-action.h                        \
	gok-gconf.c gok-gconf.h                          \
	gok-log.c gok-log.h gok-log-priv.h

test_gok_feedback_SOURCES = \
	test-gok-feedback.c                              \
	gok-feedback.c gok-feedback.h                    \
	gok-log.c gok-log.h gok-log-priv.h               \
	gok-gconf.c gok-gconf.h gok-gconf-keys.h         \
	gok-sound.c gok-sound.h				\
	gok-speech.c gok-speech.h

test_gok_sound_SOURCES = \
	test-gok-sound.c                                 \
	gok-sound.c gok-sound.h                          \
	gok-log.c gok-log.h gok-log-priv.h

test_gok_bounds_SOURCES = \
	test-gok-bounds.c                                \
	gok-bounds.c gok-bounds.h

test_gok_wordcomplete_SOURCES = \
	test-gok-wordcomplete.c \
	gok-word-complete.c \
	gok-word-complete.h \
	gok-utf8-word-complete.c \
	gok-utf8-word-complete.h \
	gok-sliding-window-word-complete.c \
	gok-sliding-window-word-complete.h \
	word-complete.c \
	word-complete.h 

keyboard_geometry_SOURCES = keyboard-geometry.c

create_branching_keyboard_SOURCES = create-branching-keyboard.c
