
#define FF 0
#define FI 1
#define FL 2
#define FFI 3
#define FFL 4

int whichligs[5];

#define MAXCHARS 5000

struct charmetric {
    int code;
    double width;
    char name[100];
    double bbllx;
    double bblly;
    double bburx;
    double bbury;
} characters[MAXCHARS];

int ncharacters = 0;

#define NMATHONLY 2

struct mathentry {
    char shortname[10];
    char longname[20];
} mathonly[] =
{
    { "pl", "plus" },
    { "eq", "equal"}
};

#define NPROC 14

struct procentry {
    char name[5];
    int ccode;
    int kerncode;
    float width;
    int special;
} proc[] =
{
    { "13", 136, 0, 833, 0},	/* 1/3 */
    { "23", 137, 0, 833, 0},	/* 2/3 */
    { "14", 129, 0, 833, 0},	/* 1/4 */
    { "34", 131, 0, 833, 0},	/* 3/4 */
    { "18", 132, 0, 833, 0},	/* 1/8 */
    { "38", 133, 0, 833, 0},	/* 3/8 */
    { "58", 134, 0, 833, 0},	/* 5/8 */
    { "78", 135, 0, 833, 0},	/* 7/8 */
    { "12", 130, 0, 833, 0},	/* 1/2 */
/*  { "mi", 138, 0, 549, 1}, */   /* minus */
    { "<-", 172, 0, 987, 1},	/* arrow left */
    { "->", 174, 0, 987, 1},	/* arrow right */
    { "is", 242, 3, 274, 1},	/* integral */
    { "==", 186, 0, 549, 1},	/* equivalence */
    { "sr", 214, 0, 549, 1}	/* square root */
};

#define NASCII 93

char *asciitable[NASCII] =
{
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "a",
    "ampersand",
    "asterisk",
    "at",
    "b",
    "backslash",
    "bar",
    "braceleft",
    "braceright",
    "bracketleft",
    "bracketright",
    "c",
    "colon",
    "comma",
    "d",
    "dollar",
    "e",
    "eight",
    "equal",
    "exclam",
    "f",
    "five",
    "four",
    "g",
    "greater",
    "h",
    "hyphen",
    "i",
    "j",
    "k",
    "l",
    "less",
    "m",
    "n",
    "nine",
    "numbersign",
    "o",
    "one",
    "p",
    "parenleft",
    "parenright",
    "percent",
    "period",
    "plus",
    "q",
    "question",
    "quotedbl",
    "quoteleft",
    "quoteright",
    "r",
    "s",
    "semicolon",
    "seven",
    "six",
    "slash",
    "space",
    "t",
    "three",
    "two",
    "u",
    "underscore",
    "v",
    "w",
    "x",
    "y",
    "z",
    "zero"
};




#define NDITMAP 173

