#include "libpdftex.h"
#include "image.h"

image_entry *image_ptr, *image_tab = 0;
integer image_max;

int epdf_width;
int epdf_height;
int epdf_orig_x;
int epdf_orig_y;
void *epdf_doc;
void *epdf_xref;

integer new_image_entry()
{
    ENTRY_ROOM(image, 256);
    image_ptr->ref_count = 0;
    return image_ptr++ - image_tab;
}

void img_free() 
{
    image_entry *p;
    if (image_tab == 0)
        return;
    for (p = image_tab; p < image_ptr; p++) {
        if (p->ref_count > 0) {
            pdftex_warn("pending image (%i references)", p->ref_count);
            while (p->ref_count > 0)
                deleteimageref(p - image_tab);
        }
    }
    XFREE(image_tab);
}

void addimageref(integer img)
{
    IMG_REFCOUNT(img)++;
}

void deleteimageref(integer img)
{
    if (IMG_REFCOUNT(img) == 0) {
        switch (IMG_TYPE(img)) {
        case IMAGE_TYPE_PNG:
            xfclose(PNG_PTR(img)->io_ptr, filename);
            png_destroy_read_struct(&(PNG_PTR(img)), &(PNG_INFO(img)), NULL);
            break;
        case IMAGE_TYPE_JPG:
            xfclose(JPG_INFO(img)->file, filename);
            break;
#ifdef HAVE_TIFF
        case IMAGE_TYPE_TIFF:
            TIFFClose((TIFF *) TIFF_INFO(img)->file);
            break;
#endif
        case IMAGE_TYPE_PDF:
            epdf_xref = PDF_INFO(img)->xref;
            epdf_doc = PDF_INFO(img)->doc;
            epdf_delete();
            break;
        default:
            pdftex_fail("unknown type of image");
        }
        XFREE(IMG_NAME(img));
        return;
    }
    IMG_REFCOUNT(img)--;
}

integer imagewidth(integer img)
{
    switch (IMG_TYPE(img)) {
    case IMAGE_TYPE_PNG:
        return PNG_INFO(img)->width;
    case IMAGE_TYPE_JPG:
        return JPG_INFO(img)->width;
#ifdef HAVE_TIFF
    case IMAGE_TYPE_TIFF:
        return TIFF_INFO(img)->width;
#endif
    case IMAGE_TYPE_PDF:
        return PDF_INFO(img)->width;
    default:
        pdftex_fail("unknown type of image");
    }
}

integer imageheight(integer img)
{
    switch (IMG_TYPE(img)) {
    case IMAGE_TYPE_PNG:
        return PNG_INFO(img)->height;
    case IMAGE_TYPE_JPG:
        return JPG_INFO(img)->height;
#ifdef HAVE_TIFF
    case IMAGE_TYPE_TIFF:
        return TIFF_INFO(img)->height;
#endif
    case IMAGE_TYPE_PDF:
        return PDF_INFO(img)->height;
    default:
        pdftex_fail("unknown type of image");
    }
}

integer imagexres(integer img)
{
    switch (IMG_TYPE(img)) {
    case IMAGE_TYPE_PNG:
        if (PNG_INFO(img)->valid & PNG_INFO_pHYs)
            return 0.0254*png_get_x_pixels_per_meter(PNG_PTR(img), 
                                                     PNG_INFO(img)) + 0.5;
        return 0;
    case IMAGE_TYPE_JPG:
        return JPG_INFO(img)->x_res;
#ifdef HAVE_TIFF
    case IMAGE_TYPE_TIFF:
        return TIFF_INFO(img)->x_res;
#endif
    case IMAGE_TYPE_PDF:
        return 0;
    default:
        pdftex_fail("unknown type of image");
    }
}

integer imageyres(integer img)
{
    switch (IMG_TYPE(img)) {
    case IMAGE_TYPE_PNG:
        if (PNG_INFO(img)->valid & PNG_INFO_pHYs)
            return 0.0254*png_get_y_pixels_per_meter(PNG_PTR(img),
                                                     PNG_INFO(img)) + 0.5;
        return 0;
    case IMAGE_TYPE_JPG:
        return JPG_INFO(img)->y_res;
#ifdef HAVE_TIFF
    case IMAGE_TYPE_TIFF:
        return TIFF_INFO(img)->y_res;
#endif
    case IMAGE_TYPE_PDF:
        return 0;
    default:
        pdftex_fail("unknown type of image");
    }
}

boolean ispdfimage(integer img)
{
    return IMG_TYPE(img) == IMAGE_TYPE_PDF;
}

integer epdforigx(integer img)
{
    return  PDF_INFO(img)->orig_x;
}

integer epdforigy(integer img)
{
    return  PDF_INFO(img)->orig_y;
}

integer readimg()
{
    integer img = new_image_entry();
    filename = makecstring(makenamestring());
    flushstring();
    IMG_NAME(img) = kpse_find_file(filename, kpse_tex_ps_header_format, true);
    if (IMG_NAME(img) == 0)
        pdftex_fail("cannot open image file");
    if (strcasecmp(strend(filename) - 4, ".pdf") == 0) {
        PDF_INFO(img) = XTALLOC(1, pdf_image_struct);
        read_pdf_info(IMG_NAME(img));
        PDF_INFO(img)->width = epdf_width;
        PDF_INFO(img)->height = epdf_height;
        PDF_INFO(img)->orig_x = epdf_orig_x;
        PDF_INFO(img)->orig_y = epdf_orig_y;
        PDF_INFO(img)->xref = epdf_xref;
        PDF_INFO(img)->doc = epdf_doc;
        IMG_TYPE(img) = IMAGE_TYPE_PDF;
    }
    else if (strcasecmp(strend(filename) - 4, ".png") == 0) {
        read_png_info(img);
        IMG_TYPE(img) = IMAGE_TYPE_PNG;
    }
    else if (strcasecmp(strend(filename) - 4, ".jpg") == 0) {
        JPG_INFO(img) = XTALLOC(1, JPG_IMAGE_INFO);
        switch (read_jpg_info(img)) {
        case 0:
            break;
        case 4:
            pdftex_fail("unsupported type of compression");
        default: 
            pdftex_fail("reading JPG image failed");
        }
        IMG_TYPE(img) = IMAGE_TYPE_JPG;
    }
#ifdef HAVE_TIFF
    else if (strcasecmp(strend(filename) - 4, ".tif") == 0
	     || strcasecmp(strend(filename) - 5, ".tiff") == 0) {
        TIFF_INFO(img) = XTALLOC(1, TIFF_IMAGE_INFO);
        switch (read_tiff_info(img)) {
        case 0:
            break;
        default:
	    pdftex_fail("reading TIFF image failed");
        }
        IMG_TYPE(img) = IMAGE_TYPE_TIFF;
    }
#endif
    else pdftex_fail("unknown type of image");
    filename = 0;
    return img;
}

void writeimg(integer n, integer img)
{
    tex_printf(" <%s", IMG_NAME(img));
    switch (IMG_TYPE(img)) {
    case IMAGE_TYPE_PNG:
        write_png(n, img);
        break;
    case IMAGE_TYPE_JPG:
        write_jpg(n, img);
        break;
#ifdef HAVE_TIFF
    case IMAGE_TYPE_TIFF:
        write_tiff(n, img);
        break;
#endif
    case IMAGE_TYPE_PDF:
        epdf_xref = PDF_INFO(img)->xref;
        epdf_doc = PDF_INFO(img)->doc;
        write_epdf(n, img);
        break;
    default:
        pdftex_fail("unknown type of image");
    }
    tex_printf(">");
}
