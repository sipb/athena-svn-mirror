#include "libpdftex.h"
#include "zlib.h"

#define ZIP_BUF_SIZE  32768

#define CHECK_ERR(f, fn)                                   \
    if (f != Z_OK)                                         \
        pdftex_fail("%s() failed", fn)

static char zipbuf[ZIP_BUF_SIZE];
static z_stream c_stream; /* compression stream */

void writezip(boolean finish, integer compress_level)
{
    int err;
    filename = NULL;
    if (pdfstreamlength == 0) {
        c_stream.zalloc = (alloc_func)0;
        c_stream.zfree = (free_func)0;
        c_stream.opaque = (voidpf)0;
        CHECK_ERR(deflateInit(&c_stream, compress_level), "deflateInit");
        c_stream.next_out = zipbuf;
        c_stream.avail_out = ZIP_BUF_SIZE;
    }
    c_stream.next_in = pdfbuf;
    c_stream.avail_in = pdfptr;
    if (!finish) {
        do {
            if (c_stream.avail_out == 0) {
                xfwrite(zipbuf, 1, ZIP_BUF_SIZE, pdffile);
                c_stream.next_out = zipbuf;
                c_stream.avail_out = ZIP_BUF_SIZE;
            }
            CHECK_ERR(deflate(&c_stream, Z_NO_FLUSH), "deflate");
        } while (c_stream.avail_in > 0);
    }
    else {
        for(;;) {
            if (c_stream.avail_out == 0) {
                xfwrite(zipbuf, 1, ZIP_BUF_SIZE, pdffile);
                c_stream.next_out = zipbuf;
                c_stream.avail_out = ZIP_BUF_SIZE;
            }
            if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
                break;
            if (err != Z_OK)
                pdftex_fail("deflate() failed");
        }
        if (c_stream.avail_out < ZIP_BUF_SIZE)
            xfwrite(zipbuf, 1, ZIP_BUF_SIZE - c_stream.avail_out, pdffile);
        CHECK_ERR(deflateEnd(&c_stream), "deflateEnd");
        xfflush(pdffile);
    }
    pdfstreamlength = c_stream.total_out;
}
