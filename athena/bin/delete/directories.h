typedef short Boolean;
#define True			(Boolean) 1
#define False			(Boolean) 0

typedef enum {
     FtFile,
     FtDirectory,
     FtUnknown,
} filetype;
	  
typedef struct filrec {
     char name[MAXNAMLEN];
     filetype ftype;
     struct filrec *previous;
     struct filrec *parent;
     struct filrec *dirs;
     struct filrec *files;
     struct filrec *next;
     Boolean requested;
     Boolean specified;
     Boolean freed;
} filerec;

static filerec default_cwd = {
     ".",
     FtDirectory,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     False
};

static filerec default_root = {
     "",
     FtDirectory,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     False
};

static filerec default_directory = {
     "",
     FtDirectory,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     False
};

static filerec default_file = {
     "",
     FtFile,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     False
};
