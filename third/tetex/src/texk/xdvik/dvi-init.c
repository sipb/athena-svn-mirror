/*========================================================================*\

Copyright (c) 1990-2000  Paul Vojta

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
PAUL VOJTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

NOTE:
	xdvi is based on prior work, as noted in the modification history
	in xdvi.c.

\*========================================================================*/

#include "xdvi-config.h"
#include "kpathsea/c-fopen.h"
#include "kpathsea/c-stat.h"
#include "kpathsea/magstep.h"
#include "kpathsea/tex-glyph.h"
#include "dvi.h"
#include "string-utils.h"

#ifdef HTEX
#include "HTEscape.h"
#endif

#ifdef T1LIB
#include "t1lib.h"
#endif

#define	PK_PRE		247
#define	PK_ID		89
#define	PK_MAGIC	(PK_PRE << 8) + PK_ID
#define	GF_PRE		247
#define	GF_ID		131
#define	GF_MAGIC	(GF_PRE << 8) + GF_ID
#define	VF_PRE		247
#define	VF_ID_BYTE	202
#define	VF_MAGIC	(VF_PRE << 8) + VF_ID_BYTE

#define	dvi_oops(str)	(dvi_oops_msg = (str), longjmp(dvi_env, 1))

static ino_t dvi_inode = 0;	/* inode number in dvi prop. */
static struct stat fstatbuf;


/*
 * DVI preamble and postamble information.
 */
static char job_id[300];
static unsigned long numerator, denominator;

/*
 * Offset in DVI file of last page, set in read_postamble().
 */
static long last_page_offset;

#ifdef HTEX
extern int this_is_a_href;
#endif

/*
 *	free_vf_chain frees the vf_chain structure.
 */

static void
free_vf_chain(struct tn *tnp)
{
    while (tnp != NULL) {
	struct tn *tnp1 = tnp->next;
	free((char *)tnp);
	tnp = tnp1;
    }
}

/*
 *	delete glyph information in a font.
 */

static void
delete_glyphs(struct font *fontp)
{
    struct glyph *g;

    for (g = fontp->glyph; g != NULL && g <= fontp->glyph + fontp->maxchar; ++g) {
	if (g->bitmap2.bits) {
	    free(g->bitmap2.bits);
	    g->bitmap2.bits = NULL;
	}
#ifdef	GREY
	if (g->pixmap2) {
	    XDestroyImage(g->image2);
	    g->pixmap2 = NULL;
	    if (g->pixmap2_t != NULL) {
		free(g->pixmap2_t);
		g->pixmap2_t = NULL;
	    }
	}
#endif
    }
}

/*
 *	Release all shrunken bitmaps for all fonts.
 */

void
reset_fonts(void)
{
    struct font *f;

    for (f = font_head; f != NULL; f = f->next) {
	if ((f->flags & FONT_LOADED) && !(f->flags & FONT_VIRTUAL)) {
	    delete_glyphs(f);
	}
    }
}

/*
 * free up fonts no longer in use.
 */
static void
free_unused_fonts(void)
{
    struct font *fontp;
    struct font **fontpp;

    fontpp = &font_head;
    while ((fontp = *fontpp) != NULL) {
	if (fontp->flags & FONT_IN_USE)
	    fontpp = &fontp->next;
	else {
	    if (debug & DBG_PK)
		Printf("xdvi: Discarding font \"%s\" at %d dpi\n",
		       fontp->fontname, (int)(fontp->fsize + 0.5));
	    *fontpp = fontp->next;	/* remove from list */
	    free(fontp->fontname);
	    if (fontp->flags & FONT_LOADED) {
		if (fontp->file != NULL) {
		    Fclose(fontp->file);
		    ++n_files_left;
		}
		free(fontp->filename);
		if (fontp->flags & FONT_VIRTUAL) {
		    struct macro *m;

		    for (m = fontp->macro;
			 m <= fontp->macro + fontp->maxchar; ++m)
			    if (m->free_me) free((char *)m->pos);
		    free((char *)fontp->macro);
		    free((char *)fontp->vf_table);
		    free_vf_chain(fontp->vf_chain);
		}
		else {
		    delete_glyphs(fontp);
		    free((char *)fontp->glyph);
		    fontp->glyph = NULL;
		}
		free((char *)fontp);
		fontp = NULL;
	    }
	}
    }
}

