/*
Copyright (c) 1996-2002 Han The Thanh, <thanh@pdftex.org>

This file is part of pdfTeX.

pdfTeX is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

pdfTeX is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with pdfTeX; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

$Id: writeimg.c,v 1.1.1.2 2003-02-25 22:15:37 amb Exp $
*/

#include "ptexlib.h"
#include "image.h"
#include <kpathsea/c-auto.h>
#include <kpathsea/c-memstr.h>

#define bp2int(p)    round(p*(onehundredbp/100.0))

image_entry *image_ptr, *image_tab = 0;
integer image_max;

float epdf_width;
float epdf_height;
float epdf_orig_x;
float epdf_orig_y;
integer epdf_selected_page;
integer epdf_num_pages;
integer epdf_page_box;
integer epdf_always_use_pdf_pagebox;
void *epdf_doc;

static integer new_image_entry(void)
{
    entry_room(image, 1, 256);
    image_ptr->image_type = IMAGE_TYPE_NONE;
    image_ptr->color_type = 0;
    image_ptr->num_pages = 0;
    image_ptr->x_res = 0;
    image_ptr->y_res = 0;
    image_ptr->width = 0;
    image_ptr->height = 0;
    return image_ptr++ - image_tab;
}

integer imagecolor(integer img)
{
    return img_color(img);
}

integer imagewidth(integer img)
{
    return img_width(img);
}

integer imageheight(integer img)
{
    return img_height(img);
}

integer imagexres(integer img)
{
    return img_xres(img);
}

integer imageyres(integer img)
{
    return img_yres(img);
}

boolean ispdfimage(integer img)
{
    return img_type(img) == IMAGE_TYPE_PDF;
}

boolean checkimageb(integer procset)
{
    return procset & IMAGE_COLOR_B;
}

boolean checkimagec(integer procset)
{
    return procset & IMAGE_COLOR_C;
}

boolean checkimagei(integer procset)
{
    return procset & IMAGE_COLOR_I;
}

void updateimageprocset(integer img)
{
    pdfimageprocset |= img_color(img);
}

integer epdforigx(integer img)
{
    return pdf_ptr(img)->orig_x;
}

integer epdforigy(integer img)
{
    return pdf_ptr(img)->orig_y;
}

integer imagepages(integer img)
{
    return img_pages(img);
}

integer readimage(strnumber s, integer page_num, strnumber page_name,
                  integer pdfversion, integer pdfoptionalwaysusepdfpagebox)
{
    char *image_suffix;
    char *dest = 0;
    integer img = new_image_entry();
    /* need to allocate new string as makecstring's buffer is 
       already used by cur_file_name */
    if (page_name != 0)
      dest = xstrdup(makecstring(page_name));
    cur_file_name = makecstring(s);
    img_name(img) = kpse_find_file(cur_file_name, kpse_tex_format, true);
    if (img_name(img) == 0)
        pdftex_fail("cannot find image file");
    if ((image_suffix = strrchr(cur_file_name, '.')) == 0)
        pdftex_fail("cannot find image file name extension");
    if (strcasecmp(image_suffix, ".pdf") == 0) {
        img_type(img) = IMAGE_TYPE_PDF;
        pdf_ptr(img) = xtalloc(1, pdf_image_struct);
        pdf_ptr(img)->page_box = pdflastpdfboxspec;
        pdf_ptr(img)->always_use_pdfpagebox = pdfoptionalwaysusepdfpagebox;
	    page_num = read_pdf_info(img_name(img), dest, page_num, pdfversion, pdfoptionalwaysusepdfpagebox);
        img_width(img) = bp2int(epdf_width);
        img_height(img) = bp2int(epdf_height);
        img_pages(img) = epdf_num_pages;
        pdf_ptr(img)->orig_x = bp2int(epdf_orig_x);
        pdf_ptr(img)->orig_y = bp2int(epdf_orig_y);
        pdf_ptr(img)->selected_page = page_num;
        pdf_ptr(img)->doc = epdf_doc;
    }
    else if (strcasecmp(image_suffix, ".png") == 0) {
        img_type(img) = IMAGE_TYPE_PNG;
        img_pages(img) = 1;
        read_png_info(img);
    }
    else if (strcasecmp(image_suffix, ".jpg") == 0 ||
             strcasecmp(image_suffix, ".jpeg") == 0) {
        jpg_ptr(img) = xtalloc(1, JPG_IMAGE_INFO);
        img_type(img) = IMAGE_TYPE_JPG;
        img_pages(img) = 1;
        read_jpg_info(img);
    }
    else 
        pdftex_fail("unknown type of image");
    xfree(dest);
    cur_file_name = 0;
    return img;
}

void writeimage(integer img)
{
    cur_file_name = img_name(img);
    tex_printf(" <%s", img_name(img));
    switch (img_type(img)) {
    case IMAGE_TYPE_PNG:
        write_png(img);
        break;
    case IMAGE_TYPE_JPG:
        write_jpg(img);
        break;
    case IMAGE_TYPE_PDF:
        epdf_doc = pdf_ptr(img)->doc;
        epdf_selected_page = pdf_ptr(img)->selected_page;
        epdf_page_box = pdf_ptr(img)->page_box;
        epdf_always_use_pdf_pagebox = pdf_ptr(img)->always_use_pdfpagebox;
        write_epdf();
        break;
    default:
        pdftex_fail("unknown type of image");
    }
    tex_printf(">");
    cur_file_name = 0;
}

void deleteimage(integer img)
{
    switch (img_type(img)) {
    case IMAGE_TYPE_PDF:
        epdf_doc = pdf_ptr(img)->doc;
        epdf_delete();
        break;
    case IMAGE_TYPE_PNG:
        xfclose(png_ptr(img)->io_ptr, cur_file_name);
        png_destroy_read_struct(&(png_ptr(img)), &(png_info(img)), NULL);
        break;
    case IMAGE_TYPE_JPG:
        xfclose(jpg_ptr(img)->file, cur_file_name);
        break;
    default:
        pdftex_fail("unknown type of image");
    }
    xfree(img_name(img));
    return;
}

void img_free() 
{
    xfree(image_tab);
}
