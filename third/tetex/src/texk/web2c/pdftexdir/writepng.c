#include "libpdftex.h"
#include "image.h"

integer read_png_info(integer img)
{
    FILE *png_file = xfopen(IMG_NAME(img), FOPEN_RBIN_MODE);
    if ((PNG_PTR(img) = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
        NULL, NULL, NULL)) == NULL)
        pdftex_fail("png_create_read_struct() failed");
    if ((PNG_INFO(img) = png_create_info_struct(PNG_PTR(img))) == NULL)
        pdftex_fail("png_create_info_struct() failed");
    if (setjmp(PNG_PTR(img)->jmpbuf))
        pdftex_fail("setjmp() failed");
    png_init_io(PNG_PTR(img), png_file);
    png_read_info(PNG_PTR(img), PNG_INFO(img));
    if (PNG_INFO(img)->color_type & PNG_COLOR_MASK_ALPHA) {
/*

   Before pdftex-0.12o-1 we failed when touching png with alpha channel, now we
   strip this channel. 09/06/1998 Pavel Janík ml.

*/
        png_set_strip_alpha(PNG_PTR(img));
        pdftex_warn("can't handle alpha channel correctly, stripped (please report bugs to pdftex@tug.org)");
    }
    if (PNG_INFO(img)->interlace_type != 0) {
/*

   Before pdftex-0.12o-1 we failed when touching interlaced png, now it's ok.
   09/06/1998 Pavel Janík ml.

*/
        png_bytep row = XTALLOC(PNG_INFO(img)->rowbytes, png_byte);
        int i, number_of_passes = png_set_interlace_handling(PNG_PTR(img));
        while (number_of_passes-->1) {
            for (i = 0; i < PNG_INFO(img)->height; i++)
                png_read_row(PNG_PTR(img), row, NULL);
        }
        XFREE(row);
        pdftex_warn("experimental support for interlaced png (please report bugs to pdftex@tug.org)");
    }
    if (PNG_INFO(img)->bit_depth == 16) {
        png_set_strip_16(PNG_PTR(img));
        pdftex_warn("can't handle 16 bits per channel, strip down to 8 bits");
    }
    png_read_update_info(PNG_PTR(img), PNG_INFO(img));
    return 0;
}

void write_png(integer n, integer img)
{
    int i, j;
    integer palette_objnum = 0;
    png_bytep row = XTALLOC(PNG_INFO(img)->rowbytes, png_byte);
    pdfbegindict(n);
    pdf_printf("/Type /XObject\n/Subtype /Image\n");
    pdfprintimageattr();
    pdf_printf("/Width %li\n/Height %li\n/BitsPerComponent %li\n",
               (long int)PNG_INFO(img)->width,
               (long int)PNG_INFO(img)->height,
               (long int)PNG_INFO(img)->bit_depth);
    pdf_printf("/ColorSpace ");
    switch (PNG_INFO(img)->color_type) {
    case PNG_COLOR_TYPE_PALETTE:
        pdfcreateobj(0, 0);
        palette_objnum = objptr;
        pdf_printf("[/Indexed /DeviceRGB %li %li 0 R]\n",
                   (long int)(PNG_INFO(img)->num_palette - 1),
                   (long int)palette_objnum);
        pdfimagec = 1;
        pdfimagei = 1;
        break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        pdf_printf("/DeviceGray\n");
        pdfimageb = 1;
        break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
        pdf_printf("/DeviceRGB\n");
        pdfimagec = 1;
        break;
    default:
        pdftex_fail("unsupported type of color_type <%i>", PNG_INFO(img)->color_type);
    }
    pdfbeginstream();
    for (i = 0; i < PNG_INFO(img)->height; i++) {
    	png_read_row(PNG_PTR(img), row, NULL);
        for (j = 0; j < PNG_INFO(img)->rowbytes; j++) {
            pdfbuf[pdfptr++] = row[j];
            if (pdfptr == pdfbufsize)
                pdfflush();
        }
    }
    pdfendstream();
    if (palette_objnum > 0) {
        pdfbegindict(palette_objnum);
        pdfbeginstream();
        for (i = 0; i < PNG_INFO(img)->num_palette; i++) {
            if (pdfptr + 3 >= pdfbufsize)
                pdfflush();
            pdfbuf[pdfptr++] = PNG_INFO(img)->palette[i].red;
            pdfbuf[pdfptr++] = PNG_INFO(img)->palette[i].green;
            pdfbuf[pdfptr++] = PNG_INFO(img)->palette[i].blue;
        }
        pdfendstream();
    }
    XFREE(row);
    pdfflush();
}