void
free_fontlist(void)
{
    struct font *fontp;
    for (fontp = font_head; fontp != NULL; fontp = fontp->next) {
	fontp->flags &= ~FONT_IN_USE;
    }
    free_unused_fonts();
}

/*
 *	realloc_font allocates the font structure to contain (newsize + 1)
 *	characters.
 */

void
realloc_font(struct font *fontp, wide_ubyte newsize)
{
    struct glyph *glyph;

    glyph = fontp->glyph = xrealloc(fontp->glyph,
				    (unsigned int)(newsize + 1) * sizeof(struct glyph));
    if (newsize > fontp->maxchar)
	bzero((char *)(glyph + fontp->maxchar + 1),
	      (int)(newsize - fontp->maxchar) * sizeof(struct glyph));
    maxchar = fontp->maxchar = newsize;
}


/*
 *	realloc_virtual_font does the same thing for virtual fonts.
 */

void
realloc_virtual_font(struct font *fontp, wide_ubyte newsize)
{
    struct macro *macro;

    macro = fontp->macro = xrealloc(fontp->macro,
				    (unsigned int)(newsize + 1) * sizeof(struct macro));
    if (newsize > fontp->maxchar)
	bzero((char *)(macro + fontp->maxchar + 1),
	      (int)(newsize - fontp->maxchar) * sizeof(struct macro));
    maxchar = fontp->maxchar = newsize;
}


/*
 * load_font locates the t1 font or raster file and reads the index of
 * characters, plus whatever other preprocessing is done (depending on
 * the format).  Return False on error, True on success.
 */

Boolean
load_font(struct font *fontp)
{
    double fsize = fontp->fsize;
    int dpi = fsize + 0.5;
    char *font_found;
    int size_found;
    int magic;
    Boolean hushcs = hush_chk;

    fontp->glyph = NULL;
    fontp->flags |= FONT_LOADED;

#ifdef T1LIB
    fontp->file = font_open(fontp->fontname, &font_found,
			    fsize, &size_found, fontp->magstepval,
			    &fontp->filename, &fontp->t1id);
#else
    fontp->file = font_open(fontp->fontname, &font_found,
			    fsize, &size_found, fontp->magstepval,
			    &fontp->filename);
#endif /* T1LIB */

#ifdef T1LIB
    if (fontp->t1id >= 0 && resource.t1lib) {
	/* It's a type1 font */
	fontp->fsize = fsize;	/* It comes in all sizes */
	fontp->timestamp = ++current_timestamp;
	fontp->maxchar = maxchar = 255;
	fontp->set_char_p = set_t1_char;
	/* read_T1_char is a dummy */
	fontp->read_char = read_T1_char;
	fontp->glyph = xmalloc (256 * sizeof (struct glyph));
	bzero ((char *) fontp->glyph, 256 * sizeof (struct glyph));
	if (font_found != NULL) {
	    print_statusline(STATUS_MEDIUM,
			     "Can't find font %s; using Type1 version of %s instead.",
			     fontp->fontname, font_found);
	    force_statusline_update();
	    free(fontp->fontname);
	    fontp->fontname = font_found; /* this has been allocated by font_open */
	}
	return True;
    }
#endif /* T1LIB */
    /* when not using T1lib, fontp->file == NULL means total failure */
    if (fontp->file == NULL) {
	print_statusline(STATUS_MEDIUM,
			 "Can't find font %s.%dpk",
			 fontp->fontname, dpi);
	return False;
    }

    --n_files_left;
    if (font_found != NULL) { /* fallback used */
	print_statusline(STATUS_MEDIUM,
			 "Can't find pixel font %s; using %s instead at %d dpi.",
			 fontp->fontname, font_found, dpi);
	force_statusline_update();
	free(fontp->fontname);
	fontp->fontname = font_found; /* this has been allocated by font_open */
	hushcs = True;
    }
    else if (!kpse_bitmap_tolerance((double)size_found, fsize)) { /* a different size used */
	print_statusline(STATUS_MEDIUM,
			 "Can't find pixel font %s at %d dpi; using %d dpi instead.",
			 fontp->fontname, dpi, size_found);
	force_statusline_update();
    }
    /* PK version of some font found */
    fontp->fsize = size_found;
    fontp->timestamp = ++current_timestamp;
    fontp->maxchar = maxchar = 255;
    fontp->set_char_p = set_char;
    magic = two(fontp->file);
    if (magic == PK_MAGIC)
	read_PK_index(fontp, WIDENINT hushcs);
    else
#ifdef USE_GF
    if (magic == GF_MAGIC)
	read_GF_index(fontp, WIDENINT hushcs);
    else
#endif
#ifdef Omega
    if (magic == VF_MAGIC)
	maxchar = read_VF_index(fontp, WIDENINT hushcs);
#else
    if (magic == VF_MAGIC)
	read_VF_index(fontp, WIDENINT hushcs);
#endif
    else
	oops("Cannot recognize format for font file %s", fontp->filename);

    if (fontp->flags & FONT_VIRTUAL) {
#ifdef Omega
#else
	while (maxchar > 0 && fontp->macro[maxchar].pos == NULL)
	    --maxchar;
	if (maxchar < 255)
	    realloc_virtual_font(fontp, WIDENINT maxchar);
#endif
    }
    else {
	while (maxchar > 0 && fontp->glyph[maxchar].addr == 0)
	    --maxchar;
	if (maxchar < 255)
	    realloc_font(fontp, WIDENINT maxchar);
    }
    return True;
}


