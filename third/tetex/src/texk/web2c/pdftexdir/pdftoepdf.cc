#include <GString.h>
#include <gmem.h>
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "Params.h"
#include "Error.h"

#include "epdf.h"

#define PROCS_RES     0
#define FONTS_RES     1
#define XOBJS_RES     2
#define OTHER_RES     3

struct NewRef {
    Ref ref;    // ref in original PDF
    int newNum; // object number in output PDF
    NewRef *next; // next entry in list of indirect objects
};

GBool printCommands = gFalse;
NewRef *newRefList;
static GBool isInit = gFalse;

int addNewRef(Ref *r) {
    NewRef *p, *q, *n = new NewRef[1];

    n->ref = *r;
    n->next = NULL;
    if (newRefList == NULL)
        newRefList = n;
    else {
        for (p = newRefList; p != NULL; p = p->next) {
            if (p->ref.num == r->num && p->ref.gen == r->gen) {
                delete n;
                return p->newNum;
            }
            q = p;
        }
        q->next = n;
    }
    zpdfcreateobj(0, 0);
    return n->newNum = objptr;
}

void copyObject(Object *);
        
void copyStreamDict(Object *obj) {
    Object obj1;
    int i;

    for (i = 0; i < obj->dictGetLength(); ++i) {
        if (strcmp("Length", obj->dictGetKey(i)) == 0       ||
             strcmp("Filter", obj->dictGetKey(i)) == 0      ||
             strcmp("DecodeParms", obj->dictGetKey(i)) == 0 ||
             strcmp("F", obj->dictGetKey(i)) == 0           ||
             strcmp("FFilter", obj->dictGetKey(i)) == 0     ||
             strcmp("FDecodeParms", obj->dictGetKey(i)) == 0)
             continue;
        writeEPDF("/%s ", obj->dictGetKey(i));
        obj->dictGetValNF(i, &obj1);
        copyObject(&obj1);
        obj1.free();
        writeEPDF("\n");
    }
}

void copyDict(Object *obj) {
    Object obj1;
    int i;

    for (i = 0; i < obj->dictGetLength(); ++i) {
        writeEPDF("/%s ", obj->dictGetKey(i));
        obj->dictGetValNF(i, &obj1);
        copyObject(&obj1);
        obj1.free();
        writeEPDF("\n");
    }
}

void copyStream(Object *obj) {
    int c;
    
    obj->streamReset();
    while ((c = obj->streamGetChar()) != EOF)
        pdfout(c);
}

void copyRes(Object *obj, char *key) {
    int i, k, res_type;
    Object obj1;
    Ref newref;

    if (strcmp("ProcSet", key) == 0)
        res_type = PROCS_RES;
    else if (strcmp("Font", key) == 0)
        res_type = FONTS_RES;
    else if (strcmp("XObject", key) == 0)
        res_type = XOBJS_RES;
    else
        res_type = OTHER_RES;

    if (res_type == PROCS_RES) {
        if (!obj->isArray())
            error(-1, "invalid ProcSet type <%s>", obj->getTypeName());
        writeEPDF("/%s [ ", key);
        for (i = 0; i < obj->arrayGetLength(); ++i) {
            obj->arrayGetNF(i, &obj1);
            if (!obj1.isName())
                error(-1, "invalid ProcSet type <%s>", obj1.getTypeName());
            if (strcmp(obj1.getName(), "Text") == 0)
                pdftext = 1;
            else if (strcmp(obj1.getName(), "ImageB") == 0)
                pdfimageb = 1;
            else if (strcmp(obj1.getName(), "ImageC") == 0)
                pdfimagec = 1;
            else if (strcmp(obj1.getName(), "ImageI") == 0)
                pdfimagei = 1;
            else if (strcmp(obj1.getName(), "PDF") != 0)
                error(-1, "invalid ProcSet name <%s>", obj1.getName());
            writeEPDF("/%s ", obj1.getName());
            obj1.free();
        }
        writeEPDF("]\n");
    }
    else {
        if (!obj->isDict())
            error(-1, "invalid resources type <%s>", obj->getTypeName());
        writeEPDF("/%s << ", key);
        if (res_type == OTHER_RES)
             add_other_resources();
        for (i = 0; i < obj->dictGetLength(); ++i) {
            obj->dictGetValNF(i, &obj1);
            if (!obj1.isRef())
                error(-1, "invalid resources type <%s>", obj->getTypeName());
            newref = obj1.getRef();
            k = addNewRef(&newref);
            writeEPDF("/%s %d 0 R ", obj->dictGetKey(i), k);
            add_resources_name(obj->dictGetKey(i));
            switch (res_type) {
            case FONTS_RES:
                add_extra_fonts();
                break;
            case XOBJS_RES:
                add_extra_xobjects();
                break;
            case OTHER_RES:
                add_other_resources();
            }
        }
        writeEPDF(">>\n");
        if (res_type == OTHER_RES)
             add_other_resources();
    }
}
    
