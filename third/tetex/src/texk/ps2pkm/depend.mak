# DO NOT DELETE

$(objdir)/arith.obj: arith.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	arith.h
$(objdir)/basename.obj: basename.c  \
	basics.h $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	filenames.h \
	$(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h
$(objdir)/bstring.obj: bstring.c ./c-auto.h
$(objdir)/curves.obj: curves.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	paths.h regions.h curves.h lines.h arith.h \
	basics.h
$(objdir)/encoding.obj: encoding.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h
$(objdir)/filenames.obj: filenames.c $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h basics.h \
	basics.h
$(objdir)/flisearch.obj: flisearch.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	filenames.h psearch.h \
	t1imager.h
$(objdir)/fontfcn.obj: fontfcn.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h fontmisc.h util.h \
	fontfcn.h
$(objdir)/hints.obj: hints.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	paths.h regions.h hints.h
$(objdir)/lines.obj: lines.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	regions.h lines.h \
	basics.h
$(objdir)/mag.obj: mag.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h
$(objdir)/objects.obj: objects.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	paths.h regions.h fonts.h pictures.h strokes.h cluts.h
$(objdir)/paths.obj: paths.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	paths.h regions.h fonts.h pictures.h strokes.h trig.h \
	
$(objdir)/pfb2pfa.obj: pfb2pfa.c basics.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h filenames.h \
	
$(objdir)/pk2bm.obj: pk2bm.c pkin.h \
	basics.h
$(objdir)/pkin.obj: pkin.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	 pkin.h \
	basics.h
$(objdir)/pkout.obj: pkout.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h pkout.h \
	
$(objdir)/pktest.obj: pktest.c basics.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	 pkout.h \
	basics.h
$(objdir)/ps2pk.obj: ps2pk.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	filenames.h psearch.h pkout.h ffilest.h types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h Xstuff.h fontmisc.h fontstruct.h \
	font.h fsmasks.h fontfile.h fontxlfd.h \
	basics.h
$(objdir)/psearch.obj: psearch.c $(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h strexpr.h filenames.h \
	texfiles.h
$(objdir)/regions.obj: regions.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	regions.h paths.h curves.h lines.h pictures.h fonts.h hints.h strokes.h \
	
$(objdir)/scanfont.obj: scanfont.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h t1stdio.h util.h \
	token.h fontfcn.h blues.h \
	strexpr.h
$(objdir)/sexpr.obj: sexpr.c basics.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	
$(objdir)/spaces.obj: spaces.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	paths.h pictures.h fonts.h arith.h trig.h \
	
$(objdir)/strexpr.obj: strexpr.c basics.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-fopen.h ./c-auto.h \
	
$(objdir)/t1funcs.obj: t1funcs.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h ffilest.h Xstuff.h \
	fontmisc.h fontstruct.h font.h fsmasks.h fontfile.h fontxlfd.h t1intf.h \
	objects.h spaces.h regions.h t1stdio.h util.h fontfcn.h
$(objdir)/t1info.obj: t1info.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h ffilest.h Xstuff.h \
	fontmisc.h fontstruct.h font.h fsmasks.h fontfile.h fontxlfd.h t1intf.h \
	./c-auto.h
$(objdir)/t1io.obj: t1io.c t1stdio.h types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h t1hdigit.h
$(objdir)/t1snap.obj: t1snap.c objects.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h types.h spaces.h \
	paths.h
$(objdir)/t1stub.obj: t1stub.c objects.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h types.h \
	ffilest.h
$(objdir)/t1test.obj: t1test.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h Xstuff.h fontmisc.h \
	fontstruct.h font.h fsmasks.h fontfile.h fontxlfd.h \
	texfiles.h
$(objdir)/token.obj: token.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h t1stdio.h util.h \
	digit.h token.h tokst.h hdigit.h
$(objdir)/type1.obj: type1.c types.h \
	$(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h objects.h spaces.h \
	paths.h fonts.h pictures.h util.h blues.h
$(objdir)/util.obj: util.c types.h $(kpathseadir)/kpathsea.h \
	$(kpathseadir)/config.h \
	$(kpathseadir)/c-auto.h \
	$(kpathseadir)/c-std.h \
	$(kpathseadir)/c-unistd.h \
	$(kpathseadir)/systypes.h \
	$(kpathseadir)/c-memstr.h \
	$(kpathseadir)/c-errno.h \
	$(kpathseadir)/c-minmax.h \
	$(kpathseadir)/c-limits.h \
	$(kpathseadir)/c-proto.h $(gnuw32dir)/win32lib.h \
	$(gnuw32dir)/oldnames.h $(kpathseadir)/debug.h \
	$(kpathseadir)/types.h \
	$(kpathseadir)/lib.h \
	$(kpathseadir)/progname.h \
	$(kpathseadir)/absolute.h \
	$(kpathseadir)/c-ctype.h \
	$(kpathseadir)/c-dir.h \
	$(kpathseadir)/c-fopen.h \
	$(kpathseadir)/c-namemx.h \
	$(kpathseadir)/c-pathch.h \
	$(kpathseadir)/c-pathmx.h \
	$(kpathseadir)/c-stat.h \
	$(kpathseadir)/c-vararg.h \
	$(kpathseadir)/cnf.h \
	$(kpathseadir)/concatn.h \
	$(kpathseadir)/db.h \
	$(kpathseadir)/str-list.h \
	$(kpathseadir)/default.h \
	$(kpathseadir)/expand.h \
	$(kpathseadir)/fn.h \
	$(kpathseadir)/fontmap.h \
	$(kpathseadir)/hash.h \
	$(kpathseadir)/getopt.h \
	$(kpathseadir)/line.h \
	$(kpathseadir)/magstep.h \
	$(kpathseadir)/paths.h \
	$(kpathseadir)/pathsearch.h \
	$(kpathseadir)/str-llist.h \
	$(kpathseadir)/proginit.h \
	$(kpathseadir)/readable.h \
	$(kpathseadir)/remote.h \
	$(kpathseadir)/tex-file.h \
	$(kpathseadir)/tex-glyph.h \
	$(kpathseadir)/tex-hush.h \
	$(kpathseadir)/tex-make.h \
	$(kpathseadir)/tilde.h \
	$(kpathseadir)/truncate.h \
	$(kpathseadir)/variable.h \
	$(kpathseadir)/xopendir.h \
	$(kpathseadir)/xstat.h ./c-auto.h util.h fontmisc.h
