/* -*- Mode: Text -*- */

struct dent {
	struct dent *next;
	char *word;

	unsigned short used : 1;

/* bit fields for all of the flags */
	unsigned short v_flag : 1;
		/*
			"V" flag:
		        ...E --> ...IVE  as in CREATE --> CREATIVE
		        if # .ne. E, ...# --> ...#IVE  as in PREVENT --> PREVENTIVE
		*/
	unsigned short n_flag : 1;
		/*
			"N" flag:
			        ...E --> ...ION  as in CREATE --> CREATION
			        ...Y --> ...ICATION  as in MULTIPLY --> MULTIPLICATION
			        if # .ne. E or Y, ...# --> ...#EN  as in FALL --> FALLEN
		*/
	unsigned short x_flag : 1;
		/*
			"X" flag:
			        ...E --> ...IONS  as in CREATE --> CREATIONS
			        ...Y --> ...ICATIONS  as in MULTIPLY --> MULTIPLICATIONS
			        if # .ne. E or Y, ...# --> ...#ENS  as in WEAK --> WEAKENS
		*/
	unsigned short h_flag : 1;
		/*
			"H" flag:
			        ...Y --> ...IETH  as in TWENTY --> TWENTIETH
			        if # .ne. Y, ...# --> ...#TH  as in HUNDRED --> HUNDREDTH
		*/
	unsigned short y_flag : 1;
		/*
			"Y" FLAG:
			        ... --> ...LY  as in QUICK --> QUICKLY
		*/
	unsigned short g_flag : 1;
		/*
			"G" FLAG:
			        ...E --> ...ING  as in FILE --> FILING
			        if # .ne. E, ...# --> ...#ING  as in CROSS --> CROSSING
		*/
	unsigned short j_flag : 1;
		/*
			"J" FLAG"
			        ...E --> ...INGS  as in FILE --> FILINGS
			        if # .ne. E, ...# --> ...#INGS  as in CROSS --> CROSSINGS
		*/
	unsigned short d_flag : 1;
		/*
			"D" FLAG:
			        ...E --> ...ED  as in CREATE --> CREATED
			        if @ .ne. A, E, I, O, or U,
			                ...@Y --> ...@IED  as in IMPLY --> IMPLIED
			        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
			                ...@# --> ...@#ED  as in CROSS --> CROSSED
			                                or CONVEY --> CONVEYED
		*/
	unsigned short t_flag : 1;
		/*
			"T" FLAG:
			        ...E --> ...EST  as in LATE --> LATEST
			        if @ .ne. A, E, I, O, or U,
			                ...@Y --> ...@IEST  as in DIRTY --> DIRTIEST
			        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
			                ...@# --> ...@#EST  as in SMALL --> SMALLEST
			                                or GRAY --> GRAYEST
		*/
	unsigned short r_flag : 1;
		/*
			"R" FLAG:
			        ...E --> ...ER  as in SKATE --> SKATER
			        if @ .ne. A, E, I, O, or U,
			                ...@Y --> ...@IER  as in MULTIPLY --> MULTIPLIER
			        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
			                ...@# --> ...@#ER  as in BUILD --> BUILDER
			                                or CONVEY --> CONVEYER
		*/
	unsigned short z_flag : 1;
		/*
			"Z FLAG:
			        ...E --> ...ERS  as in SKATE --> SKATERS
			        if @ .ne. A, E, I, O, or U,
			                ...@Y --> ...@IERS  as in MULTIPLY --> MULTIPLIERS
			        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
			                ...@# --> ...@#ERS  as in BUILD --> BUILDERS
			                                or SLAY --> SLAYERS
		*/
	unsigned short s_flag : 1;
		/*
			"S" FLAG:
			        if @ .ne. A, E, I, O, or U,
			                ...@Y --> ...@IES  as in IMPLY --> IMPLIES
			        if # .eq. S, X, Z, or H,
			                ...# --> ...#ES  as in FIX --> FIXES
			        if # .ne. S,X,Z,H, or Y, or (# = Y and @ = A, E, I, O, or U)
			                ...# --> ...#S  as in BAT --> BATS
			                                or CONVEY --> CONVEYS
		*/
	unsigned short p_flag : 1;
		/*
			"P" FLAG:
			        if @ .ne. A, E, I, O, or U,
			                ...@Y --> ...@INESS  as in CLOUDY --> CLOUDINESS
			        if # .ne. Y, or @ = A, E, I, O, or U,
			                ...@# --> ...@#NESS  as in LATE --> LATENESS
			                                or GRAY --> GRAYNESS
		*/
	unsigned short m_flag : 1;
		/*
			"M" FLAG:
			        ... --> ...'S  as in DOG --> DOG'S
		*/

	unsigned short keep : 1;

};

#define WORDLEN 30

struct hashheader {
	int magic;
	int stringsize;
	int tblsize;
};

#define MAGIC 1

	
/*
 * termcap variables
 */
char *tgetstr();
char PC;	/* padding character */
char *BC;	/* backspace if not ^H */
char *UP;	/* Upline (cursor up) */
char *cd;	/* clear to end of display */
char *ce;	/* clear to end of line */
char *cl;	/* clear display */
char *cm;	/* cursor movement */
char *dc;	/* delete character */
char *dl;	/* delete line */
char *dm;	/* delete mode */
char *ed;	/* exit delete mode */
char *ei;	/* exit insert mode */
char *ho;	/* home */
char *ic;	/* insert character */
char *il;	/* insert line */
char *im;	/* insert mode */
char *ip;	/* insert padding */
char *nd;	/* non-destructive space */
char *vb;	/* visible bell */
char *so;	/* standout */
char *se;	/* standout end */
int bs;
int li, co;	/* lines, columns */

char termcap[1024];
char termstr[1024];	/* for string values */
char *termptr;

char rootword[BUFSIZ];
struct dent *lastdent;

char *hashstrings;


int aflag;
int lflag;

int erasechar;
int killchar;

char tempfile[200];
