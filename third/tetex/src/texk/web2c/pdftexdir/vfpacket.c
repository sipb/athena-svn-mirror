#include "libpdftex.h"

typedef struct {
    internalfontnumber font;
    char *dataptr;
    int  len;
} packet_entry;

static packet_entry *packet_ptr, *packet_tab = 0;
static int packet_max;

typedef struct {
    char **data;
    int *len;
    internalfontnumber font;
}  vf_entry;

static vf_entry *vf_ptr, *vf_tab = 0;
static int vf_max;

static char *packet_data_ptr;

integer newvfpacket(internalfontnumber f)
{
    int i, n = fontec[f] - fontbc[f] + 1;
    ENTRY_ROOM(vf, 256);
    vf_ptr->len = XTALLOC(n, int);
    vf_ptr->data = XTALLOC(n, char *);
    for (i = 0; i < n; i++) {
        vf_ptr->data[i] = 0;
        vf_ptr->len[i] = 0;
    }
    vf_ptr->font = f;
    return vf_ptr++ - vf_tab;
}

void storepacket(integer f, integer c, integer s)
{
    int l = strstart[s + 1] - strstart[s];
    vf_tab[vfpacketbase[f]].len[c - fontbc[f]] = l;
    vf_tab[vfpacketbase[f]].data[c - fontbc[f]] = XTALLOC(l, char);
    memcpy((void *)vf_tab[vfpacketbase[f]].data[c - fontbc[f]], 
           (void *)(strpool + strstart[s]), l);
}

void pushpacketstate()
{
    ENTRY_ROOM(packet, 256);
    packet_ptr->font = f;
    packet_ptr->dataptr = packet_data_ptr;
    packet_ptr->len = vfpacketlength;
    packet_ptr++;
}

void poppacketstate()
{
    if (packet_ptr == packet_tab)
        pdftex_fail("packet stack empty, impossible to pop");
    packet_ptr--;
    f = packet_ptr->font;
    packet_data_ptr = packet_ptr->dataptr;
    vfpacketlength = packet_ptr->len;
}

void startpacket(internalfontnumber f, integer c)
{
    packet_data_ptr = vf_tab[vfpacketbase[f]].data[c - fontbc[f]];
    vfpacketlength = vf_tab[vfpacketbase[f]].len[c - fontbc[f]];
}

eightbits packetbyte()
{
    vfpacketlength--;
    return *packet_data_ptr++;
}

void vf_free()
{
    vf_entry *v;
    int n;
    char **p;
    if (vf_tab != 0) {
        for (v = vf_tab; v < vf_ptr; v++) {
            XFREE(v->len);
            n = fontec[v->font] - fontec[v->font] + 1;
            for (p = v->data; p - v->data < n ; p++)
                XFREE(*p);
            XFREE(v->data);
        }
        XFREE(vf_tab);
    }
    XFREE(packet_tab);
}