/*
 *	MAGSTEPVALUE - If the given magnification is close to a \magstep
 *	or a \magstephalf, then return twice the number of \magsteps.
 *	Otherwise return NOMAGSTP.
 */

#define	NOMAGSTP (-29999)
#define	NOBUILD	29999

static int
magstepvalue(float *mag)
{
    int m = 0;
    double fmag = *mag;
    double xmag = pixels_per_inch;
    float margin = fmag * 0.002;

    if (fmag < pixels_per_inch)
	for (;;) {
	    if (xmag - fmag < margin && -(xmag - fmag) < margin) {
		*mag = xmag;
		return m;
	    }
	    if (xmag < fmag)
		break;
	    xmag *= 0.9128709292;
	    --m;
	}
    else
	for (;;) {
	    if (xmag - fmag < margin && -(xmag - fmag) < margin) {
		*mag = xmag;
		return m;
	    }
	    if (xmag > fmag)
		break;
	    xmag *= 1.095445115;
	    ++m;
	}
    return NOMAGSTP;
}

/*
 *	reuse_font recursively sets the flags for font structures being reused.
 */

static void
reuse_font(struct font *fontp)
{
    struct font **fp;
    struct tn *tnp;

    if (fontp->flags & FONT_IN_USE)
	return;

    fontp->flags |= FONT_IN_USE;
    if (list_fonts)
	Printf("xdvi: (reusing) %s at %d dpi\n", fontp->fontname,
	       (int)(fontp->fsize + 0.5));
    if (fontp->flags & FONT_VIRTUAL) {
	for (fp = fontp->vf_table; fp < fontp->vf_table + VFTABLELEN; ++fp)
	    if (*fp != NULL)
		reuse_font(*fp);
	for (tnp = fontp->vf_chain; tnp != NULL; tnp = tnp->next)
	    reuse_font(tnp->fontp);
    }
}


/*
 *      define_font reads the rest of the fntdef command and then reads in
 *      the specified pixel file, adding it to the global linked-list holding
 *      all of the fonts used in the job.
 */