void copyObject(Object *obj) {
    int  i;
    char *p;
    Object obj1;

    if (obj->isBool()) {
        writeEPDF("%s", obj->getBool() ? "true" : "false");
    }
    else if (obj->isInt()) {
        writeEPDF("%i", obj->getInt());
    }
    else if (obj->isReal()) {
        writeEPDF("%g", obj->getReal());
    }
    else if (obj->isNum()) {
        writeEPDF("%g", obj->getNum());
    }
    else if (obj->isString()) {
        writeEPDF("(");
        for (p = obj->getString()->getCString(); *p; p++)
            if (*p == '(' || *p == ')' || *p == '\\')
                writeEPDF("\\%c", *p);
            else if (!(*p > 0x20 && *p < 0x80 - 1))
                writeEPDF("\\%03o", *p);
            else
                pdfout(*p);
        writeEPDF(")");
    }
    else if (obj->isName()) {
        writeEPDF("/%s", obj->getName());
    }
    else if (obj->isNull()) {
    }
    else if (obj->isArray()) {
        writeEPDF("[ ");
        for (i = 0; i < obj->arrayGetLength(); ++i) {
            obj->arrayGetNF(i, &obj1);
            copyObject(&obj1);
            writeEPDF(" ");
            obj1.free();
        }
        writeEPDF("]");
    }
    else if (obj->isDict()) {
        writeEPDF("<<\n");
        copyDict(obj);
        writeEPDF(">>");
    }
    else if (obj->isStream()) {
        obj1.initDict(obj->getStream()->getDict());
        writeEPDF("<<\n");
        copyStreamDict(&obj1);
        obj1.free();
        pdfbeginstream();
        copyStream(obj);
        pdfendstream();
    }
    else if (obj->isRef()) {
	Ref newref = obj->getRef();
        writeEPDF("%d 0 R", addNewRef(&newref));
    }
    else {
        error(-1, "type <%s> cannot be copied", obj->getTypeName());
    }
}

void writeRefs() {
    NewRef *r, *n;
    Object obj;

    for (r = newRefList; r != NULL; r = r->next) {
        zpdfbeginobj(r->newNum);
        xref->fetch(r->ref.num, r->ref.gen, &obj);
        copyObject(&obj);
        if (!obj.isStream())
            writeEPDF("\nendobj\n");
        obj.free();
    }
    for (r = newRefList; r != NULL; r = n) {
        n = r->next;
        delete r;
    }
}

integer read_pdf_info(char *image_name) {
    PDFDoc *doc;
    GString *docName;
    Page *page;
    Object contents, obj, resources;

    // initialize
    if (!isInit) {
        isInit = gTrue;
        errorInit();
    }

    // open PDF file
    xref = NULL;
    docName = new GString(image_name);
    doc = new PDFDoc(docName);
    if (!doc->isOk() || !doc->okToPrint())
        return -1;
    epdf_doc = (void *)doc;
    epdf_xref = (void *)xref;

    // get the first page
    page = doc->getCatalog()->getPage(1);
    if (page->isCropped()) {
        epdf_width = (int)(page->getCropX2() - page->getCropX1());
        epdf_height = (int)(page->getCropY2() - page->getCropY1());
        epdf_orig_x = (int)page->getCropX1();
        epdf_orig_y = (int)page->getCropY1();
    }
    else {
        epdf_width = (int)(page->getX2() - page->getX1());
        epdf_height = (int)(page->getY2() - page->getY1());
        epdf_orig_x = (int)page->getX1();
        epdf_orig_y = (int)page->getY1();
    }
    return 0;
}

void write_epdf(integer n, integer img) {
    Page *page;
    Object contents, obj, obj1;
    int i;

    xref = (XRef *)epdf_xref;
    page = ((PDFDoc *)epdf_doc)->getCatalog()->getPage(1);
    newRefList = NULL;
    
    // write the Page header
    zpdfbegindict(n);
    writeEPDF("/Type /XObject\n");
    writeEPDF("/Subtype /Form\n");
    writeEPDF("/FormType 1\n");
    writeEPDF("/Matrix [1 0 0 1 0 0]\n");
    pdfprintimageattr();
    if (page->isCropped())
        writeEPDF("/BBox [%i %i %i %i]\n",
                  (int)page->getCropX1(),
                  (int)page->getCropY1(),
                  (int)page->getCropX2(),
                  (int)page->getCropY2());
    else 
        writeEPDF("/BBox [%i %i %i %i]\n",
                  (int)page->getX1(),
                  (int)page->getY1(),
                  (int)page->getX2(),
                  (int)page->getY2());

    // write the Resources dictionary
    obj.initDict(page->getResourceDict());
    if (!obj.isDict())
        error(-1, "invalid resources type <%s>", obj.getTypeName());
    if (pdfincludeformresources == 1) {
        writeEPDF("/Resources <<\n");
        for (i = 0; i < obj.dictGetLength(); ++i) {
            obj.dictGetValNF(i, &obj1);
            copyRes(&obj1, obj.dictGetKey(i));
            obj1.free();
        }
        writeEPDF(">>\n");
    }
    else {
        writeEPDF("/Resources ");
        copyObject(&obj);
        writeEPDF("\n");
    }
    obj.free();

    // write the page contents
    page->getContents(&contents);
    pdfbeginstream();
    if (contents.isArray()) {
        for (i = 0; i < contents.arrayGetLength(); ++i) {
            contents.arrayGet(i, &obj);
            if (!obj.isStream())
                error(-1, "invalid page contents array element type <%s>", obj.getTypeName());
            copyStream(&obj);
            obj.free();
        }
    }
    else if (contents.isStream()) {
        copyStream(&contents);
    }
    else
        error(-1, "invalid page contents type <%s>", contents.getTypeName());
    contents.free();
    pdfendstream();

    // write out all indirect objects
    writeRefs();
}

void epdf_delete() {
    xref = (XRef *)epdf_xref;
    delete (PDFDoc *)epdf_doc;
}

void epdf_check_mem() {
    if (isInit) {
        Object::memCheck(errFile);
        gMemReport(errFile);
    }
}
