nonui_subdirs = util unicode
ui_subdirs    = widgets e-paned e-table e-text menus shortcut-bar

SUBDIRS = $(nonui_subdirs) $(ui_subdirs) 

lib_LTLIBRARIES = libgal.la

libgal_la_LDFLAGS = -version-info $(GAL_CURRENT):$(GAL_REVISION):$(GAL_AGE) \
		    $(ICONV_LIBS) $(GTK_LIBS)
libgal_la_SOURCES = bogus_automake_hack.c

# WARNING
# WARNING Be sure to add any files here into the child Makefile too.
# WARNING
libgal_la_LIBADD = \
	util/e-bit-array.lo				\
	util/e-cache.lo					\
	util/e-iconv.lo					\
	util/e-sorter-array.lo				\
	util/e-sorter.lo				\
	util/e-text-event-processor-emacs-like.lo	\
	util/e-text-event-processor.lo			\
	util/e-unicode-i18n.lo				\
	util/e-util.lo					\
	util/e-xml-utils.lo				\
	\
	unicode/gunibreak.lo				\
	unicode/gunicollate.lo				\
	unicode/gunidecomp.lo				\
	unicode/guniprop.lo				\
	unicode/gutf8.lo				\
	\
	widgets/color-group.lo				\
	widgets/color-palette.lo			\
	widgets/e-canvas-background.lo			\
	widgets/e-canvas-utils.lo			\
	widgets/e-canvas-vbox.lo			\
	widgets/e-canvas.lo				\
	widgets/e-categories-master-list-array.lo	\
	widgets/e-categories-master-list-combo.lo	\
	widgets/e-categories-master-list-dialog-model.lo\
	widgets/e-categories-master-list-dialog.lo	\
	widgets/e-categories-master-list.lo		\
	widgets/e-categories.lo				\
	widgets/e-colors.lo				\
	widgets/e-cursors.lo				\
	widgets/e-font.lo				\
	widgets/e-gui-utils.lo				\
	widgets/e-hscrollbar.lo				\
	widgets/e-hsv-utils.lo				\
	widgets/e-popup-menu.lo				\
	widgets/e-printable.lo				\
	widgets/e-reflow-model.lo			\
	widgets/e-reflow.lo				\
	widgets/e-scroll-frame.lo			\
	widgets/e-selection-model-array.lo		\
	widgets/e-selection-model-simple.lo		\
	widgets/e-selection-model.lo			\
	widgets/e-vscrollbar.lo				\
	widgets/e-unicode.lo				\
	widgets/gtk-combo-box.lo			\
	widgets/gtk-combo-stack.lo			\
	widgets/gtk-combo-text.lo			\
	widgets/widget-color-combo.lo			\
	widgets/widget-pixmap-combo.lo			\
	\
	e-paned/e-hpaned.lo				\
	e-paned/e-paned.lo				\
	e-paned/e-vpaned.lo				\
	\
	e-table/e-cell.lo				\
	e-table/e-cell-checkbox.lo			\
	e-table/e-cell-combo.lo				\
	e-table/e-cell-date.lo				\
	e-table/e-cell-float.lo				\
	e-table/e-cell-number.lo			\
	e-table/e-cell-pixbuf.lo			\
	e-table/e-cell-popup.lo				\
	e-table/e-cell-size.lo				\
	e-table/e-cell-spin-button.lo			\
	e-table/e-cell-text.lo				\
	e-table/e-cell-toggle.lo			\
	e-table/e-cell-tree.lo				\
	e-table/e-table.lo				\
	e-table/e-table-click-to-add.lo			\
	e-table/e-table-col.lo				\
	e-table/e-table-column-specification.lo		\
	e-table/e-table-config.lo			\
	e-table/e-table-extras.lo			\
	e-table/e-table-field-chooser-dialog.lo		\
	e-table/e-table-field-chooser-item.lo		\
	e-table/e-table-field-chooser.lo		\
	e-table/e-table-group.lo			\
	e-table/e-table-group-container.lo		\
	e-table/e-table-group-leaf.lo			\
	e-table/e-table-header.lo			\
	e-table/e-table-header-item.lo			\
	e-table/e-table-header-utils.lo			\
	e-table/e-table-item.lo				\
	e-table/e-table-memory-callbacks.lo		\
	e-table/e-table-memory.lo			\
	e-table/e-table-model.lo			\
	e-table/e-table-one.lo				\
	e-table/e-table-scrolled.lo			\
	e-table/e-table-selection-model.lo		\
	e-table/e-table-simple.lo			\
	e-table/e-table-sort-info.lo			\
	e-table/e-table-sorted.lo			\
	e-table/e-table-sorted-variable.lo		\
	e-table/e-table-sorter.lo			\
	e-table/e-table-sorting-utils.lo		\
	e-table/e-table-specification.lo		\
	e-table/e-table-state.lo			\
	e-table/e-table-subset.lo			\
	e-table/e-table-subset-variable.lo		\
	e-table/e-table-without.lo			\
	e-table/e-table-utils.lo			\
	e-table/e-tree-memory-callbacks.lo		\
	e-table/e-tree-memory.lo			\
	e-table/e-tree-model.lo				\
	e-table/e-tree-scrolled.lo			\
	e-table/e-tree-selection-model.lo		\
	e-table/e-tree-sorted.lo			\
	e-table/e-tree-table-adapter.lo			\
	e-table/e-tree.lo				\
	\
	e-text/e-completion-match.lo			\
	e-text/e-completion.lo				\
	e-text/e-completion-view.lo			\
	e-text/e-entry.lo				\
	e-text/e-table-text-model.lo			\
	e-text/e-text-model-repos.lo			\
	e-text/e-text-model.lo				\
	e-text/e-text-model-uri.lo			\
	e-text/e-text.lo				\
	\
	menus/gal-define-views-dialog.lo		\
	menus/gal-define-views-model.lo			\
	menus/gal-view-etable.lo			\
	menus/gal-view-factory-etable.lo		\
	menus/gal-view-factory.lo			\
	menus/gal-view-new-dialog.lo			\
	menus/gal-view-collection.lo			\
	menus/gal-view.lo				\
	\
	shortcut-bar/e-group-bar.lo			\
	shortcut-bar/e-icon-bar-bg-item.lo		\
	shortcut-bar/e-icon-bar.lo			\
	shortcut-bar/e-shortcut-bar.lo			\
	shortcut-bar/e-shortcut-model.lo		\
	shortcut-bar/e-vscrolled-bar.lo
# WARNING
# WARNING Be sure to add any files here into the child Makefile too.
# WARNING

