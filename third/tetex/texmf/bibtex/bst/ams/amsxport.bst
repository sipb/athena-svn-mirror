%%
%% This is file `amsxport.bst',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% amsxport.dtx  (with options: `bst')
%% 
%%% ====================================================================
%%% @BibTeX-source-file{
%%%   filename  = "amsxport.dtx",
%%%   version   = "1.05",
%%%   date      = "2002/01/30",
%%%   time      = "15:16:33 EST",
%%%   author    = "Michael J Downes",
%%%   address   = "American Mathematical Society,
%%%                Publications Technical Group,
%%%                PO Box 6248,
%%%                Providence, RI 02940,
%%%                USA",
%%%   email     = "tech-support at ams.org",
%%%   URL       = "http://www.ams.org/",
%%%   abstract  = "The BibTeX style file amsxport.bst produces .bbl
%%%                entries where most of the database structure is
%%%                retained. This is intended for processing by a smart
%%%                bibliography package such as the amsrefs package.",
%%%   checksum  = "07988 1902 7374 54746",
%%%   docstring = "The checksum field, produced by Robert Solovay's
%%%                checksum utility, gives CRC-16 checksum, lines,
%%%                words, and characters.",
%%%  }
%%% ====================================================================

ENTRY
  {
    address      institution  number       status
    archive      isbn         organization subtitle
    author       issn         pages        title
    booktitle    journal      part         translator
    chapter      key          pii          type
    date         language     preprint     url
    edition      meeting      publisher    volume
    editor       month        review       xid
    eprint       mrnumber     school       year
    howpublished note         series
  }
  { first.such.label }
  { label }
