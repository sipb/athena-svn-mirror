/*
 * M K E D B
 *
 * Program to compile the error data base from text to binary format.
 */

#include <sys/types.h> 
#include <stdio.h>

#include "error.h"

int CurLine = 0;
FILE *InFD, *OutFD;

enum tok_t {
    TOK_EOF,
    TOK_SUBSYS,
    TOK_MODULE,
    TOK_CODE,
    TOK_END
};

#define N_TOK_TYPES 4

struct {
    char *text;
    enum tok_t tok;
} TokTable[N_TOK_TYPES] = {
    {"subsys",  TOK_SUBSYS},
    {"module",  TOK_MODULE},
    {"code",    TOK_CODE}, 
    {"end",     TOK_END} 
};


struct subsys_t {
    char *name;
    struct module_t {
        char *name;
        short min_code;
        short max_code;
        char **codes;
    } modtab[MAX_MODULE + 1];
} SubsysTab[MAX_SUBSYS + 1];


/*
 * g e t L i n e
 *
 * Get a single line.  Ignore blank lines and lines beginning with "#".  Parse off
 * the first token and return its type.
 */

enum tok_t
getLine(s)
    char *s;
{
    unsigned short i;
    char buff[100];
    char toktext[30];
    int match;
    int ntok;

    do {
        if (fgets(buff, sizeof buff, InFD) == NULL) {
            fprintf(stderr, "(mkedb) Unexpected error (possibly missing \"end\") at line %d\n", CurLine);
            exit(1);
        }
        CurLine++;
    }
    while (buff[0] == '#' || strlen(buff) == 1);

    ntok = sscanf(buff, "%s %[^\n]", toktext, s);

    if (ntok < 1) {
        fprintf(stderr, "(mkedb) No tokens on line %d\n", CurLine);
        exit(1);
    }

    match = 0;

    for (i = 0; i < N_TOK_TYPES; i++)
        if (strcmp(TokTable[i].text, toktext) == 0) {
            match = 1;
            break;
        }

    if (! match) {
        fprintf(stderr, "(mkedb) Unknown keyword in line:\n    %s", buff);
        exit(1);
    }
     
    if (ntok != 2 && TokTable[i].tok != TOK_END) {
        fprintf(stderr, "(mkedb) Not enough tokens on line %d:\n    %s", CurLine, buff);
        exit(1);
    }

    return TokTable[i].tok;
}


/*
 * r e a d L i n e
 *
 * Get a line that begins with the specified token type and parse off the fields.
 */

int
readLine(wanttok, text, val)
    enum tok_t wanttok;
    char *text;
    short *val;
{
    char buff[100];
    enum tok_t tok;
    int i;

    tok = getLine(buff);
    if (tok == TOK_END)
        return 0;

    if (tok != wanttok) {
        fprintf(stderr, "(mkedb) Wrong keyword in line %d:\n    %s\n", CurLine, buff);
        exit(1);
    }

    if (sscanf(buff, "%x %[^\n]", &i, text) != 2) {
        fprintf(stderr, "(mkedb) Ill-formed line %d:\n     %s\n", CurLine, buff);
        exit(1);
    }

    *val = (short) i;
    return 1;
}


#if ! defined(apollo) || ! defined(SYS5)

/*
 * s t r d u p 
 *
 * Make a copy of a string in malloc'd storage.
 */