struct font *
define_font(FILE *file,
	    wide_ubyte cmnd,
	    struct font *vfparent,	/* vf parent of this font, or NULL */
	    struct font **tntable,	/* table for low TeXnumbers */
	    unsigned int tn_table_len,	/* length of table for TeXnumbers */
	    struct tn **tn_headpp,	/* addr of head of list of TeXnumbers */
	    Boolean *not_found_flag)	/* signal that font hasn't been found */
{
    unsigned int TeXnumber;
    struct font *fontp;
    float fsize;
    double scale_dimconv;
    long checksum;
    int scale, orig_scale;
    int design;
    int magstepval;
    int len;
    char *fontname;
    int size;

    TeXnumber = num(file, (int)cmnd - FNTDEF1 + 1);
    checksum = four(file);
    scale = orig_scale = four(file);
    design = four(file);
    len = one(file);
    len += one(file);	/* sequence point in the middle */
    fontname = xmalloc((unsigned)len + 1);
    Fread(fontname, sizeof(char), len, file);
    fontname[len] = '\0';
    if (debug & DBG_PK)
	Printf("xdvi: Define font \"%s\" scale=%d design=%d number=%d\n",
	       fontname, scale, design, TeXnumber);
    if (vfparent == NULL) {
	fsize = 0.001 * scale / design * magnification * pixels_per_inch;
	scale_dimconv = dimconv;
    }
    else {
	/*
	 * The scaled size is given in units of vfparent->scale * 2 ** -20
	 * SPELL units, so we convert it into SPELL units by multiplying by
	 *         vfparent->dimconv.
	 * The design size is given in units of 2 ** -20 pt, so we convert
	 * into SPELL units by multiplying by
	 *         (pixels_per_inch * 2**16) / (72.27 * 2**20).
	 */
	fsize = (72.27 * (1 << 4)) * vfparent->dimconv * scale / design;
	scale_dimconv = vfparent->dimconv;
	scale = vfparent->scale * ((orig_scale * scale_dimconv / (1 << 20)) / vfparent->dimconv); /* Inherit the scale from the virtual parent */
    }
    magstepval = magstepvalue(&fsize);
    size = fsize + 0.5;
    /*
     * reuse font if possible
     */
    for (fontp = font_head;; fontp = fontp->next) {
	if (fontp == NULL) {	/* if font doesn't exist yet */
	    if (list_fonts)
		Printf("xdvi: %s at %d dpi\n", fontname, (int)(fsize + 0.5));
	    fontp = xmalloc((unsigned)sizeof(struct font));
	    fontp->fontname = fontname;
	    fontp->fsize = fsize;
	    fontp->magstepval = magstepval;
	    fontp->file = NULL;	/* needed if it's a virtual font */
	    fontp->checksum = checksum;
	    fontp->flags = FONT_IN_USE;
	    fontp->dimconv =  orig_scale * scale_dimconv / (1 << 20);
	    fontp->set_char_p = load_n_set_char;
	    fontp->scale = scale;
	    if (vfparent == NULL) {
		if (!load_font(fontp)) {
		    *not_found_flag = True;
		}
	    }

	    fontp->next = font_head;
	    font_head = fontp;
	    break;
	}
	if (strcmp(fontname, fontp->fontname) == 0
	    && size == (int)(fontp->fsize + 0.5)) {
	    /* if font already in use */
	    reuse_font(fontp);
	    free(fontname);
	    break;
	}
    }
    if (TeXnumber < tn_table_len)
	tntable[TeXnumber] = fontp;
    else {
	struct tn *tnp;
	tnp = xmalloc((unsigned)sizeof(struct tn));
	tnp->next = *tn_headpp;
	*tn_headpp = tnp;
	tnp->TeXnumber = TeXnumber;
	tnp->fontp = fontp;
    }
    return fontp;
}


/*
 *      process_preamble reads the information in the preamble and stores
 *      it into global variables for later use.
 */
static void
process_preamble(void)
{
    ubyte k;

    if (dvi_file == NULL) {
	return;
    }
    if (one(dvi_file) != PRE)
	dvi_oops("Not a DVI file.");
    if (one(dvi_file) != 2)
	dvi_oops("Wrong version of DVI output for this program");
    numerator = four(dvi_file);
    denominator = four(dvi_file);
    magnification = four(dvi_file);
    dimconv = (((double)numerator * magnification)
	       / ((double)denominator * 1000.));
    dimconv = dimconv * (((long)pixels_per_inch) << 16) / 254000;
    tpic_conv = pixels_per_inch * magnification / 1000000.0;
    k = one(dvi_file);
    Fread(job_id, sizeof(char), (int)k, dvi_file);
    job_id[k] = '\0';
}

