/* This is header file for JPG support of pdfTeX. */

/*
#include <kpathsea/c-auto.h>
#include <kpathsea/c-std.h>
*/

#define JPG_GRAY  1     /* Gray color space, use /DeviceGray  */
#define JPG_RGB   3     /* RGB color space, use /DeviceRGB    */
#define JPG_CMYK  4     /* CMYK color space, use /DeviceCMYK  */

#define JPG_UINT16      unsigned int
#define JPG_UINT32      unsigned long
#define JPG_UINT8       unsigned char

/* JPG_IMAGE_INFO is main structure for interchange of image data */

typedef struct {
  JPG_UINT16 width,              /* width of image in pixels                   */
             height,             /* height of image in pixels                  */
             x_res,              /* horizontal resolution in dpi, 0 if unknown */
             y_res,              /* vertical resolution in dpi, 0 if unknown   */
             color_space;        /* used color space. See JPG_ constants       */
  JPG_UINT8  bits_per_component; /* bits per component                         */
  JPG_UINT32 length;             /* length of file/data                        */
  FILE *file;                    /* jpg file                                   */
  } JPG_IMAGE_INFO;


/* Function jpg_read_image_info opens specified file and reads informations
 * into JPG_IMAGE_INFO structure.
 *
 * Return value: 0 - OK
 *               1 - Cannot open file for read
 *               2 - Not JPG file
 *               3 - Cannot find necessary informations
 *               4 - Unsupported type of compression
 */

int jpg_read_image_info(JPG_IMAGE_INFO *image_info,char *file_name);


/* Function jpg_read_image reads specified number of bytes into specified
 * buffer.
 *
 * Return value: number of read bytes.0 - OK
 */

JPG_UINT16 jpg_read_file(JPG_IMAGE_INFO *image_info,JPG_UINT8* buf,JPG_UINT16 num_of_bytes);
