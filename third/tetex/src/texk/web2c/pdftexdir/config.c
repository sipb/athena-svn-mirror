#include "libpdftex.h"

#define CFG_BUF_SIZE     1024

static FILE *cfg_file;
static char config_name[] = "pdftex.cfg";
char *mapfiles;
boolean true_dimen;

#define CFG_OPEN()       texpsheaderbopenin(cfg_file)
#define CFG_CLOSE()      xfclose(cfg_file, filename)
#define CFG_GETCHAR()    xgetc(cfg_file)
#define CFG_EOF()        feof(cfg_file)

typedef struct {
    char *name;
    integer value;
    boolean is_true_dimen;
} cfg_entry;

#define CFG_FONT_MAP           0
#define CFG_OUTPUT_FORMAT      1
#define CFG_COMPRESS_LEVEL     2
#define CFG_DECIMAL_DIGITS     3
#define CFG_PK_RESOLUTION      4
#define CFG_IMAGE_RESOLUTION   5
#define CFG_INC_FORM_RESOURCES 6
#define CFG_PAGE_WIDTH         7
#define CFG_PAGE_HEIGHT        8
#define CFG_HORIGIN            9
#define CFG_VORIGIN            10
#define CFG_MAX                (CFG_VORIGIN + 1)

cfg_entry cfg_tab[CFG_MAX] = {
    {"map",                    0, false},
    {"output_format",          0, false},
    {"compress_level",         0, false},
    {"decimal_digits",         0, false},
    {"pk_resolution",          0, false},
    {"image_resolution",       0, false},
    {"include_form_resources", 0, false},
    {"page_width",             0, false},
    {"page_height",            0, false},
    {"horigin",                0, false},
    {"vorigin",                0, false}
};
                               
#define CFG_VALUE(F, V) integer F() { return cfg_tab[V].value; }

#define is_cfg_comment(c) (c == '*' || c == '#' || c == ';' || c == '%')

CFG_VALUE(cfgoutput,               CFG_OUTPUT_FORMAT)
CFG_VALUE(cfgcompresslevel,        CFG_COMPRESS_LEVEL)
CFG_VALUE(cfgdecimaldigits,        CFG_DECIMAL_DIGITS)
CFG_VALUE(cfgpkresolution,         CFG_PK_RESOLUTION)
CFG_VALUE(cfgimageresolution,      CFG_IMAGE_RESOLUTION)
CFG_VALUE(cfgincludeformresources, CFG_INC_FORM_RESOURCES)
CFG_VALUE(cfgpagewidth,            CFG_PAGE_WIDTH)
CFG_VALUE(cfgpageheight,           CFG_PAGE_HEIGHT)
CFG_VALUE(cfghorigin,              CFG_HORIGIN)
CFG_VALUE(cfgvorigin,              CFG_VORIGIN)

void readconfig()
{
    int c, res;
    cfg_entry *ce;
    char cfg_line[CFG_BUF_SIZE], *p, *r;
    filename = config_name;
    packfilename(maketexstring(filename), getnullstr(), getnullstr());
    if (!CFG_OPEN())
        pdftex_fail("cannot open config file");
    tex_printf("[%s", (nameoffile+1));
    mapfiles = 0;
    for (;;) {
        if (CFG_EOF()) {
            CFG_CLOSE();
            tex_printf("]");
            break;
        }
        p = cfg_line;
        do {
            c = CFG_GETCHAR();
            APPEND_CHAR_TO_BUF(c, p, cfg_line, CFG_BUF_SIZE);
        } while (c != 10);
        APPEND_EOL(p, cfg_line, CFG_BUF_SIZE);
        c = *cfg_line;
        if (p - cfg_line == 1 || is_cfg_comment(c))
            continue;
        p = cfg_line;
        for (ce = cfg_tab; ce - cfg_tab  < CFG_MAX; ce++)
            if (!strncmp(cfg_line, ce->name, strlen(ce->name)))
                break;
        if (ce - cfg_tab == CFG_MAX) {
            pdftex_warn("invalid parameter name in config file: `%s'", cfg_line);
            continue;
        }
        p = cfg_line + strlen(ce->name);
        if (*p == ' ')
            p++;
        if (*p == '=')
            p++;
        if (*p == ' ')
            p++;
        switch (ce - cfg_tab) {
        case CFG_FONT_MAP:
            if (*p != '+') {
                XFREE(mapfiles);
                mapfiles = 0;
            }
            else
                p++;
            for (r = p; *r != ' ' && *r != 10; r++);
            if (mapfiles == 0) {
                mapfiles = XTALLOC(r - p + 2, char);
                *mapfiles = 0;
            }
            else
                mapfiles = 
                    XRETALLOC(mapfiles, strlen(mapfiles) + r - p + 2, char);
            strncat(mapfiles, p, r - p);
            strcat(mapfiles, "\n");
            p = r;
            break;
        case CFG_OUTPUT_FORMAT:
        case CFG_COMPRESS_LEVEL:
        case CFG_DECIMAL_DIGITS:
        case CFG_PK_RESOLUTION:
        case CFG_IMAGE_RESOLUTION:
        case CFG_INC_FORM_RESOURCES:
            ce->value = myatol(&p);
            if (ce->value == -1) {
                pdftex_warn("invalid parameter value in config file: `%s'", cfg_line);
                ce->value = 0;
            }
            break;
        case CFG_PAGE_WIDTH:
        case CFG_PAGE_HEIGHT:
        case CFG_HORIGIN:
        case CFG_VORIGIN:
            ce->value = myatodim(&p);
            ce->is_true_dimen = true_dimen;
            break;
        }
        for (; *p == ' '; p++);
        if (*p != 10 && !is_cfg_comment(*p))
            pdftex_warn("invalid line in config file: `%s'", cfg_line);
    }
    res = cfgpkresolution();
    if (res == 0)
        res = 600;
    kpse_init_prog("PDFTEX", res, NULL, NULL);
    if (mapfiles == 0)
        mapfiles = xstrdup("psfonts.map\n");
}

void adjustcfgdimens(scaled m)
{
    cfg_entry *ce;
    for (ce = cfg_tab; ce - cfg_tab  < CFG_MAX; ce++)
        if (ce->is_true_dimen) {
            ce->value = xnoverd(ce->value, 1000, m);
        }
}