/*
 *      find_postamble locates the beginning of the postamble
 *	and leaves the file ready to start reading at that location.
 */
#define	TMPSIZ	516	/* 4 trailer bytes + 512 junk bytes allowed */
static void
find_postamble(void)
{
    long pos;
    ubyte temp[TMPSIZ];
    ubyte *p;
    ubyte *p1;
    ubyte byte;

    Fseek(dvi_file, (long)0, 2);
    pos = ftell(dvi_file) - TMPSIZ;
    if (pos < 0)
	pos = 0;
    Fseek(dvi_file, pos, 0);

    p = temp + fread((char *)temp, sizeof(char), TMPSIZ, dvi_file);
    for (;;) {
	p1 = p;
	while (p1 > temp && *(--p1) != TRAILER)
	    ; /* do nothing */
	p = p1;
	while (p > temp && *(--p) == TRAILER)
	    ; /* do nothing */
	if (p <= p1 - 4) {
	    break;	/* found 4 TRAILER bytes */
	}
	if (p <= temp) {
	    dvi_oops("DVI file corrupted");
	}
    }
    pos += p - temp;
    byte = *p;
    while (byte == TRAILER) {
	Fseek(dvi_file, --pos, 0);
	byte = one(dvi_file);
    }
    if (byte != 2)
	dvi_oops("Wrong version of DVI output for this program");
    Fseek(dvi_file, pos - 4, 0);
    Fseek(dvi_file, sfour(dvi_file), 0);
    return;
}


/*
 *      read_postamble reads the information in the postamble,
 *	storing it into global variables.
 *      It also takes care of reading in all of the pixel files for the fonts
 *      used in the job.
 */
static void
read_postamble(void)
{
    Boolean font_not_found = False;

    ubyte cmnd;

    if (one(dvi_file) != POST)
	dvi_oops("Postamble doesn't begin with POST");
    last_page_offset = four(dvi_file);
    if (numerator != four(dvi_file)
	|| denominator != four(dvi_file)
	|| magnification != four(dvi_file))
	dvi_oops("Postamble doesn't match preamble");
    /* read largest box height and width */
    unshrunk_page_h = (spell_conv(sfour(dvi_file)) >> 16) + offset_y;
    if (unshrunk_page_h < unshrunk_paper_h)
	unshrunk_page_h = unshrunk_paper_h;
    unshrunk_page_w = (spell_conv(sfour(dvi_file)) >> 16) + offset_x;
    if (unshrunk_page_w < unshrunk_paper_w)
	unshrunk_page_w = unshrunk_paper_w;
    (void)two(dvi_file);	/* max stack size */
    total_pages = two(dvi_file);

    while ((cmnd = one(dvi_file)) >= FNTDEF1 && cmnd <= FNTDEF4) {
	(void)define_font(dvi_file, cmnd, (struct font *)NULL,
			  tn_table, TNTABLELEN, &tn_head, &font_not_found);
	if (font_not_found)
	    dvi_oops("Not all pixel files were found");
    }
    if (cmnd != POSTPOST)
	dvi_oops("Non-fntdef command found in postamble");
    free_unused_fonts();
}


static void
prepare_pages(void)
{
    int i;

    page_offset = xmalloc((unsigned)total_pages * sizeof(long));
    i = total_pages;
    page_offset[--i] = last_page_offset;
    Fseek(dvi_file, last_page_offset, 0);
    /*
     * Follow back pointers through pages in the DVI file,
     * storing the offsets in the page_offset table.
     */
    while (i > 0) {
	Fseek(dvi_file, (long)(1 + 4 + (9 * 4)), 1);
	Fseek(dvi_file, page_offset[--i] = four(dvi_file), 0);
    }
}

void
init_page(void)
{
    page_w = ROUNDUP(unshrunk_page_w, mane.shrinkfactor) + 2;
    page_h = ROUNDUP(unshrunk_page_h, mane.shrinkfactor) + 2;
}

#ifndef	S_ISDIR
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif

/*
 *	internal_open_dvi does the real opening of the dvi file, and sets
 *	dvi_time.  It returns True on success.
 */