MACRO {jan} {"-01"}
MACRO {feb} {"-02"}
MACRO {mar} {"-03"}
MACRO {apr} {"-04"}
MACRO {may} {"-05"}
MACRO {jun} {"-06"}
MACRO {jul} {"-07"}
MACRO {aug} {"-08"}
MACRO {sep} {"-09"}
MACRO {oct} {"-10"}
MACRO {nov} {"-11"}
MACRO {dec} {"-12"}
STRINGS { s t options }
INTEGERS { verbose.flag }
FUNCTION {NOT} { { #0 } { #1 } if$ }

FUNCTION {AND} { 'skip$ { pop$ #0 } if$ }

FUNCTION {OR} { { pop$ #1 } 'skip$ if$ }

FUNCTION {TRUE} { #1 }

FUNCTION {FALSE} { #0 }

FUNCTION {verbose.msg} { verbose.flag 'top$ 'pop$ if$ }

FUNCTION {field.or.null}
{ duplicate$ empty$
    { pop$ "" }
    'skip$
  if$
}
FUNCTION {missing.or.empty} { duplicate$ missing$ swap$ empty$ OR }

INTEGERS { len ptr }

FUNCTION {current.char} {ptr #1 substring$}

FUNCTION {incr.ptr} {ptr #1 + 'ptr :=}

FUNCTION {copy.two.chars} {t * incr.ptr s current.char * incr.ptr }

FUNCTION {is.digit}
{ chr.to.int$ duplicate$ "0" chr.to.int$ #1 - >
  swap$ "9" chr.to.int$ #1 + <
  AND
}

FUNCTION {is.lowercase.letter}
{ chr.to.int$ duplicate$ "a" chr.to.int$ #1 - >
  swap$ "z" chr.to.int$ #1 + <
  AND
}

FUNCTION {is.uppercase.letter}
{ chr.to.int$ duplicate$ "A" chr.to.int$ #1 - >
  swap$ "Z" chr.to.int$ #1 + <
  AND
}

FUNCTION {is.letter}
{ duplicate$
  is.lowercase.letter
    { pop$ TRUE }
    { is.uppercase.letter }
  if$
}
FUNCTION {is.nonletter}
{ chr.to.int$
  duplicate$ "z" chr.to.int$ >
    { pop$ TRUE }
    { duplicate$ "A" chr.to.int$ <
        { pop$ TRUE }
        { duplicate$ "Z" chr.to.int$ >
          swap$ "a" chr.to.int$ <
          AND
        }
      if$
    }
  if$
}
FUNCTION {is.lowercase} { duplicate$ "l" change.case$ = }

FUNCTION {append.ndash.and.skip.hyphens}
{ "\ndash " * incr.ptr
  { s current.char "-" = }
    { incr.ptr }
  while$
}
FUNCTION {n.dashify}
{ 's :=   #1 'ptr :=
  ""
  { s current.char 't :=  t "" = NOT }
    { t "\" =
        { copy.two.chars }
        { t "-" =
            { append.ndash.and.skip.hyphens }
            { t * incr.ptr }
          if$
        }
      if$
    }
  while$
}
FUNCTION {missing.warning}
{ "missing " swap$ * " in " * cite$ * warning$ }

INTEGERS { string.ptr next.subscript}

STRINGS { label.list label.duplicates }

FUNCTION {string.length}
{ #1 'string.ptr :=
  { duplicate$ string.ptr #1 substring$ "" = 'FALSE 'TRUE if$ }
    { string.ptr #1 + 'string.ptr := }
  while$
  pop$ string.ptr #1 -
}

FUNCTION {chr.to.number} { chr.to.int$ "0" chr.to.int$ - }

FUNCTION {int.to.va}
{ #32 +
  duplicate$ "~" chr.to.int$ >
    { "Number too large for visible-ascii conversion" warning$
      pop$ ""
    }
    { int.to.chr$ }
  if$
}

FUNCTION {va.to.int} { chr.to.int$ #32 - }
STRINGS { p }
INTEGERS { brace.level }
FUNCTION {copy.cs}
{ incr.ptr
  s ptr #1 substring$ is.letter
    { incr.ptr
      { s ptr #1 substring$ is.letter }
        'incr.ptr
      while$
      { s ptr #1 substring$ " " = }
        'incr.ptr
      while$
    }
    { incr.ptr }
  if$
}
FUNCTION {copy.arg}
{ brace.level #1 + 'brace.level :=
  incr.ptr
  { brace.level #0 > ptr len < AND }
    { s ptr #1 substring$ 'p :=
      p "{" =
        { brace.level #1 + 'brace.level :=  incr.ptr }
        { p "}" =
            { brace.level #1 - 'brace.level :=  incr.ptr}
            { p "\" =
                { copy.cs }
                { incr.ptr }
              if$
            }
          if$
        }
      if$
    }
  while$
}
FUNCTION {process.cs}
{ copy.cs
  { s ptr #1 substring$ "{" = }
    { copy.arg }
  while$
}
FUNCTION {drop.a.brace}
{ s #1 ptr #1 - substring$
  s ptr #1 + global.max$ substring$ *
  's :=
}
FUNCTION {brace.group}
{ s ptr #1 + #1 substring$ "\" =
    'copy.arg
    'drop.a.brace
  if$
}
FUNCTION {remove.braces}
{ 's :=
  s string.length 'len :=
  #1 'ptr :=
  { ptr len < }
    { s ptr #1 substring$ 'p :=
      p "{" =
        { brace.group }
        { p "}" =
            { drop.a.brace }
            { p "\" =
                'process.cs
                { p "^" = p "_" = OR
                    { ptr #1 - 'ptr := process.cs }
                    'incr.ptr
                  if$
                }
              if$
            }
          if$
        }
      if$
    }
  while$
  s
}
FUNCTION {downcase.title}
{ "t" change.case$ remove.braces }
FUNCTION {start.field}
{ duplicate$ text.length$ 'len :=
  len #10 <
    { "          " #1 #10 len - substring$ swap$ * "={" * write$ }
    { "={" * write$ }
  if$
}

FUNCTION {fin.field} { "}," write$ newline$ }

FUNCTION {write.field}
{ duplicate$ missing$
    { pop$ missing.warning }
    { duplicate$ empty$
        { pop$ missing.warning }
        { swap$ start.field write$ fin.field }
      if$
    }
  if$
}
FUNCTION {optional.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { swap$ start.field write$ fin.field }
  if$
}
FUNCTION {optional.title.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { downcase.title swap$ start.field write$ fin.field }
  if$
}
FUNCTION {optional.pages.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { swap$ start.field
      n.dashify
      write$ fin.field
    }
  if$
}
FUNCTION {optional.mr.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { swap$ start.field
      "\MR{" swap$ * "}" * write$ fin.field }
  if$
}
FUNCTION {remove.ordinal.suffix}
{ 's :=
  s string.length 'len :=
  #1 'ptr :=
  { ptr len < s ptr #1 substring$ is.digit AND }
    'incr.ptr
  while$
  s ptr global.max$ substring$ 'p :=
  p "st" =
    { TRUE }
    { p "nd" =
      { TRUE }
      { p "rd" =
        { TRUE }
        { p "th" =
          { TRUE }
          { FALSE }
          if$
        } if$
      } if$
    } if$
    { s #1 ptr #1 - substring$ }
    { s }
  if$
}
FUNCTION {optional.edition.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { swap$ start.field
      remove.ordinal.suffix
      write$ fin.field }
  if$
}
INTEGERS { numnames namesleft }
STRINGS { namelist fieldname }
FUNCTION {format.name} { "{vv~}{ll}{, ff}{, jj}" format.name$ }

FUNCTION {optional.name.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { swap$ 'fieldname :=
      duplicate$ num.names$
      'namesleft :=
      #1 'ptr :=
      { namesleft #0 > }
      { fieldname start.field
        duplicate$ ptr format.name write$
        fin.field
        ptr #1 + 'ptr :=
        namesleft #1 - 'namesleft :=
      }
      while$
      pop$
    }
  if$
}
FUNCTION {assemble.date.field}
{ date missing$
    { year missing.or.empty
        { status missing.or.empty
            { "No year or other date information for "
               cite$ * warning$ }
            'skip$
          if$
          "status"
        }
        { year month missing$ { "" } { month } if$ * }
      if$
    }
    { date }
  if$
  duplicate$ "status" =
    { pop$ pop$ }
    { swap$ start.field write$ fin.field }
  if$
}
FUNCTION {optional.journal.field}
{ duplicate$ missing.or.empty
    { pop$ pop$ }
    { 's :=
      start.field
      #1 'ptr :=
      s current.char duplicate$ is.lowercase swap$ is.letter AND
        { s "/" * 's :=
          { s ptr #1 substring$ "/" = NOT }
            { incr.ptr }
          while$
          s ptr #1 + #1 substring$ empty$
            { s #1 ptr #1 - substring$ }
            { s #1 ptr #1 - substring$
              write$ fin.field
              "ISSN" start.field
              s ptr #1 + #9 substring$
            }
          if$
        }
        { s }
      if$
      write$ fin.field
    }
  if$
}
FUNCTION {optional.type.field}
{ duplicate$ missing.or.empty
    { type$ "mastersthesis" = { "Master's Thesis" }
        { type$ "phdthesis" = { "Ph.D. Thesis" }
          { "" } if$ } if$
      swap$ pop$
    }
    'skip$
  if$
  duplicate$ empty$
    { pop$ pop$ }
    { swap$ start.field write$ fin.field }
  if$
}
FUNCTION {url.note.status.review}
{ "url" url optional.field
  "note" note optional.field
  "status" status optional.field
  "review" review optional.field
  "review" mrnumber optional.mr.field
}
INTEGERS { option-initials }

FUNCTION {start.entry}
{ newline$
  "\bib{" cite$ * "}{" *
  type$ "mastersthesis" =   type$ "phdthesis" =  OR
    { "thesis" }
    { type$ }
  if$
  * "}{" * write$
  newline$
}
FUNCTION {fin.entry}
{ "}" write$
  newline$
}
FUNCTION {bst-option} { }

FUNCTION {article}
{ start.entry
  "author" author optional.name.field
  "translator" translator optional.name.field
  "title" title downcase.title write.field
  "subtitle" subtitle optional.title.field
  "language" language optional.field
  "organization" organization optional.field
  "how" howpublished optional.field
  "date" assemble.date.field
  "ISSN" issn optional.field
  "journal" journal optional.journal.field
  "volume" volume optional.field
  "number" number optional.field
  "pages" pages optional.pages.field
  "PII" pii optional.field
  "archive" archive optional.field
  "eprint" eprint optional.field
  "preprint" preprint optional.field
  url.note.status.review
  fin.entry
}
FUNCTION {inproceedings}
{ start.entry
  "author" author optional.name.field
  "translator" translator optional.name.field
  "title" title downcase.title write.field
  "subtitle" subtitle optional.title.field
  "language" language optional.field
  "organization" organization optional.field
  "how" howpublished optional.field
  "date" assemble.date.field
  "xid" xid optional.field
  "conference" meeting optional.field
  "booktitle" booktitle optional.title.field
  "edition" edition optional.edition.field
  "editor" editor optional.name.field
  "series" series optional.field
  "volume" volume optional.field
  "publisher" publisher optional.field
  "address" address optional.field
  "pages" pages optional.pages.field
  url.note.status.review
  fin.entry
}

FUNCTION {inbook} { inproceedings }

FUNCTION {incollection} { inproceedings }

FUNCTION {conference} { inproceedings }

FUNCTION {book}
{ start.entry
  "author" author optional.name.field
  "editor" editor optional.name.field
  "translator" translator optional.name.field
  "title" title missing$ { booktitle } { title } if$
     downcase.title write.field
  "subtitle" subtitle optional.title.field
  "type" type optional.type.field
  "language" language optional.field
  "conference" meeting optional.field
  "edition" edition optional.edition.field
  "series" series optional.field
  "publisher" publisher optional.field
  "organization" organization optional.field
  "institution" institution optional.field
  "address" address optional.field
  "how" howpublished optional.field
  "date" year write.field
  "volume" volume optional.field
  "number" number optional.field
  "ISBN" isbn optional.field
  url.note.status.review
  fin.entry
}

FUNCTION {booklet} { book }

FUNCTION {manual} { book }

FUNCTION {mastersthesis} { book }

FUNCTION {phdthesis} { book }

FUNCTION {proceedings} { book }

FUNCTION {collection} { book }

FUNCTION {techreport} { book }

FUNCTION {unpublished} { book }

FUNCTION {misc} { book }

FUNCTION {default.type} { misc }


READ

FUNCTION {sortify}
{ purify$
  "l" change.case$
}

FUNCTION {chop.word}
{ 's :=
  'len :=
  s #1 len substring$ =
    { s len #1 + global.max$ substring$ }
    's
  if$
}
FUNCTION {sort.format.title}
{ 't :=
  "A " #2
    "An " #3
      "The " #4 t chop.word
    chop.word
  chop.word
  sortify
  #1 global.max$ substring$
}
FUNCTION {update.label}
{ label
  s ptr "{l}" format.name$
  * 'label :=
}
FUNCTION {skip.accent}
{ pop$ incr.ptr }
FUNCTION {strip.label}
{ #1 'ptr :=
  label 's :=
  ""
  { s ptr #1 substring$ duplicate$ empty$ NOT }
    { duplicate$ is.letter
        { * }
        { duplicate$ "\" =
            { skip.accent }
            'pop$
          if$
        }
      if$
      incr.ptr
    }
  while$
  pop$
  'label :=
}
FUNCTION {sort.format.names}
{ 's :=
  #1 'ptr :=
  "" 'label :=
  ""
  s num.names$ 'numnames :=
  numnames 'namesleft :=
    { namesleft #0 > }
    { ptr #1 >
        { "   " * }
        'skip$
      if$
      s ptr "{ll}{ ff}{ jj}" format.name$ 't :=
      ptr numnames = t "others" = AND
        { "et al" * }
        { t sortify * }
      if$
      update.label
      ptr #1 + 'ptr :=
      namesleft #1 - 'namesleft :=
    }
  while$
  strip.label
}
FUNCTION {sort.warning}
{ "to sort, need " swap$ * " or key in " * cite$ * warning$
}
FUNCTION {author.sort}
{ author empty$
    { key empty$
        { "author" sort.warning
          ""
        }
        { key sortify }
      if$
    }
    { author sort.format.names }
  if$
}
FUNCTION {author.editor.sort}
{ author empty$
    { editor empty$
        { key empty$
            { "author, editor," sort.warning
              ""
            }
            { key sortify }
          if$
        }
        { editor sort.format.names }
      if$
    }
    { author sort.format.names }
  if$
}
FUNCTION {author.organization.sort}
{ author empty$
    { organization empty$
        { key empty$
            { "author, organization," sort.warning
              ""
            }
            { key sortify }
          if$
        }
        { "The " #4 organization chop.word sortify }
      if$
    }
    { author sort.format.names }
  if$
}
FUNCTION {editor.organization.sort}
{ editor empty$
    { organization empty$
        { key empty$
            { "editor, organization," sort.warning
              ""
            }
            { key sortify }
          if$
        }
        { "The " #4 organization chop.word sortify }
      if$
    }
    { editor sort.format.names }
  if$
}
FUNCTION {normal.presort}
{ type$ "book" = type$ "inbook" = OR type$ "collection" = OR
    'author.editor.sort
    { type$ "proceedings" =
        'editor.organization.sort
        { type$ "manual" =
            'author.organization.sort
            'author.sort
          if$
        }
      if$
    }
  if$
  "    " *
  year field.or.null sortify *
  "    " *
  title field.or.null sort.format.title *
  #1 entry.max$ substring$
  'sort.key$ :=
}
FUNCTION {set.option}
{ 's :=
  s "[citation-star]" =
    { "." options * 'options := }
    { options #1 #1 substring$ "." =
        'skip$
        { s "[citation-order]" =
            { options "C" *  'options := }
            { s "[verbose]" =
                { #1 'verbose.flag := }
                { "Unknown option: " s * warning$ }
  if$} if$} if$} if$
}
FUNCTION {option.citation.order} { "C" options = }

FUNCTION {presort}
{ type$ "bst-option" =
    { cite$ set.option "~~~~" 'sort.key$ := }
    'normal.presort
  if$
}
FUNCTION {initialize.options}
{ "" 'options :=
  #0 'verbose.flag :=
}
EXECUTE {initialize.options}

ITERATE {presort}

SORT




FUNCTION {initialize.for.labels}
{ "" 'label.list :=
  "" 'label.duplicates :=
}
FUNCTION {add.to.label.list} {
  swap$ int.to.str$ swap$ * " " * label.list * 'label.list :=
  TRUE 'first.such.label :=
}

FUNCTION {label.list.first.part} { label.list #1 ptr substring$ }

FUNCTION {label.list.last.part}
{ label.list ptr #2 + len + global.max$ substring$ }
FUNCTION {do.duplicate.label}
{ 's :=  'ptr :=  'len :=
  label.list.first.part
  s *
  label.list ptr #1 + len + #1 substring$
  va.to.int #1 + 'next.subscript :=
  next.subscript int.to.va *
  label.list.last.part * 'label.list :=
  next.subscript #1 =
    { len int.to.str$ s * label.duplicates * 'label.duplicates :=
    }
    'skip$
  if$
  label "~" * next.subscript #1 + int.to.str$ * 'label :=
  FALSE 'first.such.label :=
}

FUNCTION {next.length} { label.list ptr #1 substring$ }

FUNCTION {next.label} { label.list ptr #1 + len substring$ }

FUNCTION {advance.label.list.ptr}
{ ptr len + #2 + 'ptr := }

FUNCTION {exit.while} { global.max$ 'ptr := }
FUNCTION {index.in.label.list}
{ 's :=
  #0 s string.length
  label.list empty$
    'skip$
    { #1 'ptr :=
      { next.length  duplicate$ empty$ NOT }
        { chr.to.number 'len :=
          next.label s =
            { pop$ pop$
              ptr len  exit.while }
            { advance.label.list.ptr }
          if$
        }
      while$
      pop$
    }
  if$
}

FUNCTION {label.pass}
{ label empty$
    { "??" 'label :=
      type$ "bst-option" =
        'skip$
        { "Preliminary label is empty in " cite$ * warning$ }
      if$
    }
    { label index.in.label.list  swap$ duplicate$ #0 =
        { pop$  label add.to.label.list }
        { label do.duplicate.label }
      if$
    }
  if$
  type$ "bst-option" =
    'skip$
    { "Label = [" label * "]" * verbose.msg }
  if$
}

EXECUTE {initialize.for.labels}

ITERATE {label.pass}

FUNCTION {reverse.label.pass}
{ first.such.label
    { label pop$ }
    'skip$
  if$
}

ITERATE {reverse.label.pass}
FUNCTION {write.with.newlines}
{ 's :=  #1 'ptr :=
  { s ptr #2 + #1 substring$ "" = NOT }
  { s ptr #3 substring$ "^^M" =
    { s #1 ptr #1 - substring$ write$ newline$
      ptr #3 + 'ptr :=
      s ptr global.max$ substring$ 's :=
      #1 'ptr :=
    }
    { incr.ptr }
    if$
  }
  while$
  s write$ newline$
}
FUNCTION {begin.bib}
{ preamble$ empty$
    'skip$
    { preamble$ write.with.newlines }
  if$
  "% \bib, bibdiv, biblist are defined by the amsrefs package."
     write$ newline$
  "\begin{bibdiv}" write$ newline$
  "\begin{biblist}" write$ newline$
}

FUNCTION {end.bib}
{ newline$
  "\end{biblist}" write$ newline$
  "\end{bibdiv}" write$ newline$
}
EXECUTE {begin.bib}

ITERATE {call.type$}

EXECUTE {end.bib}
%% 
%%
%% End of file `amsxport.bst'.
