#include "libpdftex.h"

key_entry font_keys[FONT_KEYS_NUM] = {
    {"Ascent",       "Ascender",     {0}, false},
    {"CapHeight",    "CapHeight",    {0}, false},
    {"Descent",      "Descender",    {0}, false},
    {"FontName",     "FontName",     {0}, false},
    {"ItalicAngle",  "ItalicAngle",  {0}, false},
    {"StemV",        "StdVW",        {0}, false},
    {"XHeight",      "XHeight",      {0}, false},
    {"FontBBox",     "FontBBox",     {0}, false},
    {"",             "",             {0}, false},
    {"",             "",             {0}, false},
    {"",             "",             {0}, false}
};

void print_key(integer code, integer v)
{
    pdf_printf("/%s ", font_keys[code].pdfname);
    if (!font_keys[code].valid) {
        pdf_printf("%li",
                   (code == ITALIC_ANGLE_CODE) ? (long int)v :
                   (long int)dividescaled(v, pdffontsize[tex_font], 3));
    }
    else 
        pdf_printf("%li", (long int)font_keys[code].value.i);
    pdf_printf("\n");
}

integer getitalicangle(internalfontnumber f)
{
    return -atan(getslant(f)/65536.0)*(180/M_PI);
}

integer getstemv(internalfontnumber f)
{
    return getquad(f)/10;
}

void getbbox(internalfontnumber f)
{
    font_keys[FONTBBOX1_CODE].value.i = 0;
    font_keys[FONTBBOX2_CODE].value.i = 
        dividescaled(-getchardepth(f, 'g'), pdffontsize[f], 3);
    font_keys[FONTBBOX3_CODE].value.i =
        dividescaled(getquad(f), pdffontsize[f], 3);
    font_keys[FONTBBOX4_CODE].value.i =
        dividescaled(getcharheight(f, 'H'), pdffontsize[f], 3);
}

static char *subset_prefix(internalfontnumber f)
{
    integer k, l = 0;
    char buf[7], *p = buf;
    for (k = 0; k < MAX_CHAR_NUM; k++)
        if (pdfischarused(f, k))
            l = (l << 1) + k;
    if (l < 0)
        l = -l - 1;
    for (k = 0; k < 6; k++) {
        *p++ = 'A' + l%('Z' - 'A');
        l /= 'Z' - 'A';
    }
    *p = 0;
    return xstrdup(buf);
}

