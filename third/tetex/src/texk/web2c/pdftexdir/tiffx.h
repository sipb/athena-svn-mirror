/* This is header file for TIFF support of pdfTeX. */

#include <stdio.h>
#include <tiff.h>
#include <tiffio.h>

/* TIFF_IMAGE_INFO is main structure for interchange of image data */

typedef struct {
  unsigned int width,              /* width of image in pixels                   */
               height,             /* height of image in pixels                  */
               x_res,              /* horizontal resolution in dpi, 0 if unknown */
               y_res,              /* vertical resolution in dpi, 0 if unknown   */
               color_space;        /* used color space. */
  unsigned char bits_per_component; /* bits per component                         */
  unsigned long length;             /* length of file/data                        */
  FILE *file;                    /* TIFF file                                   */
  } TIFF_IMAGE_INFO;


/* Function tiff_read_image_info opens specified file and reads informations
 * into TIFF_IMAGE_INFO structure.
 *
 * Return value: 0 - OK
 *               1 - Cannot open file for read
 */

int tiff_read_image_info(TIFF_IMAGE_INFO *image_info,char *file_name);


/* Function tiff_read_image reads specified number of bytes into specified
 * buffer.
 *
 * Return value: number of read bytes.0 - OK
 */

unsigned int tiff_read_file(TIFF_IMAGE_INFO *image_info,unsigned char *buf,unsigned int num_of_bytes);
