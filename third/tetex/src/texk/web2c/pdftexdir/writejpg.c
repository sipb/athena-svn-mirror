
/* -----------------------------------------------------------------------
 * This is JPG support to pdfTeX. It was written by Jiri Osoba in May 1998
 * -----------------------------------------------------------------------
 */

#include "libpdftex.h"
#include "image.h"

typedef enum {		/* JPEG marker codes			*/
  M_SOF0  = 0xc0,	/* baseline DCT				*/
  M_SOF1  = 0xc1,	/* extended sequential DCT		*/
  M_SOF2  = 0xc2,	/* progressive DCT			*/
  M_SOF3  = 0xc3,	/* lossless (sequential)		*/

  M_SOF5  = 0xc5,	/* differential sequential DCT		*/
  M_SOF6  = 0xc6,	/* differential progressive DCT		*/
  M_SOF7  = 0xc7,	/* differential lossless		*/

  M_JPG   = 0xc8,	/* JPEG extensions			*/
  M_SOF9  = 0xc9,	/* extended sequential DCT		*/
  M_SOF10 = 0xca,	/* progressive DCT			*/
  M_SOF11 = 0xcb,	/* lossless (sequential)		*/

  M_SOF13 = 0xcd,	/* differential sequential DCT		*/
  M_SOF14 = 0xce,	/* differential progressive DCT		*/
  M_SOF15 = 0xcf,	/* differential lossless		*/

  M_DHT   = 0xc4,	/* define Huffman tables		*/

  M_DAC   = 0xcc,	/* define arithmetic conditioning table	*/

  M_RST0  = 0xd0,	/* restart				*/
  M_RST1  = 0xd1,	/* restart				*/
  M_RST2  = 0xd2,	/* restart				*/
  M_RST3  = 0xd3,	/* restart				*/
  M_RST4  = 0xd4,	/* restart				*/
  M_RST5  = 0xd5,	/* restart				*/
  M_RST6  = 0xd6,	/* restart				*/
  M_RST7  = 0xd7,	/* restart				*/

  M_SOI   = 0xd8,	/* start of image			*/
  M_EOI   = 0xd9,	/* end of image				*/
  M_SOS   = 0xda,	/* start of scan			*/
  M_DQT   = 0xdb,	/* define quantization tables		*/
  M_DNL   = 0xdc,	/* define number of lines		*/
  M_DRI   = 0xdd,	/* define restart interval		*/
  M_DHP   = 0xde,	/* define hierarchical progression	*/
  M_EXP   = 0xdf,	/* expand reference image(s)		*/

  M_APP0  = 0xe0,	/* application marker, used for JFIF	*/
  M_APP1  = 0xe1,	/* application marker			*/
  M_APP2  = 0xe2,	/* application marker			*/
  M_APP3  = 0xe3,	/* application marker			*/
  M_APP4  = 0xe4,	/* application marker			*/
  M_APP5  = 0xe5,	/* application marker			*/
  M_APP6  = 0xe6,	/* application marker			*/
  M_APP7  = 0xe7,	/* application marker			*/
  M_APP8  = 0xe8,	/* application marker			*/
  M_APP9  = 0xe9,	/* application marker			*/
  M_APP10 = 0xea,	/* application marker			*/
  M_APP11 = 0xeb,	/* application marker			*/
  M_APP12 = 0xec,	/* application marker			*/
  M_APP13 = 0xed,	/* application marker			*/
  M_APP14 = 0xee,	/* application marker, used by Adobe	*/
  M_APP15 = 0xef,	/* application marker			*/

  M_JPG0  = 0xf0,	/* reserved for JPEG extensions		*/
  M_JPG13 = 0xfd,	/* reserved for JPEG extensions		*/
  M_COM   = 0xfe,	/* comment				*/

  M_TEM   = 0x01,	/* temporary use			*/

  M_ERROR = 0x100	/* dummy marker, internal use only	*/
} JPEG_MARKER;

#define read2bytes(a) (JPG_UINT16)((xgetc(a)<<8)+xgetc(a))


/* Function jpg_read_image_info opens specified file and reads informations
 * into JPG_IMAGE_INFO structure.
 *
 * Return value: 0 - OK
 *               1 - Cannot open file for read
 *               2 - Not JPG file
 *               3 - Cannot find necessary informations
 */

