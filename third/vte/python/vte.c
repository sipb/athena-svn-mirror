/* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 4 "vte.override"
#include <Python.h>
#include <pygtk/pygtk.h>
#include <pygobject.h>
#include <gtk/gtk.h>
#include "../src/vte.h"
#line 14 "vte.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGdkPixbuf_Type;
#define PyGdkPixbuf_Type (*_PyGdkPixbuf_Type)
static PyTypeObject *_PyGtkMenuShell_Type;
#define PyGtkMenuShell_Type (*_PyGtkMenuShell_Type)
static PyTypeObject *_PyGtkWidget_Type;
#define PyGtkWidget_Type (*_PyGtkWidget_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyVteTerminal_Type;


/* ----------- VteTerminal ----------- */

static int
_wrap_vte_terminal_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":VteTerminal.__init__", kwlist))
        return -1;
    self->obj = (GObject *)vte_terminal_new();

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create VteTerminal object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

#line 15 "vte.override"

static PyObject *
_wrap_vte_terminal_fork_command(PyGObject * self, PyObject * args,
				PyObject * kwargs)
{
	gchar **argv = NULL, **envv = NULL;
	gchar *command = NULL, *directory = NULL;
	static char *kwlist[] = { "command", "argv", "envv", "directory",
				  "loglastlog", "logutmp", "logwtmp",
				  NULL };
	PyObject *py_argv = NULL, *py_envv = NULL,
		 *loglastlog = NULL, *logutmp = NULL, *logwtmp = NULL;
	int i, n_args, n_envs;
	pid_t pid;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|sOOsOOO:fork_command",
					 kwlist, &command, &py_argv, &py_envv,
					 &directory,
					 &loglastlog, &logutmp, &logwtmp)) {
		return NULL;
	}


	if (py_argv != NULL && py_argv != Py_None) {
		if (!PySequence_Check(py_argv)) {
			PyErr_SetString(PyExc_TypeError,
					"argv must be a sequence");
			return NULL;
		}

		n_args = PySequence_Length(py_argv);
		argv = g_new(gchar *, n_args + 1);
		for (i = 0; i < n_args; i++) {
			PyObject *item = PySequence_GetItem(py_argv, i);
			Py_DECREF(item);  /* PySequence_GetItem INCREF's */
			argv[i] = PyString_AsString(item);
		}
		argv[n_args] = NULL;
	}

	if (py_envv != NULL && py_envv != Py_None) {
		if (!PySequence_Check(py_envv)) {
			PyErr_SetString(PyExc_TypeError,
					"envv must be a sequence");
			return NULL;
		}

		n_envs = PySequence_Length(py_envv);
		envv = g_new(gchar *, n_envs + 1);
		for (i = 0; i < n_envs; i++) {
			PyObject *item = PySequence_GetItem(py_envv, i);
			Py_DECREF(item);  /* PySequence_GetItem INCREF's */
			envv[i] = PyString_AsString(item);
		}
		envv[n_envs] = NULL;
	}

	pid = vte_terminal_fork_command(VTE_TERMINAL(self->obj),
					command, argv, envv, directory,
					(loglastlog != NULL) &&
					PyObject_IsTrue(loglastlog),
					(logutmp != NULL) &&
					PyObject_IsTrue(logutmp),
					(logwtmp != NULL) &&
					PyObject_IsTrue(logwtmp));

	if (argv) {
		g_free(argv);
	}

	return PyInt_FromLong(pid);
}
#line 122 "vte.c"


