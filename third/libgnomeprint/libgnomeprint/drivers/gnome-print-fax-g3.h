#ifndef __GNOME_PRINT_RGB2FAX_H__
#define __GNOME_PRINT_RGB2FAX_H__



G_BEGIN_DECLS

typedef struct g3table{
    int code;
    int length;
} g3table;
 

/* Terminating white code table*/

struct g3table terminating_white_table[] = {
	{ 0x35 , 8 }, /*  0 */
	{ 0x7  , 6 }, /*  1 */
	{ 0x7  , 4 }, /*  2 */
	{ 0x8  , 4 }, /*  3 */ 
	{ 0xb  , 4 }, /*  4 */ 
	{ 0xc  , 4 }, /*  5 */ 
	{ 0xe  , 4 }, /*  6 */
	{ 0xf  , 4 }, /*  7 */ 
	{ 0x13 , 5 }, /*  8 */
	{ 0x14 , 5 }, /*  9 */
	{ 0x7  , 5 }, /* 10 */
	{ 0x8  , 5 }, /* 11 */
	{ 0x8  , 6 }, /* 12 */
	{ 0x3  , 6 }, /* 13 */
	{ 0x34 , 6 }, /* 14 */
	{ 0x35 , 6 }, /* 15 */
	{ 0x2a , 6 }, /* 16 */
	{ 0x2b , 6 }, /* 17 */
	{ 0x27 , 7 }, /* 18 */
	{ 0xc  , 7 }, /* 19 */
	{ 0x8  , 7 }, /* 20 */
	{ 0x17 , 7 }, /* 21 */
	{ 0x3  , 7 }, /* 22 */
	{ 0x4  , 7 }, /* 23 */
	{ 0x28 , 7 }, /* 24 */
	{ 0x2b , 7 }, /* 25 */
	{ 0x13 , 7 }, /* 26 */
	{ 0x24 , 7 }, /* 27 */
	{ 0x18 , 7 }, /* 28 */
	{ 0x2  , 8 }, /* 29 */
	{ 0x3  , 8 }, /* 30 */
	{ 0x1a , 8 }, /* 31 */
	{ 0x1b , 8 }, /* 32 */
	{ 0x12 , 8 }, /* 33 */
	{ 0x13 , 8 }, /* 34 */
	{ 0x14 , 8 }, /* 35 */
	{ 0x15 , 8 }, /* 36 */
	{ 0x16 , 8 }, /* 37 */
	{ 0x17 , 8 }, /* 38 */
	{ 0x28 , 8 }, /* 39 */
	{ 0x29 , 8 }, /* 40 */
	{ 0x2a , 8 }, /* 41 */
	{ 0x2b , 8 }, /* 42 */
	{ 0x2c , 8 }, /* 43 */
	{ 0x2d , 8 }, /* 44 */
	{ 0x4  , 8 }, /* 45 */
	{ 0x5  , 8 }, /* 46 */
	{ 0xa  , 8 }, /* 47 */
	{ 0xb  , 8 }, /* 48 */
	{ 0x52 , 8 }, /* 49 */
	{ 0x53 , 8 }, /* 50 */
	{ 0x54 , 8 }, /* 51 */
	{ 0x55 , 8 }, /* 52 */
	{ 0x24 , 8 }, /* 53 */
	{ 0x25 , 8 }, /* 54 */
	{ 0x58 , 8 }, /* 55 */
	{ 0x59 , 8 }, /* 56 */
	{ 0x5a , 8 }, /* 57 */
	{ 0x5b , 8 }, /* 58 */
	{ 0x4a , 8 }, /* 59 */
	{ 0x4b , 8 }, /* 60 */
	{ 0x32 , 8 }, /* 61 */
	{ 0x33 , 8 }, /* 62 */
	{ 0x34 , 8 }, /* 63 */
};

/* Make up white code table */

struct g3table makeup_white_table[] = {
	{ 0x1b, 5 }, /* 64 */
	{ 0x12, 5 }, /* 128 */
	{ 0x17, 6 }, /* 192 */
	{ 0x37, 7 }, /* 256 */
	{ 0x36, 8 }, /* 320 */
	{ 0x37, 8 }, /* 384 */
	{ 0x64, 8 }, /* 448 */
	{ 0x65, 8 }, /* 512 */
	{ 0x68, 8 }, /* 576 */
	{ 0x67, 8 }, /* 640 */
	{ 0xcc, 9 }, /* 704 */
	{ 0xcd, 9 }, /* 768 */
	{ 0xd2, 9 }, /* 832 */
	{ 0xd3, 9 }, /* 896 */
	{ 0xd4, 9 }, /* 960 */
	{ 0xd5, 9 }, /* 1024 */
	{ 0xd6, 9 }, /* 1088 */
	{ 0xd7, 9 }, /* 1152 */
	{ 0xd8, 9 }, /* 1216 */
	{ 0xd9, 9 }, /* 1280 */
	{ 0xda, 9 }, /* 1344 */
	{ 0xdb, 9 }, /* 1408 */
	{ 0x98, 9 }, /* 1472 */
	{ 0x99, 9 }, /* 1536 */
	{ 0x9a, 9 }, /* 1600 */
	{ 0x18, 6 }, /* 1664 */
	{ 0x9b, 9 }, /* 1728 */
    };