integer read_jpg_info(integer img)
{
  int i,j;
  char jpg_id[]="JFIF";
  int units;
  JPG_IMAGE_INFO *image_info = JPG_INFO(img);

  JPG_INFO(img)->file = xfopen(IMG_NAME(img), FOPEN_RBIN_MODE);
  xfseek(image_info->file,0,SEEK_END,filename);
  image_info->length=xftell(image_info->file,filename);
  xfseek(image_info->file,0,SEEK_SET,filename);
  if(read2bytes(image_info->file)!=0xFFD8) {
    xfclose(image_info->file,filename); return(2);
  }
  while(1) {
    if(feof(image_info->file) || fgetc(image_info->file)!=0xFF) {
      xfclose(image_info->file,filename); return(2);
    }
    switch(xgetc(image_info->file)) {
      case M_APP0:		/* check for JFIF marker with resolution */
	j=read2bytes(image_info->file);
        for(i=0;i<5;i++) if(xgetc(image_info->file)!=jpg_id[i]) {
          xfclose(image_info->file,filename); return(2);
        }
        (void)read2bytes(image_info->file);
        units=xgetc(image_info->file);
        image_info->x_res=read2bytes(image_info->file);
        image_info->y_res=read2bytes(image_info->file);
        switch(units) {
          case 1: break; /* pixels per inch */
          case 2: image_info->x_res*=2.54; image_info->y_res*=2.54;
                  break; /* pixels per cm */
          default:image_info->x_res=image_info->y_res=0; break;
        }
        xfseek(image_info->file,j-14,SEEK_CUR,filename); break;
      case M_SOF2:
      case M_SOF3:
      case M_SOF5:
      case M_SOF6:
      case M_SOF7:
      case M_SOF9:
      case M_SOF10:
      case M_SOF11:
      case M_SOF13:
      case M_SOF14:
      case M_SOF15:xfclose(image_info->file,filename); return(4);
      case M_SOF0:
      case M_SOF1:
	(void)read2bytes(image_info->file);    /* read segment length  */
	image_info->bits_per_component = xgetc(image_info->file);
	image_info->height             = read2bytes(image_info->file);
	image_info->width              = read2bytes(image_info->file);
	image_info->color_space        = xgetc(image_info->file);
        xfseek(image_info->file,0,SEEK_SET,filename); return(0);

      case M_SOI:		/* ignore markers without parameters */
      case M_EOI:
      case M_TEM:
      case M_RST0:
      case M_RST1:
      case M_RST2:
      case M_RST3:
      case M_RST4:
      case M_RST5:
      case M_RST6:
      case M_RST7:
	break;

      default:			/* skip variable length markers */
        xfseek(image_info->file,read2bytes(image_info->file)-2,SEEK_CUR,filename);
        break;
    }
  }
}

void write_jpg(integer n, integer img)
{
    long unsigned l;
    pdfbegindict(n);
    pdf_printf("/Type /XObject\n/Subtype /Image\n");
    pdfprintimageattr();
    pdf_printf("/Name /Im%li\n/Width %li\n/Height %li\n/BitsPerComponent %li\n/Length %li\n",
               (long int)objinfo(n),
               (long int)JPG_INFO(img)->width,
               (long int)JPG_INFO(img)->height,
               (long int)JPG_INFO(img)->bits_per_component,
               (long int)JPG_INFO(img)->length);
    pdf_printf("/ColorSpace ");
    switch (JPG_INFO(img)->color_space) {
    case JPG_GRAY:
        pdf_printf("/DeviceGray\n");
        pdfimageb = 1;
        break;
    case JPG_RGB:
        pdf_printf("/DeviceRGB\n");
        pdfimagec = 1;
        break;
    case JPG_CMYK:
        pdf_printf("/DeviceCMYK\n/Decode [1 0 1 0 1 0 1 0]\n");
        pdfimagec = 1;
        break;
    default:
        pdftex_fail("Unsupported color space %i", 
             (int)JPG_INFO(img)->color_space);
    }
    pdf_printf("/Filter /DCTDecode\n>>\nstream\n");
    for (l = JPG_INFO(img)->length; l >0; l--)
        pdfout(xgetc(JPG_INFO(img)->file));
    pdf_printf("endstream\nendobj\n");
}