struct ditmapentry {
    char *pname;
    char *dnames;
} ditmap[NDITMAP] =
{
    { "AE", "AE" },
    { "Alpha", "*A" },
    { "Beta", "*B" },
    { "Chi", "*X" },
    { "Delta", "*D" },
    { "Epsilon", "*E" },
    { "Eta", "*Y" },
    { "Gamma", "*G" },
    { "Iota", "*I" },
    { "Kappa", "*K" },
    { "Lambda", "*L" },
    { "Lslash", "PL" },
    { "Mu", "*M" },
    { "Nu", "*N" },
    { "OE", "OE" },
    { "Omega", "*W" },
    { "Omicron", "*O" },
    { "Oslash", "O/" },
    { "Phi", "*F" },
    { "Pi", "*P" },
    { "Psi", "*Q" },
    { "Rho", "*R" },
    { "Sigma", "*S" },
    { "Tau", "*T" },
    { "Theta", "*H" },
    { "Upsilon", "*U" },
    { "Xi", "*C" },
    { "Zeta", "*Z" },
    { "acute", "aa \\'" },
    { "ae", "ae" },
    { "aleph", "al" },
    { "alpha", "*a" },
    { "angleleft", "l<" },
    { "angleright", "r>" },
    { "approxequal", "~=" },
    { "arrowboth", "<>" },
    { "arrowdblboth", "io" },
    { "arrowdblleft", "<: lh" }, /* left double arrow (& hand) */
    { "arrowdblright", ":> im rh" }, /* right double arrow (& hand) */
    { "arrowdown", "da" },
    /*  { "arrowleft", "<-" }, *//* see procs */
    /* { "arrowright", "->" }, *//* see procs */
    { "arrowup", "ua" },
    { "asteriskmath", "**" },
    { "bar", "or" },
    { "beta", "*b" },
    { "br", "br" },		/* box rule */
    { "breve", "be" },
    { "bu", "bu" },		/* bullet */
    { "bv", "bv" },		/* bold vertical */
    { "bx", "bx" },		/* box */
    { "caron", "hc" },
    { "carriagereturn", "cr" },
    { "cedilla", "cd" },
    { "cent", "ct" },
    { "chi", "*x" },
    { "ci", "ci" },		/* circle */
    { "circlemultiply", "ax" },
    { "circleplus", "a+" },
    { "circumflex", "^" },	/* see ascii */
    { "copyrightserif", "co" },
    { "dagger", "dg" },
    { "daggerdbl", "dd" },
    { "degree", "de" },
    { "delta", "*d" },
    { "diamond", "dm" },
    { "dieresis", "um .." },	/* umlaut */ 
    { "divide", "di" }, 
    { "dotaccent", "dt" },
    { "dotlessi", "ui" },
    { "dotmath", "m." },
    { "element", "mo cm" },
    { "emdash", "em" },
    { "emptyset", "es" },
    { "endash", "en" },
    { "epsilon", "*e" },
    { "equal", "eq" },
    /* { "equivalence", "==" }, *//* see procs */
    { "eta", "*y" },
    { "exclamdown", "!! I!" },
    { "existential", "te" },
    { "ff", "ff" },
    { "ffi", "Fi" },
    { "ffl", "Fl" },
    { "fi", "fi" },
    { "fl", "fl" },
    { "florin", "$D" },
    { "gamma", "*g" },
    { "germandbls", "ss" },
    { "gradient", "gr" },
    { "grave", "ga \\`" },
    { "greaterequal", ">=" },
    { "guillemotleft", "d<" },
    { "guillemotright", "d>" },
    { "heart", "bs" },		/* bell system logo */
    { "hyphen", "hy" },
    { "infinity", "if" },
    /* { "integral", "is" }, *//* see procs */
    { "intersection", "ca" },
    { "iota", "*i" },
    { "kappa", "*k" },
    { "lambda", "*l" },
    { "lb", "lb" },		/* left bot curly */
    { "lc", "lc" },		/* left ceil */
    { "lessequal", "<=" },
    { "lf", "lf" },		/* left floor */
    { "lk", "lk" },		/* left center curly */
    { "logicaland", "an la" },
    { "logicalnot", "no" },
    { "logicalor", "lo" },
    { "lslash", "Pl" },
    { "lt", "lt" },		/* left top curly */
    { "macron", "mc ma" },
    { "minus", "\\- mi" },
    { "minute", "fm mt" },
    { "mu", "*m" },
    { "multiply", "mu" },
    { "notelement", "!m" },
    { "notequal", "!=" },
    { "notsubset", "!s" },
    { "nu", "*n" },
    { "ob", "ob" },		/* outline bullet */
    { "oe", "oe" },
    { "ogonek", "og" },
    { "omega", "*w" },
    { "omicron", "*o" },
    { "oslash", "o/" },
    { "paragraph", "pp" },
    { "partialdiff", "pd" },
    { "perpendicular", "bt" },
    { "perthousand", "pm" },
    { "phi", "*f" },
    { "pi", "*p" },
    { "plus", "pl" },
    { "plusminus", "+-" },
    { "propersubset", "sb" },
    { "propersuperset", "sp" },
    { "proportional", "pt" },
    { "psi", "*q" },
    { "questiondown", "?? I?" },
    { "quotedblleft", "lq" },
    { "quotedblright", "rq" },
    { "quotesingle", "n'" },
    /* { "radical", "sr" }, *//* see procs */
    { "rb", "rb" },		/* right bot curly */
    { "rc", "rc" },		/* right ceil */
    { "reflexsubset", "ib" },
    { "reflexsuperset", "ip" },
    { "registerserif", "rg" },
    { "rf", "rf" },		/* right floor */
    { "rho", "*r" },
    { "ring", "ri" },
    { "rk", "rk" },		/* right center curly */
    { "rn", "rn" },		/* root extender */
    { "rt", "rt" },		/* rith top curly */
    { "ru", "ru" },		/* rule */
    { "second", "sd" },
    { "section", "sc" },
    { "sigma", "*s" },
    { "sigma1", "ts" },
    { "similar", "ap" },
    { "slash", "sl" },
    { "sq", "sq" },		/* square */
    { "sterling", "ps po" },
    { "tau", "*t" },
    { "therefore", "tf" },
    { "theta", "*h" },
    { "tilde", "~" },		/* see ascii */
    { "trademarkserif", "tm" },
    { "ul", "ul" },		/* under rule */
    { "underscore", "\\_" },
    { "union", "cu" },
    { "universal", "fa" },
    { "upsilon", "*u" },
    { "vr", "vr" },		/* vertical rule */
    { "xi", "*c" },
    { "yen", "yi yn $J" },
    { "zeta", "*z" },
};