static Boolean
internal_open_dvi(_Xconst char *path)
{
    dvi_file = xdvi_xfopen(path, OPEN_MODE);
    if (dvi_file == NULL)
	return False;

    if (fstat(fileno(dvi_file), &fstatbuf) != 0 || S_ISDIR(fstatbuf.st_mode)) {	/* if it's a directory */
	fclose(dvi_file);
	dvi_file = NULL;
	fstatbuf.st_mtime = 0; /* hack to workaround file opening bug */
	return False;
    }
    /* If file has zero size, something has gone wrong with downloading
       it, and the user should already have been warned about that;
       just return in this case.
       TODO: can it still happen that we try to load such a file as .dvi
       file? (Will exit  with `draw_part: unknown op-code xyz' or some such).
       In this case, it would be better to look at the preamble before
       entering the drawing loop.
    */
    if (fstatbuf.st_size == 0) {
	fclose(dvi_file);
	dvi_file = NULL;
	return False;
    }
    
    dvi_time = fstatbuf.st_mtime;

    return True;
}

/**
 **	open_dvi_file opens the dvi file (if present).
 **	Upon failure, dvi_file == NULL.
 **/

void
open_dvi_file(void)
{
    char *new_name;
    unsigned int n;

    n = strlen(dvi_name);

#ifdef HTEX    
    if (!this_is_a_href) {
#endif      
	/*
	 * Try foo.dvi before foo, in case there's an executable foo with
	 * documentation foo.tex.  Unless it already ends with ".dvi".
	 */
	if (n < sizeof(".dvi")
	    || strcmp(dvi_name + n - sizeof(".dvi") + 1, ".dvi") != 0) {
	    new_name = xmalloc(n + sizeof(".dvi"));
	    memcpy(new_name, dvi_name, n);
	    memcpy(new_name + n, ".dvi", sizeof(".dvi"));
	    if (internal_open_dvi(new_name)) {	/* if found */
		dvi_name = new_name;
		return;
	    }
	    free(new_name);
	}
#ifdef HTEX    
    }
#endif
    /* Then try `foo', in case the user likes DVI files without `.dvi'. */
    (void)internal_open_dvi(dvi_name);
}

/**
 **	form_dvi_property forms the property used to exhibit the dvi file name
 **	used as a window property (used for source specials).
 **/

void
form_dvi_property(void)
{
    size_t len;
    unsigned long ino;
    int i;

    if (dvi_file == NULL)
	return;

    if (dvi_property != NULL)
	free(dvi_property);

    len = strlen(global_dvi_name);

    dvi_property_length = len + 8;
    dvi_property = xmalloc(dvi_property_length);

    /* Do it this way because different copies of xdvi might be running on
       machines with different endiannesses.  */
    ino = dvi_inode = fstatbuf.st_ino;
    for (i = 0; i < 8; ++i) {
	dvi_property[i] = ino;
	ino >>= 8;
    }
    memcpy(dvi_property + 8, global_dvi_name, len);
}

/*
 *	internal_init_dvi is the main subroutine for reading the startup
 *	information from the dvi file.
 */

static void
internal_init_dvi(void)
{
    process_preamble();
    find_postamble();
    read_postamble();
    prepare_pages();
    init_page();
    if (current_page >= total_pages)
	current_page = total_pages - 1;
    warn_spec_now = warn_spec;
#if	PS
    ps_newdoc();
#endif
    source_fwd_box_page = -1;
    return;
}

#ifdef SELFILE
/* Allow the user to choose a new dvi file, by popping up a dialog box
   which allows the graphical selection of the correct filename,
   maybe we should only allow files ending in .dvi to be selected.  */