char *
strdup(s)
{
    char *p = (char *) malloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

#endif


/*
 * l o a d M o d u l e
 *
 * Load information for a single module.
 */

void 
loadModule(ss, module, name)
    int ss;
    int module;
    char *name;
{
    char text[100];
    short code, min_code, n_codes, prev_code;
    short j;
    int first_time;
#   define MAX_CODES 0xefff     /* ffff? Avoid sign problems for now */
    static char **ctable = NULL;
    char **atable;

    if (ctable == NULL)
        ctable = (char **) malloc(sizeof(char *) * MAX_CODES);

    first_time = 1;

    while (readLine(TOK_CODE, text, &code)) {
        if (first_time) {
            min_code = code;
            first_time = 0;
        }
        else {
            short i;

            if (code <= prev_code) {
                fprintf(stderr, "(mkedb) Codes must be in increasing order within a single module (near line %d)\n", CurLine);
                exit(1);
            }

            if (code - min_code > MAX_CODES) {
                fprintf(stderr, "(mkedb) Too many codes for one module (near line %d)\n", CurLine);
                exit(1);
            }

            for (i = prev_code + 1; i < code; i++)
                ctable[i - min_code] = "";
        }

        ctable[code - min_code] = strdup(text);
        prev_code = code;
    }

    if (first_time)  /* handle case of empty module */
        n_codes = 0;
    else
        n_codes = prev_code - min_code + 1;

    atable = (char **) malloc(sizeof(char *) * n_codes);
    for (j = 0; j < n_codes; j++)
        atable[j] = ctable[j]; 

    SubsysTab[ss].modtab[module].name     = strdup(name);
    SubsysTab[ss].modtab[module].min_code = min_code;
    SubsysTab[ss].modtab[module].max_code = min_code + n_codes - 1;
    SubsysTab[ss].modtab[module].codes    = atable;
}


/*
 * l o a d S u b s y s
 * 
 * Load info for a single subsystem.
 */

void 
loadSubsys(ss, name)
    int ss;
    char *name;
{
    char mname[100];
    short module;

    SubsysTab[ss].name = strdup(name);

    while (readLine(TOK_MODULE, mname, &module))
        loadModule(ss, module, mname);
}


/*
 * d u m p S u b M o d
 *
 * Write out information for a single valid [subsystem, module] pair.
 */

void 
dumpSubMod(ss, module)
    int ss;
    int module;
{
    struct module_t *mp = &SubsysTab[ss].modtab[module];
    short i, j;
    struct modhdr_t modhdr;
    
#ifdef EDEBUG
    printf("    subsys = %02x, module = %02x (%s/%s)\n", ss, module, SubsysTab[ss].name, mp->name);
#endif

    modhdr.min_code = mp->min_code;
    modhdr.max_code = mp->max_code;

    swab_16(&modhdr.min_code);
    swab_16(&modhdr.max_code);

    strcpy(modhdr.ss_name, SubsysTab[ss].name);
    strcpy(modhdr.mod_name, mp->name);

    fwrite(&modhdr, sizeof modhdr, 1, OutFD);

    for (i = mp->min_code, j = 0; i <= mp->max_code; i++, j++)
        fwrite(mp->codes[j], 1, strlen(mp->codes[j]) + 1, OutFD);
}


/*
 * m a i n 
 * 
 * Main program.
 */

main(argc, argv)
int argc;
char *argv[];
{
    short ss, module;
    char sname[100];
    long ssmm;
    long version;
    long hdr_size;
    struct hdr_t *hdr;

    if (argc < 3) {
        fprintf(stderr, "usage: mkedb <input> <output>\n");
        exit(1);
    }

    /*
     * Process the input file.
     */

    if ((InFD = fopen(argv[1], "r")) == NULL) {
        perror("Can't open input file");
        exit(1);
    }

    printf("(mkedb) Scanning input...\n");

    while (readLine(TOK_SUBSYS, sname, &ss)) {
#ifdef EDEBUG
        printf("    subsystem %02x, %s\n", ss, sname);
#endif
        loadSubsys(ss, sname);
    }

    fclose(InFD);

    /*
     * Generate the output file.
     */

    printf("(mkedb) Generating output...\n"); 

    if ((OutFD = fopen(argv[2], "w")) == NULL) {
        perror("Can't open output file");
        exit(1);
    }

    /*
     * Count the number of valid subsys/module pairs.  We do this so we know
     * how big the header is going to be and hence where the text section should
     * start.
     */

    ssmm = 0;

    for (ss = 0; ss < MAX_SUBSYS; ss++)
        if (SubsysTab[ss].name != NULL)
            for (module = 0; module < MAX_MODULE; module++) 
                if (SubsysTab[ss].modtab[module].name != NULL)
                    ssmm++;

    printf("(mkedb) %d valid subsystem/module pairs found\n", ssmm);
    hdr_size = ssmm * sizeof(struct hdr_elt_t) + sizeof(struct hdr_hdr_t);

    hdr = (struct hdr_t *) malloc(hdr_size);
    if (hdr == NULL) {
        fprintf(stderr, "(mkedb) Can't allocate header storage\n");
        exit(1);
    }

    hdr->h.version = ERROR_VERSION;
    hdr->h.count = ssmm;

    swab_32(&hdr->h.version);
    swab_32(&hdr->h.count);

    fseek(OutFD, hdr_size, 0);
    ssmm = 0;

    for (ss = 0; ss < MAX_SUBSYS; ss++)
        if (SubsysTab[ss].name != NULL)
            for (module = 0; module < MAX_MODULE; module++) 
                if (SubsysTab[ss].modtab[module].name != NULL) {
                    struct hdr_elt_t hdre;

                    hdr->ents[ssmm].submod = (ss << 8) | module;
                    hdr->ents[ssmm].offset = ftell(OutFD); 
                    hdr->ents[ssmm].pad    = 0;

                    swab_16(&hdr->ents[ssmm].submod);
                    swab_32(&hdr->ents[ssmm].offset);

                    ssmm++;
                    dumpSubMod(ss, module);
                }

    /*
     * Seek back to top and write out the header.
     */

    fseek(OutFD, 0, 0);
    fwrite(hdr, 1, hdr_size, OutFD);

    fclose(OutFD);
}