/* terminating black code table */

struct g3table terminating_black_table[] = {
	{ 0x37 , 10 }, /*  0 */
	{ 0x2  ,  3 }, /*  1 */
	{ 0x3  ,  2 }, /*  2 */
	{ 0x2  ,  2 }, /*  3 */
	{ 0x3  ,  3 }, /*  4 */
	{ 0x3  ,  4 }, /*  5 */
	{ 0x2  ,  4 }, /*  6 */
	{ 0x3  ,  5 }, /*  7 */
	{ 0x5  ,  6 }, /*  8 */
	{ 0x4  ,  6 }, /*  9 */
	{ 0x4  ,  7 }, /* 10 */
	{ 0x5  ,  7 }, /* 11 */
	{ 0x7  ,  7 }, /* 12 */
	{ 0x4  ,  8 }, /* 13 */
	{ 0x7  ,  8 }, /* 14 */
	{ 0x18 ,  9 }, /* 15 */
	{ 0x17 , 10 }, /* 16 */
	{ 0x18 , 10 }, /* 17 */
	{ 0x8  , 10 }, /* 18 */
	{ 0x67 , 11 }, /* 19 */
	{ 0x68 , 11 }, /* 20 */
	{ 0x6c , 11 }, /* 21 */
	{ 0x37 , 11 }, /* 22 */
	{ 0x28 , 11 }, /* 23 */
	{ 0x17 , 11 }, /* 24 */
	{ 0x18 , 11 }, /* 25 */
	{ 0xca , 12 }, /* 26 */
	{ 0xcb , 12 }, /* 27 */
	{ 0xcc , 12 }, /* 28 */
	{ 0xcd , 12 }, /* 29 */
	{ 0x68 , 12 }, /* 30 */
	{ 0x69 , 12 }, /* 31 */
	{ 0x6a , 12 }, /* 32 */
	{ 0x6b , 12 }, /* 33 */
	{ 0xd2 , 12 }, /* 34 */
	{ 0xd3 , 12 }, /* 35 */
	{ 0xd4 , 12 }, /* 36 */
	{ 0xd5 , 12 }, /* 37 */
	{ 0xd6 , 12 }, /* 38 */
	{ 0xd7 , 12 }, /* 39 */
	{ 0x6c , 12 }, /* 40 */
	{ 0x6d , 12 }, /* 41 */
	{ 0xda , 12 }, /* 42 */
	{ 0xdb , 12 }, /* 43 */
	{ 0x54 , 12 }, /* 44 */
	{ 0x55 , 12 }, /* 45 */
	{ 0x56 , 12 }, /* 46 */
	{ 0x57 , 12 }, /* 47 */
	{ 0x64 , 12 }, /* 48 */
	{ 0x65 , 12 }, /* 49 */
	{ 0x52 , 12 }, /* 50 */
	{ 0x53 , 12 }, /* 51 */
	{ 0x24 , 12 }, /* 52 */
	{ 0x37 , 12 }, /* 53 */
	{ 0x38 , 12 }, /* 54 */
	{ 0x27 , 12 }, /* 55 */
	{ 0x28 , 12 }, /* 56 */
	{ 0x58 , 12 }, /* 57 */
	{ 0x59 , 12 }, /* 58 */
	{ 0x2b , 12 }, /* 59 */
	{ 0x2c , 12 }, /* 60 */
	{ 0x5a , 12 }, /* 61 */
	{ 0x66 , 12 }, /* 62 */
	{ 0x67 , 12 }, /* 63 */
    };

/* Make up black code table */

struct g3table makeup_black_table[] = {
	{ 0xf  , 10 }, /*  64 */
	{ 0xc8 , 12 }, /* 128 */
	{ 0xc9 , 12 }, /*  192 */
	{ 0x5b , 12 }, /*  256 */
	{ 0x33 , 12 }, /*  320 */
	{ 0x34 , 12 }, /*  384 */
	{ 0x35 , 12 }, /*  448 */
	{ 0x6c , 13 }, /*  512 */
	{ 0x6d , 13 }, /*  576 */
	{ 0x4a , 13 }, /*  640 */
	{ 0x4b , 13 }, /*  704 */
	{ 0x4c , 13 }, /*  768 */
	{ 0x4d , 13 }, /*  832 */
	{ 0x72 , 13 }, /*  896 */
	{ 0x73 , 13 }, /*  960 */
	{ 0x74 , 13 }, /* 1024 */
	{ 0x75 , 13 }, /* 1088 */
	{ 0x76 , 13 }, /* 1152 */
	{ 0x77 , 13 }, /* 1216 */
	{ 0x52 , 13 }, /* 1280 */
	{ 0x53 , 13 }, /* 1344 */
	{ 0x54 , 13 }, /* 1408 */
	{ 0x55 , 13 }, /* 1472 */
	{ 0x5a , 13 }, /* 1536 */
	{ 0x5b , 13 }, /* 1600 */
	{ 0x64 , 13 }, /* 1664 */
	{ 0x65 , 13 }, /* 1728 */
    };

/* End of line code */

struct g3table g3eol = {0x1 , 12 };

G_END_DECLS

#endif /* __GNOME_PRINT_RGB2FAX_H__ */