FILE *
select_filename(int open, int move_home)
{
    extern FILE *XsraSelFile();
    FILE *dummy_file;
    static char *dummy_name = NULL;

    dummy_file = XsraSelFile(top_level, "Select a dvi file: ",
			     "Ok", "Cancel",
			     "Can't open file: ", NULL,
			     OPEN_MODE, NULL, &dummy_name);

    if (dummy_file != NULL) {
	/* we may not want the file they returned... */
	if (!open) {
	    fclose(dummy_file);
	}

	if (fstat(fileno(dummy_file), &fstatbuf) == 0 && !S_ISDIR(fstatbuf.st_mode)) {	/* if it's a directory */
	    dvi_time = fstatbuf.st_mtime + 1;
	}

	/* FIXME: re-design the logic of current function so that 
	   internal_open_dvi can be used! */

	/* The name is what we really want, so use it, but turn it into an URL */
	free(dvi_name);
	if (global_dvi_name)
	    free(global_dvi_name);

	/* save normalized filename in global_dvi_name, as in xdvi.c */
	global_dvi_name = normalize_and_expand_filename(dummy_name);
	    
#ifdef HTEX
	dummy_name = HTEscape(dummy_name, URL_PATH);
#endif

	dvi_name = xmalloc((unsigned)strlen(dummy_name) + 6);
	strcat(strcpy(dvi_name,"file:"), dummy_name);

	free(dummy_name);
	
	current_page = 0;	/* go to start of new dvi file */
	free_fontlist();
	
	if (move_home)
	    home(False);	/* Move to home position on new first page */
    }
    else if (open) {	/* User cancelled, so open old file */
	if (!internal_open_dvi(dvi_name)) {
	    dvi_time = fstatbuf.st_mtime + 1;
	    dvi_oops("Could not open old file");
	}
    }

    return dummy_file;
}
#endif /* SELFILE */

/**
 **	init_dvi_file initializes the dvi file.  Called only from main().
 **/

void
init_dvi_file(void)
{
    if (setjmp(dvi_env))
	oops(dvi_oops_msg);

    internal_init_dvi();
}

/**
 **	set_dvi_property sets the appropriate property for the main
 **	window (used in source special handoff).
 **/

void
set_dvi_property(void)
{
    XChangeProperty(DISP, XtWindow(top_level), ATOM_DVI_FILE, ATOM_DVI_FILE,
		    8, PropModeReplace, dvi_property, dvi_property_length);
}

/**
 **	Check for changes in dvi file.
 **	return 0 if file hasn't changed, 1 if it can be reloaded, 2 if it is corrupted.
 **/

int
dvi_file_changed(void)
{
    struct font *fontp;
    unsigned int old_page_w, old_page_h;

    if (dvi_file == NULL
	|| fstat(fileno(dvi_file), &fstatbuf) != 0
	|| fstatbuf.st_mtime != dvi_time) {
	if (dvi_file) {
	    Fclose(dvi_file);
	    if (list_fonts)
		Putchar('\n');
	    dvi_file = NULL;
	}
	if (page_offset != NULL) {
	    free(page_offset);
	    page_offset = NULL;
	}
	bzero(tn_table, sizeof tn_table);
	free_vf_chain(tn_head);
	tn_head = NULL;
	for (fontp = font_head; fontp != NULL; fontp = fontp->next) {
	    fontp->flags &= ~FONT_IN_USE;
	}
	if (!setjmp(dvi_env)) {
	    old_page_w = page_w;
	    old_page_h = page_h;
#ifdef SELFILE
	    if ((dvi_time > fstatbuf.st_mtime) &&	/* choose a new file */
		(dvi_file = select_filename(True, True)) == NULL) {
		dvi_oops("Cannot open new dvi file.");
	    }
	    else
#endif /* SELFILE */
		if (!internal_open_dvi(dvi_name)) {
		    dvi_oops("Cannot reopen dvi file.");
		}
	    internal_init_dvi();
	    set_icon_and_title(dvi_name, NULL, NULL, 1);

	    if (fstatbuf.st_ino != dvi_inode) {
		form_dvi_property();
		set_dvi_property();
	    }
	    if (page_w != old_page_w || page_h != old_page_h) {
		reconfig();
	    }
#ifdef HTEX
	    htex_reinit();
#endif /* HTEX */
	    redraw_page();
	    print_statusline(STATUS_MEDIUM, "DVI file reloaded");
	    return 1;
	}
	else {
	    print_statusline(STATUS_MEDIUM, "DVI file corrupted");
	    XClearWindow(DISP, mane.win);
	    return 2;
	}
    }
    return 0;
}