void dopdffont(integer objnum, internalfontnumber f)
{
    int k, e = 0;
    char *p, *r, buf[1024];
    tex_font = f;
    filename = 0;
    if (pdffontmap[f] == -1) {
        if (pdfexpandfont[f] == 0)
            pdftex_fail("pdffontmap[%i] not initialized", (int)f);
        else if (pdfexpandfont[f] != 0) {
            strcpy(buf, makecstring(fontname[f]));
            p = strend(buf);
            for (r = p - 1; r > buf && isdigit(*r); r--);
            if (r == buf || r == p - 1 || (*r != '+' && *r != '-'))
                pdftex_fail("invalid name of expanded font (%s)", buf);
            *r = 0;
            pdffontmap[f] = fmlookup(maketexstring(buf));
            flushstring();
        }
    }
    if (pdffontmap[f] >= 0)
        fm_cur = fm_tab + pdffontmap[f];
    else
        fm_cur = 0;
    if (fm_cur == 0 || (fm_cur->base_name == 0 && fm_cur->ff_name == 0)) {
        writet3(objnum, f);
        return;
    }
    if (is_subsetted())
        fm_cur->prefix = subset_prefix(f);
    if (is_reencoded())
        e = enc_objnum(fm_cur->encoding);
    if (pdfexpandfont[f] != 0 && is_included())
        fm_cur = fm_ext_entry(f);
    pdfbegindict(objnum);
    pdf_printf("/Type /Font\n");
    if (pdfexpandfont[f] != 0)
        pdf_printf("%% expand ratio = %i\n", (int)pdfexpandfont[f]);
    pdf_printf("/Subtype /%s\n",
               is_truetype() ? "TrueType" : "Type1");
    if (is_reencoded() && !is_truetype())
        pdf_printf("/Encoding %li 0 R\n", (long int)e);
    if (is_basefont()) {
        pdf_printf("/BaseFont /%s\n>> endobj\n", fm_cur->base_name);
        return;
    }
    pdf_printf("/FirstChar %li\n/LastChar %li\n/Widths %li 0 R\n",
               (long int)fontbc[tex_font], (long int)fontec[tex_font]
, (long int)(objptr + 1));
/*
 *    if font is included then
 *        |obj_ptr + 1| is the width array,
 *        |obj_ptr + 2| is the font file stream,
 *        if font is TrueType then
 *            |obj_ptr + 3| is the font file length
 *            |obj_ptr + 4| is the font base name,
 *            |obj_ptr + 5| is the font descriptor,
 *        else
 *            |obj_ptr + 3..obj_ptr + 6| are
 *                the lengths of sections in font file,
 *            |obj_ptr + 7| is the font base name,
 *            |obj_ptr + 8| is the font descriptor,
 *    else
 *         |obj_ptr + 1| is the width array,
 *         |obj_ptr + 2| is the font base name,
 *         if font file is present then
 *             |obj_ptr + 3| is the font descriptor,
 */
 
    if (is_included()) {
        if (is_truetype())
            pdf_printf("/BaseFont %li 0 R\n/FontDescriptor %li 0 R\n",
                       (long int)(objptr + 4), (long int)(objptr + 5));
        else
            pdf_printf("/BaseFont %li 0 R\n/FontDescriptor %li 0 R\n",
                       (long int)(objptr + 7), (long int)(objptr + 8));
    }
    else if (!is_noparsing())
        pdf_printf("/BaseFont %li 0 R\n/FontDescriptor %li 0 R\n",
                   (long int)(objptr + 2), (long int)(objptr + 3));
	else
        pdf_printf("/BaseFont /%s\n", fm_cur->base_name);
    pdf_printf(">> endobj\n");
    pdfnewobj(0, 0); /* chars width array */
    pdf_printf("[ ");
    for (k = fontbc[tex_font]; k <= fontec[tex_font]; k++)
        pdf_printf("%li ",
                    (long int)dividescaled(getcharwidth(tex_font, k),
                                           pdffontsize[tex_font], 3));
    pdf_printf("]\nendobj\n");
    if (is_noparsing())
        return;
    if (is_included()) {
        pdfnewdict(0, 0); /* font file stream */
        if (!is_truetype())
            pdf_printf("/Length %li 0 R\n/Length1 %li 0 R\n/Length2 %li 0 R\n/Length3 %li 0 R\n",
                       (long int)(objptr + 1), (long int)(objptr + 2),
                       (long int)(objptr + 3), (long int)(objptr + 4));
        else 
            pdf_printf("/Length %li 0 R\n/Length1 %li 0 R\n",
                       (long int)(objptr + 1), (long int)(objptr + 1));
        pdf_printf(">>\nstream\n");
    }
    for (k = 0; k < FONT_KEYS_NUM; k++)
        font_keys[k].valid = false;
    font_file_not_found = false;
    cur_glyph_names = NULL;
    if (is_truetype())
        writettf();
    else
        writet1();
    if (is_included() && !font_file_not_found) {
        if (!is_truetype()) {
            pdf_printf("endstream\nendobj\n");
            pdfnewobj(0, 0);
            pdf_printf("%li\nendobj\n",
                       (long int)(t1_length1 + t1_length2 + t1_length3));
            pdfnewobj(0, 0);
            pdf_printf("%li\nendobj\n", (long int)t1_length1);
            pdfnewobj(0, 0);
            pdf_printf("%li\nendobj\n", (long int)t1_length2);
            pdfnewobj(0, 0);
            pdf_printf("%li\nendobj\n", (long int)t1_length3);
        }
        else {
            pdf_printf("endstream\nendobj\n");
            pdfnewobj(0, 0);
            pdf_printf("%li\nendobj\n", (long int)ttf_length);
        }
    }
    if (!font_keys[FONTNAME_CODE].valid)
        pdftex_warn("cannot read key `%s' from font file, used value from map file",
             font_keys[FONTNAME_CODE].pdfname);
/* suppress basename mismatch; seems to be useless */
/*
    else if (fm_cur->base_name != 0 && fm_cur->extend == 0 && fm_cur->slant == 0 &&
             strcmp(font_keys[FONTNAME_CODE].value.s, fm_cur->base_name))
        pdftex_warn("Base font name mismatch: `%s' (in font file) and  `%s' (in map file)!\nThe name specified in map file was ignored"
, font_keys[FONTNAME_CODE].value.s 
, fm_cur->base_name);
*/
    pdfnewobj(0, 0); /* font base name */
    pdf_printf("/");
    if (fm_cur->prefix)
        pdf_printf("%s+", fm_cur->prefix);
    if (font_keys[FONTNAME_CODE].valid) {
        pdf_printf("%s", font_keys[FONTNAME_CODE].value.s);
        XFREE(font_keys[FONTNAME_CODE].value.s);
    }
    else if (fm_cur->base_name != 0)
        pdf_printf("%s", fm_cur->base_name);
    else
        pdf_printf("unknown!");
    if (pdfexpandfont[f] != 0 && !is_included())
        pdf_printf("%+li", pdfexpandfont[f]);
    pdf_printf("\nendobj\n");
    pdfnewdict(0, 0); /* font descriptor */
    print_key(ASCENT_CODE, getcharheight(tex_font, 'h'));
    print_key(CAPHEIGHT_CODE, getcharheight(tex_font, 'H'));
    print_key(DESCENT_CODE, -getchardepth(tex_font, 'q'));
    pdf_printf("/FontName %li 0 R\n", (long int)(objptr - 1));
    print_key(ITALIC_ANGLE_CODE, getitalicangle(tex_font));
    print_key(STEMV_CODE, getstemv(tex_font));
    print_key(XHEIGHT_CODE, getxheight(tex_font));
    if (!font_keys[FONTBBOX1_CODE].valid) {
        getbbox(tex_font);
    }
    pdf_printf("/%s [ %li %li %li %li ]\n",
               font_keys[FONTBBOX1_CODE].pdfname,
               (long int)font_keys[FONTBBOX1_CODE].value.i,
               (long int)font_keys[FONTBBOX2_CODE].value.i,
               (long int)font_keys[FONTBBOX3_CODE].value.i,
               (long int)font_keys[FONTBBOX4_CODE].value.i);
    pdf_printf("/Flags %li\n", (long int)fm_cur->flags);
    if (is_subsetted() && cur_glyph_names != NULL) {
        pdf_printf("/CharSet (");
        for (k = 0; k < MAX_CHAR_NUM; k++)
            if (pdfischarused(tex_font, k) && cur_glyph_names[k] != notdef)
                pdf_printf("/%s", cur_glyph_names[k]);
        pdf_printf(")\n");
        if (cur_glyph_names == builtin_glyph_names)
            for (k = 0; k < MAX_CHAR_NUM; k++)
                if (cur_glyph_names[k] != notdef)
                    XFREE(cur_glyph_names[k]);
    }
    if (is_included() && !font_file_not_found) {
        if (is_truetype())
            pdf_printf("/FontFile2 %li 0 R\n", (long int)(objptr - 3));
        else
            pdf_printf("/FontFile %li 0 R\n", (long int)(objptr - 6));
    }
    pdf_printf(">> endobj\n");
}