static PyObject *
_wrap_vte_terminal_feed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "length", NULL };
    char *data;
    int length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "si:VteTerminal.feed", kwlist, &data, &length))
        return NULL;
    vte_terminal_feed(VTE_TERMINAL(self->obj), data, length);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_feed_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "length", NULL };
    char *data;
    int length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "si:VteTerminal.feed_child", kwlist, &data, &length))
        return NULL;
    vte_terminal_feed_child(VTE_TERMINAL(self->obj), data, length);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_copy_clipboard(PyGObject *self)
{
    vte_terminal_copy_clipboard(VTE_TERMINAL(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_paste_clipboard(PyGObject *self)
{
    vte_terminal_paste_clipboard(VTE_TERMINAL(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_copy_primary(PyGObject *self)
{
    vte_terminal_copy_primary(VTE_TERMINAL(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_paste_primary(PyGObject *self)
{
    vte_terminal_paste_primary(VTE_TERMINAL(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "columns", "rows", NULL };
    int columns, rows;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii:VteTerminal.set_size", kwlist, &columns, &rows))
        return NULL;
    vte_terminal_set_size(VTE_TERMINAL(self->obj), columns, rows);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_audible_bell(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "is_audible", NULL };
    int is_audible;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_audible_bell", kwlist, &is_audible))
        return NULL;
    vte_terminal_set_audible_bell(VTE_TERMINAL(self->obj), is_audible);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_audible_bell(PyGObject *self)
{
    int ret;
    PyObject *py_ret;

    ret = vte_terminal_get_audible_bell(VTE_TERMINAL(self->obj));
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_set_visible_bell(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "is_visible", NULL };
    int is_visible;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_visible_bell", kwlist, &is_visible))
        return NULL;
    vte_terminal_set_visible_bell(VTE_TERMINAL(self->obj), is_visible);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_visible_bell(PyGObject *self)
{
    int ret;
    PyObject *py_ret;

    ret = vte_terminal_get_visible_bell(VTE_TERMINAL(self->obj));
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_set_scroll_on_output(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "scroll", NULL };
    int scroll;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_scroll_on_output", kwlist, &scroll))
        return NULL;
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(self->obj), scroll);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_scroll_on_keystroke(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "scroll", NULL };
    int scroll;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_scroll_on_keystroke", kwlist, &scroll))
        return NULL;
    vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(self->obj), scroll);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_color_bold(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bold", NULL };
    PyObject *py_bold;
    GdkColor *bold = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:VteTerminal.set_color_bold", kwlist, &py_bold))
        return NULL;
    if (pyg_boxed_check(py_bold, GDK_TYPE_COLOR))
        bold = pyg_boxed_get(py_bold, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "bold should be a GdkColor");
        return NULL;
    }
    vte_terminal_set_color_bold(VTE_TERMINAL(self->obj), bold);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_color_foreground(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "foreground", NULL };
    PyObject *py_foreground;
    GdkColor *foreground = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:VteTerminal.set_color_foreground", kwlist, &py_foreground))
        return NULL;
    if (pyg_boxed_check(py_foreground, GDK_TYPE_COLOR))
        foreground = pyg_boxed_get(py_foreground, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "foreground should be a GdkColor");
        return NULL;
    }
    vte_terminal_set_color_foreground(VTE_TERMINAL(self->obj), foreground);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_color_background(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "background", NULL };
    PyObject *py_background;
    GdkColor *background = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:VteTerminal.set_color_background", kwlist, &py_background))
        return NULL;
    if (pyg_boxed_check(py_background, GDK_TYPE_COLOR))
        background = pyg_boxed_get(py_background, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "background should be a GdkColor");
        return NULL;
    }
    vte_terminal_set_color_background(VTE_TERMINAL(self->obj), background);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 308 "vte.override"
static PyObject *
_wrap_vte_terminal_set_colors(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "foreground", "background", "palette", NULL };
    PyObject *py_foreground, *py_background, *py_palette, *item;
    int palette_size, i;
    GdkColor *foreground = NULL, *background = NULL, *palette = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOO:VteTerminal.set_colors", kwlist, &py_foreground, &py_background, &py_palette, &palette_size))
        return NULL;
    if (pyg_boxed_check(py_foreground, GDK_TYPE_COLOR))
        foreground = pyg_boxed_get(py_foreground, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "foreground should be a GdkColor");
        return NULL;
    }
    if (pyg_boxed_check(py_background, GDK_TYPE_COLOR))
        background = pyg_boxed_get(py_background, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "background should be a GdkColor");
        return NULL;
    }
    if (PySequence_Check(py_palette)) {
	palette_size = PySequence_Length(py_palette);
        palette = g_malloc(sizeof(GdkColor) * palette_size);
	for (i = 0; i < palette_size; i++) {
	    item = PySequence_GetItem(py_palette, i);
            if (!pyg_boxed_check(item, GDK_TYPE_COLOR)) {
		g_free(palette);
        	PyErr_SetString(PyExc_TypeError, "palette should be a list of GdkColors");
		return NULL;
	    }
	    palette[i] = *((GdkColor*)pyg_boxed_get(py_palette, GdkColor));
            Py_DECREF(item);
	}
    } else {
        PyErr_SetString(PyExc_TypeError, "palette should be a list of GdkColors");
        return NULL;
    }
    vte_terminal_set_colors(VTE_TERMINAL(self->obj), foreground, background, palette, palette_size);
    g_free(palette);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 379 "vte.c"


static PyObject *
_wrap_vte_terminal_set_default_colors(PyGObject *self)
{
    vte_terminal_set_default_colors(VTE_TERMINAL(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_background_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "image", NULL };
    PyGObject *image;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:VteTerminal.set_background_image", kwlist, &PyGdkPixbuf_Type, &image))
        return NULL;
    vte_terminal_set_background_image(VTE_TERMINAL(self->obj), GDK_PIXBUF(image->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_background_image_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "path", NULL };
    char *path;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:VteTerminal.set_background_image_file", kwlist, &path))
        return NULL;
    vte_terminal_set_background_image_file(VTE_TERMINAL(self->obj), path);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_background_saturation(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "saturation", NULL };
    double saturation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d:VteTerminal.set_background_saturation", kwlist, &saturation))
        return NULL;
    vte_terminal_set_background_saturation(VTE_TERMINAL(self->obj), saturation);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_background_transparent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "transparent", NULL };
    int transparent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_background_transparent", kwlist, &transparent))
        return NULL;
    vte_terminal_set_background_transparent(VTE_TERMINAL(self->obj), transparent);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_cursor_blinks(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "blink", NULL };
    int blink;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_cursor_blinks", kwlist, &blink))
        return NULL;
    vte_terminal_set_cursor_blinks(VTE_TERMINAL(self->obj), blink);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_scrollback_lines(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "lines", NULL };
    int lines;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_scrollback_lines", kwlist, &lines))
        return NULL;
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(self->obj), lines);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_im_append_menuitems(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "menushell", NULL };
    PyGObject *menushell;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:VteTerminal.im_append_menuitems", kwlist, &PyGtkMenuShell_Type, &menushell))
        return NULL;
    vte_terminal_im_append_menuitems(VTE_TERMINAL(self->obj), GTK_MENU_SHELL(menushell->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_font(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font_desc", NULL };
    PyObject *py_font_desc;
    PangoFontDescription *font_desc = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:VteTerminal.set_font", kwlist, &py_font_desc))
        return NULL;
    if (pyg_boxed_check(py_font_desc, PANGO_TYPE_FONT_DESCRIPTION))
        font_desc = pyg_boxed_get(py_font_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "font_desc should be a PangoFontDescription");
        return NULL;
    }
    vte_terminal_set_font(VTE_TERMINAL(self->obj), font_desc);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_font_from_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:VteTerminal.set_font_from_string", kwlist, &name))
        return NULL;
    vte_terminal_set_font_from_string(VTE_TERMINAL(self->obj), name);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_font(PyGObject *self)
{
    PangoFontDescription *ret;

    ret = vte_terminal_get_font(VTE_TERMINAL(self->obj));
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_vte_terminal_get_using_xft(PyGObject *self)
{
    int ret;
    PyObject *py_ret;

    ret = vte_terminal_get_using_xft(VTE_TERMINAL(self->obj));
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_set_allow_bold(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "allow_bold", NULL };
    int allow_bold;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_allow_bold", kwlist, &allow_bold))
        return NULL;
    vte_terminal_set_allow_bold(VTE_TERMINAL(self->obj), allow_bold);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_allow_bold(PyGObject *self)
{
    int ret;
    PyObject *py_ret;

    ret = vte_terminal_get_allow_bold(VTE_TERMINAL(self->obj));
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_get_has_selection(PyGObject *self)
{
    int ret;
    PyObject *py_ret;

    ret = vte_terminal_get_has_selection(VTE_TERMINAL(self->obj));
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_set_word_chars(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "spec", NULL };
    char *spec;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:VteTerminal.set_word_chars", kwlist, &spec))
        return NULL;
    vte_terminal_set_word_chars(VTE_TERMINAL(self->obj), spec);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_is_word_char(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "c", NULL };
    int ret;
    PyObject *py_ret;
    gunichar c;
    Py_UNICODE *py_c = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "u:VteTerminal.is_word_char", kwlist, &py_c))
        return NULL;
    if (py_c[1] != 0) {
        PyErr_SetString(PyExc_TypeError, "c should be a 1 character unicode string");
        return NULL;
    }
    c = (gunichar)py_c[0];
    ret = vte_terminal_is_word_char(VTE_TERMINAL(self->obj), c);
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_set_backspace_binding(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "binding", NULL };
    PyObject *py_binding = NULL;
    VteTerminalEraseBinding binding;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:VteTerminal.set_backspace_binding", kwlist, &py_binding))
        return NULL;
    if (pyg_enum_get_value(VTE_TYPE_TERMINAL_ERASE_BINDING, py_binding, (gint *)&binding))
        return NULL;
    vte_terminal_set_backspace_binding(VTE_TERMINAL(self->obj), binding);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_delete_binding(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "binding", NULL };
    PyObject *py_binding = NULL;
    VteTerminalEraseBinding binding;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:VteTerminal.set_delete_binding", kwlist, &py_binding))
        return NULL;
    if (pyg_enum_get_value(VTE_TYPE_TERMINAL_ERASE_BINDING, py_binding, (gint *)&binding))
        return NULL;
    vte_terminal_set_delete_binding(VTE_TERMINAL(self->obj), binding);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_mouse_autohide(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "setting", NULL };
    int setting;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:VteTerminal.set_mouse_autohide", kwlist, &setting))
        return NULL;
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(self->obj), setting);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_mouse_autohide(PyGObject *self)
{
    int ret;
    PyObject *py_ret;

    ret = vte_terminal_get_mouse_autohide(VTE_TERMINAL(self->obj));
    py_ret = ret ? Py_True : Py_False;
    Py_INCREF(py_ret);
    return py_ret;
}

static PyObject *
_wrap_vte_terminal_reset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "full", "clear_history", NULL };
    int full, clear_history;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii:VteTerminal.reset", kwlist, &full, &clear_history))
        return NULL;
    vte_terminal_reset(VTE_TERMINAL(self->obj), full, clear_history);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 89 "vte.override"

static gboolean
call_callback(VteTerminal *terminal, glong column, glong row, gpointer data)
{
    PyObject *cb, *self, *args, *result;
    gboolean ret;
    int i;
    if (!PyArg_ParseTuple(data, "|O:call_callback", &args)) {
        return FALSE;
    }
    args = PyList_New(0);
    cb = PySequence_GetItem(args, 0); /* INCREFs */
    Py_DECREF(cb);
    self = PySequence_GetItem(args, 1); /* INCREFs */
    Py_DECREF(self);
    PyList_Append(args, self);
    PyList_Append(args, PyInt_FromLong(column));
    PyList_Append(args, PyInt_FromLong(row));
    for (i = 2; i < PySequence_Length(args); i++) {
        PyObject *item = PySequence_GetItem(args, i);
	Py_DECREF(item);
        PyList_Append(args, item);
    }
    result = PyObject_CallObject(cb, args);
    ret = (result && PyObject_IsTrue(result));
    Py_DECREF(args);
    Py_DECREF(result);
    return ret;
}

static gboolean
always_true(VteTerminal *terminal, glong row, glong column, gpointer data)
{
    return TRUE;
}
static PyObject *
_wrap_vte_terminal_get_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "attributes", "data", NULL };
    PyObject *callback = NULL, *do_attr = NULL, *data = NULL;
    PyObject *callback_and_args = NULL;
    GArray *attrs = NULL;
    PyObject *text;
    PyObject *py_attrs;
    int count;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOO:terminal_get_text",
				     kwlist, &callback, &do_attr, &args)) {
        return NULL;
    }

    if (do_attr != NULL && do_attr != Py_None) {
	attrs = g_array_new(FALSE, TRUE, sizeof(struct vte_char_attributes));
    } else {
    	attrs = NULL;
    }

    if ((callback != NULL) && (callback != Py_None)) {
	if (!PyCallable_Check(callback)) {
	    PyErr_SetString(PyExc_TypeError, "1st argument not callable.");
            if (attrs) {
                g_array_free(attrs, TRUE);
            }
	    return NULL;
	}
    } else {
    	callback = NULL;
    }

    if (callback != NULL) {
        callback_and_args = PyList_New(0);
        PyList_Append(callback_and_args, callback);
        PyList_Append(callback_and_args, (PyObject*) self);
        if (PyList_Check(data)) {
            for (count = 0; count < PyList_Size(data); count++) {
                PyList_Append(callback_and_args, PyList_GetItem(data, count));
            }
        } else {
            PyList_Append(callback_and_args, data);
        }
    }

    text = PyString_FromString(vte_terminal_get_text(VTE_TERMINAL(self->obj),
						     callback ?
						     call_callback :
						     always_true,
						     callback ?
						     callback_and_args :
						     NULL,
						     attrs));
    Py_XDECREF(callback_and_args);

    if (attrs) {
	py_attrs = PyTuple_New(attrs->len);
	for (count = 0; count < attrs->len; count++) {
	    struct vte_char_attributes *cht;
	    PyObject *py_char_attr;
	    PyObject *py_gdkcolor;

	    cht = &g_array_index(attrs, struct vte_char_attributes, count);
	    py_char_attr = PyDict_New();
	    PyDict_SetItemString(py_char_attr, "row", PyInt_FromLong(cht->row));
	    PyDict_SetItemString(py_char_attr, "column", PyInt_FromLong(cht->column));
	    
	    py_gdkcolor = pyg_boxed_new(GDK_TYPE_COLOR, &cht->fore, TRUE, TRUE);
	    PyDict_SetItemString(py_char_attr, "fore", py_gdkcolor);
	    py_gdkcolor = pyg_boxed_new(GDK_TYPE_COLOR, &cht->back, TRUE, TRUE);
	    PyDict_SetItemString(py_char_attr, "back", py_gdkcolor);
	    
	    PyDict_SetItemString(py_char_attr, "underline", 
		    PyInt_FromLong(cht->underline));
	    PyDict_SetItemString(py_char_attr, "strikethrough", 
		    PyInt_FromLong(cht->strikethrough));

	    PyTuple_SetItem(py_attrs, count, py_char_attr);
	}
	g_array_free(attrs, TRUE);
	return Py_BuildValue("(OO)", text, py_attrs);
    } else {
    	return Py_BuildValue("O", text);
    }
}
#line 801 "vte.c"


#line 213 "vte.override"
static PyObject *
_wrap_vte_terminal_get_text_range(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "start_row", "start_col", "end_row",
			      "end_col", "attributes", "data", NULL };
    PyObject *callback = NULL, *do_attr = NULL, *data = NULL;
    glong start_row, start_col, end_row, end_col;
    PyObject *callback_and_args = NULL;
    GArray *attrs = NULL;
    PyObject *text;
    PyObject *py_attrs;
    int count;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|llllOOO:terminal_get_text",
				     kwlist,
				     &start_row, &start_col, &end_row, &end_col,
				     &callback, &do_attr, &args)) {
        return NULL;
    }

    if (do_attr != NULL && do_attr != Py_None) {
	attrs = g_array_new(FALSE, TRUE, sizeof(struct vte_char_attributes));
    } else {
    	attrs = NULL;
    }

    if ((callback != NULL) && (callback != Py_None)) {
	if (!PyCallable_Check(callback)) {
	    PyErr_SetString(PyExc_TypeError, "1st argument not callable.");
            if (attrs) {
                g_array_free(attrs, TRUE);
            }
	    return NULL;
	}
    } else {
    	callback = NULL;
    }

    if (callback != NULL) {
        callback_and_args = PyList_New(0);
        PyList_Append(callback_and_args, callback);
        PyList_Append(callback_and_args, (PyObject*) self);
        if (PyList_Check(data)) {
            for (count = 0; count < PyList_Size(data); count++) {
                PyList_Append(callback_and_args, PyList_GetItem(data, count));
            }
        } else {
            PyList_Append(callback_and_args, data);
        }
    }

    text = PyString_FromString(vte_terminal_get_text_range(VTE_TERMINAL(self->obj),
							   start_row, start_col,
							   end_row, end_col,
							   callback ?
							   call_callback :
							   always_true,
							   callback ?
							   callback_and_args :
							   NULL,
							   attrs));
    Py_XDECREF(callback_and_args);

    if (attrs) {
	py_attrs = PyTuple_New(attrs->len);
	for (count = 0; count < attrs->len; count++) {
	    struct vte_char_attributes *cht;
	    PyObject *py_char_attr;
	    PyObject *py_gdkcolor;

	    cht = &g_array_index(attrs, struct vte_char_attributes, count);
	    py_char_attr = PyDict_New();
	    PyDict_SetItemString(py_char_attr, "row", PyInt_FromLong(cht->row));
	    PyDict_SetItemString(py_char_attr, "column", PyInt_FromLong(cht->column));
	    
	    py_gdkcolor = pyg_boxed_new(GDK_TYPE_COLOR, &cht->fore, TRUE, TRUE);
	    PyDict_SetItemString(py_char_attr, "fore", py_gdkcolor);
	    py_gdkcolor = pyg_boxed_new(GDK_TYPE_COLOR, &cht->back, TRUE, TRUE);
	    PyDict_SetItemString(py_char_attr, "back", py_gdkcolor);
	    
	    PyDict_SetItemString(py_char_attr, "underline", 
		    PyInt_FromLong(cht->underline));
	    PyDict_SetItemString(py_char_attr, "strikethrough", 
		    PyInt_FromLong(cht->strikethrough));

	    PyTuple_SetItem(py_attrs, count, py_char_attr);
	}
	g_array_free(attrs, TRUE);
	return Py_BuildValue("(OO)", text, py_attrs);
    } else {
    	return Py_BuildValue("O", text);
    }
}
#line 898 "vte.c"


static PyObject *
_wrap_vte_terminal_match_clear_all(PyGObject *self)
{
    vte_terminal_match_clear_all(VTE_TERMINAL(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_match_add(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "match", NULL };
    char *match;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:VteTerminal.match_add", kwlist, &match))
        return NULL;
    ret = vte_terminal_match_add(VTE_TERMINAL(self->obj), match);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_set_emulation(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "emulation", NULL };
    char *emulation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:VteTerminal.set_emulation", kwlist, &emulation))
        return NULL;
    vte_terminal_set_emulation(VTE_TERMINAL(self->obj), emulation);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_emulation(PyGObject *self)
{
    const gchar *ret;

    ret = vte_terminal_get_emulation(VTE_TERMINAL(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_set_encoding(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "codeset", NULL };
    char *codeset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:VteTerminal.set_encoding", kwlist, &codeset))
        return NULL;
    vte_terminal_set_encoding(VTE_TERMINAL(self->obj), codeset);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_encoding(PyGObject *self)
{
    const gchar *ret;

    ret = vte_terminal_get_encoding(VTE_TERMINAL(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_status_line(PyGObject *self)
{
    const gchar *ret;

    ret = vte_terminal_get_status_line(VTE_TERMINAL(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_adjustment(PyGObject *self)
{
    GtkAdjustment *ret;

    ret = vte_terminal_get_adjustment(VTE_TERMINAL(self->obj));
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_vte_terminal_get_char_width(PyGObject *self)
{
    int ret;

    ret = vte_terminal_get_char_width(VTE_TERMINAL(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_get_char_height(PyGObject *self)
{
    int ret;

    ret = vte_terminal_get_char_height(VTE_TERMINAL(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_get_char_descent(PyGObject *self)
{
    int ret;

    ret = vte_terminal_get_char_descent(VTE_TERMINAL(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_get_char_ascent(PyGObject *self)
{
    int ret;

    ret = vte_terminal_get_char_ascent(VTE_TERMINAL(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_get_row_count(PyGObject *self)
{
    int ret;

    ret = vte_terminal_get_row_count(VTE_TERMINAL(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_get_column_count(PyGObject *self)
{
    int ret;

    ret = vte_terminal_get_column_count(VTE_TERMINAL(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_vte_terminal_get_window_title(PyGObject *self)
{
    const gchar *ret;

    ret = vte_terminal_get_window_title(VTE_TERMINAL(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_vte_terminal_get_icon_title(PyGObject *self)
{
    const gchar *ret;

    ret = vte_terminal_get_icon_title(VTE_TERMINAL(self->obj));
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef _PyVteTerminal_methods[] = {
    { "fork_command", (PyCFunction)_wrap_vte_terminal_fork_command, METH_VARARGS|METH_KEYWORDS },
    { "feed", (PyCFunction)_wrap_vte_terminal_feed, METH_VARARGS|METH_KEYWORDS },
    { "feed_child", (PyCFunction)_wrap_vte_terminal_feed_child, METH_VARARGS|METH_KEYWORDS },
    { "copy_clipboard", (PyCFunction)_wrap_vte_terminal_copy_clipboard, METH_NOARGS },
    { "paste_clipboard", (PyCFunction)_wrap_vte_terminal_paste_clipboard, METH_NOARGS },
    { "copy_primary", (PyCFunction)_wrap_vte_terminal_copy_primary, METH_NOARGS },
    { "paste_primary", (PyCFunction)_wrap_vte_terminal_paste_primary, METH_NOARGS },
    { "set_size", (PyCFunction)_wrap_vte_terminal_set_size, METH_VARARGS|METH_KEYWORDS },
    { "set_audible_bell", (PyCFunction)_wrap_vte_terminal_set_audible_bell, METH_VARARGS|METH_KEYWORDS },
    { "get_audible_bell", (PyCFunction)_wrap_vte_terminal_get_audible_bell, METH_NOARGS },
    { "set_visible_bell", (PyCFunction)_wrap_vte_terminal_set_visible_bell, METH_VARARGS|METH_KEYWORDS },
    { "get_visible_bell", (PyCFunction)_wrap_vte_terminal_get_visible_bell, METH_NOARGS },
    { "set_scroll_on_output", (PyCFunction)_wrap_vte_terminal_set_scroll_on_output, METH_VARARGS|METH_KEYWORDS },
    { "set_scroll_on_keystroke", (PyCFunction)_wrap_vte_terminal_set_scroll_on_keystroke, METH_VARARGS|METH_KEYWORDS },
    { "set_color_bold", (PyCFunction)_wrap_vte_terminal_set_color_bold, METH_VARARGS|METH_KEYWORDS },
    { "set_color_foreground", (PyCFunction)_wrap_vte_terminal_set_color_foreground, METH_VARARGS|METH_KEYWORDS },
    { "set_color_background", (PyCFunction)_wrap_vte_terminal_set_color_background, METH_VARARGS|METH_KEYWORDS },
    { "set_colors", (PyCFunction)_wrap_vte_terminal_set_colors, METH_VARARGS|METH_KEYWORDS },
    { "set_default_colors", (PyCFunction)_wrap_vte_terminal_set_default_colors, METH_NOARGS },
    { "set_background_image", (PyCFunction)_wrap_vte_terminal_set_background_image, METH_VARARGS|METH_KEYWORDS },
    { "set_background_image_file", (PyCFunction)_wrap_vte_terminal_set_background_image_file, METH_VARARGS|METH_KEYWORDS },
    { "set_background_saturation", (PyCFunction)_wrap_vte_terminal_set_background_saturation, METH_VARARGS|METH_KEYWORDS },
    { "set_background_transparent", (PyCFunction)_wrap_vte_terminal_set_background_transparent, METH_VARARGS|METH_KEYWORDS },
    { "set_cursor_blinks", (PyCFunction)_wrap_vte_terminal_set_cursor_blinks, METH_VARARGS|METH_KEYWORDS },
    { "set_scrollback_lines", (PyCFunction)_wrap_vte_terminal_set_scrollback_lines, METH_VARARGS|METH_KEYWORDS },
    { "im_append_menuitems", (PyCFunction)_wrap_vte_terminal_im_append_menuitems, METH_VARARGS|METH_KEYWORDS },
    { "set_font", (PyCFunction)_wrap_vte_terminal_set_font, METH_VARARGS|METH_KEYWORDS },
    { "set_font_from_string", (PyCFunction)_wrap_vte_terminal_set_font_from_string, METH_VARARGS|METH_KEYWORDS },
    { "get_font", (PyCFunction)_wrap_vte_terminal_get_font, METH_NOARGS },
    { "get_using_xft", (PyCFunction)_wrap_vte_terminal_get_using_xft, METH_NOARGS },
    { "set_allow_bold", (PyCFunction)_wrap_vte_terminal_set_allow_bold, METH_VARARGS|METH_KEYWORDS },
    { "get_allow_bold", (PyCFunction)_wrap_vte_terminal_get_allow_bold, METH_NOARGS },
    { "get_has_selection", (PyCFunction)_wrap_vte_terminal_get_has_selection, METH_NOARGS },
    { "set_word_chars", (PyCFunction)_wrap_vte_terminal_set_word_chars, METH_VARARGS|METH_KEYWORDS },
    { "is_word_char", (PyCFunction)_wrap_vte_terminal_is_word_char, METH_VARARGS|METH_KEYWORDS },
    { "set_backspace_binding", (PyCFunction)_wrap_vte_terminal_set_backspace_binding, METH_VARARGS|METH_KEYWORDS },
    { "set_delete_binding", (PyCFunction)_wrap_vte_terminal_set_delete_binding, METH_VARARGS|METH_KEYWORDS },
    { "set_mouse_autohide", (PyCFunction)_wrap_vte_terminal_set_mouse_autohide, METH_VARARGS|METH_KEYWORDS },
    { "get_mouse_autohide", (PyCFunction)_wrap_vte_terminal_get_mouse_autohide, METH_NOARGS },
    { "reset", (PyCFunction)_wrap_vte_terminal_reset, METH_VARARGS|METH_KEYWORDS },
    { "get_text", (PyCFunction)_wrap_vte_terminal_get_text, METH_VARARGS|METH_KEYWORDS },
    { "get_text_range", (PyCFunction)_wrap_vte_terminal_get_text_range, METH_VARARGS|METH_KEYWORDS },
    { "match_clear_all", (PyCFunction)_wrap_vte_terminal_match_clear_all, METH_NOARGS },
    { "match_add", (PyCFunction)_wrap_vte_terminal_match_add, METH_VARARGS|METH_KEYWORDS },
    { "set_emulation", (PyCFunction)_wrap_vte_terminal_set_emulation, METH_VARARGS|METH_KEYWORDS },
    { "get_emulation", (PyCFunction)_wrap_vte_terminal_get_emulation, METH_NOARGS },
    { "set_encoding", (PyCFunction)_wrap_vte_terminal_set_encoding, METH_VARARGS|METH_KEYWORDS },
    { "get_encoding", (PyCFunction)_wrap_vte_terminal_get_encoding, METH_NOARGS },
    { "get_status_line", (PyCFunction)_wrap_vte_terminal_get_status_line, METH_NOARGS },
    { "get_adjustment", (PyCFunction)_wrap_vte_terminal_get_adjustment, METH_NOARGS },
    { "get_char_width", (PyCFunction)_wrap_vte_terminal_get_char_width, METH_NOARGS },
    { "get_char_height", (PyCFunction)_wrap_vte_terminal_get_char_height, METH_NOARGS },
    { "get_char_descent", (PyCFunction)_wrap_vte_terminal_get_char_descent, METH_NOARGS },
    { "get_char_ascent", (PyCFunction)_wrap_vte_terminal_get_char_ascent, METH_NOARGS },
    { "get_row_count", (PyCFunction)_wrap_vte_terminal_get_row_count, METH_NOARGS },
    { "get_column_count", (PyCFunction)_wrap_vte_terminal_get_column_count, METH_NOARGS },
    { "get_window_title", (PyCFunction)_wrap_vte_terminal_get_window_title, METH_NOARGS },
    { "get_icon_title", (PyCFunction)_wrap_vte_terminal_get_icon_title, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyVteTerminal_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "Terminal",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,			/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    0,			/* tp_as_number */
    0,		/* tp_as_sequence */
    0,			/* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    0,					/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,			/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyVteTerminal_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_vte_terminal_new,		/* tp_init */
};



/* ----------- functions ----------- */

static PyObject *
_wrap_vte_terminal_get_type(PyObject *self)
{
    GType ret;

    ret = vte_terminal_get_type();
    return pyg_type_wrapper_new(ret);
}

static PyObject *
_wrap_vte_terminal_erase_binding_get_type(PyObject *self)
{
    GType ret;

    ret = vte_terminal_erase_binding_get_type();
    return pyg_type_wrapper_new(ret);
}

PyMethodDef pyvte_functions[] = {
    { "vte_terminal_get_type", (PyCFunction)_wrap_vte_terminal_get_type, METH_NOARGS },
    { "vte_terminal_erase_binding_get_type", (PyCFunction)_wrap_vte_terminal_erase_binding_get_type, METH_NOARGS },
    { NULL, NULL, 0 }
};


/* ----------- enums and flags ----------- */

void
pyvte_add_constants(PyObject *module, const gchar *strip_prefix)
{
    pyg_enum_add_constants(module, VTE_TYPE_TERMINAL_ERASE_BINDING, strip_prefix);
}

/* intialise stuff extension classes */
void
pyvte_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGtkMenuShell_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "MenuShell");
        _PyGtkWidget_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Widget");
    } else {
        Py_FatalError("could not import gtk");
        return;
    }
    if ((module = PyImport_ImportModule("gtk.gdk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGdkPixbuf_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Pixbuf");
    } else {
        Py_FatalError("could not import gtk.gdk");
        return;
    }


#line 1237 "vte.c"
    pygobject_register_class(d, "VteTerminal", VTE_TYPE_TERMINAL, &PyVteTerminal_Type, Py_BuildValue("(O)", &PyGtkWidget_Type));
}
