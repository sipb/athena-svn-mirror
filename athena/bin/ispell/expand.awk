BEGIN {FS = "/"}
    {
    print $1
#Let  #  and  @  be  "variables"  that can stand for any letter.  Upper case
#letters are constants.  "..."  stands  for  any  string  of  zero  or  more
#letters,  but note that no word may exist in the dictionary which is not at
#least 2 letters long, so, for example, FLY may not be produced  by  placing
#the  "Y"  flag  on "F".  Also, no flag is effective unless the word that it
#creates is at least 4 letters  long,  so,  for  example,  WED  may  not  be
#produced by placing the "D" flag on "WE".
    size = length ($1)
    #
    # Break out the last two letters into "tail", and put
    # corresponding versions of the root with the tail trimmed
    # off into "trimmed".  If they are vowels, set vowel[i].
    # (Actually, only vowel[2] is used).
    #
    for (i = 1;  i < 3;  i++)
	{
	tail[i] = substr ($1, size - i + 1, 1)
	if (tail[i] == "A"  ||  tail[i] == "E" ||  tail[i] == "I" \
	  ||  tail[i] == "O"  ||  tail[i] == "U")
	    vowel[i] = 1
	else
	    vowel[i] = 0
	trimmed[i] = substr ($1, 1, size - i)
	}
    for (i = 2;  i <= NF;  i++)
	{
	if ($i == "V")
	    {
#		...E --> ...IVE  as in CREATE --> CREATIVE
#		if # .ne. E, ...# --> ...#IVE  as in PREVENT --> PREVENTIVE
	    if (tail[1] == "E")
		print trimmed[1] "IVE"
	    else
		print $1 "IVE"
	    }
	else if ($i == "N"  ||  $i == "X")
	    {
#	        ...E --> ...ION  as in CREATE --> CREATION
#	        ...Y --> ...ICATION  as in MULTIPLY --> MULTIPLICATION
#	        if # .ne. E or Y, ...# --> ...#EN  as in FALL --> FALLEN
#	    "X" flag:
#	        ...E --> ...IONS  as in CREATE --> CREATIONS
#	        ...Y --> ...ICATIONS  as in MULTIPLY --> MULTIPLICATIONS
#	        if # .ne. E or Y, ...# --> ...#ENS  as in WEAK --> WEAKENS
	    if ($i == "N")
		plural = ""
	    else
		plural = "S"
	    if (tail[1] == "E")
		print trimmed[1] "ION" plural
	    else if (tail[1] == "Y")
		print trimmed[1] "ICATION" plural
	    else
		print $1 "EN" plural
	    }
	else if ($i == "H")
	    {
#	        ...Y --> ...IETH  as in TWENTY --> TWENTIETH
#	        if # .ne. Y, ...# --> ...#TH  as in HUNDRED --> HUNDREDTH
	    if (tail[1] == "Y")
		print trimmed[1] "IETH"
	    else
		print $1 "TH"
	    }
	else if ($i == "Y")
	    {
#	        ... --> ...LY  as in QUICK --> QUICKLY
	    print $1 "LY"
	    }
	else if ($i == "G"  ||  $i == "G")
	    {
#	        ...E --> ...ING  as in FILE --> FILING
#	        if # .ne. E, ...# --> ...#ING  as in CROSS --> CROSSING
#	    "J" flag:
#	        ...E --> ...INGS  as in FILE --> FILINGS
#	        if # .ne. E, ...# --> ...#INGS  as in CROSS --> CROSSINGS
	    if ($i == "G")
		plural = ""
	    else
		plural = "S"
	    if (tail[1] == "E")
		print trimmed[1] "ING" plural
	    else
		print $1 "ING" plural
	    }
	else if ($i == "D")
	    {
#	        ...E --> ...ED  as in CREATE --> CREATED
#	        if @ .ne. A, E, I, O, or U,
#	                ...@Y --> ...@IED  as in IMPLY --> IMPLIED
#	        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
#	                ...@# --> ...@#ED  as in CROSS --> CROSSED
#	                                or CONVEY --> CONVEYED
	    if (tail[1] == "E")
		print $1 "D"
	    else if (tail[1] == "Y"  && !vowel[2])
		print trimmed[1] "IED"
	    else
		print $1 "ED"
	    }
	else if ($i == "T")
	    {
#	        ...E --> ...EST  as in LATE --> LATEST
#	        if @ .ne. A, E, I, O, or U,
#	                ...@Y --> ...@IEST  as in DIRTY --> DIRTIEST
#	        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
#	                ...@# --> ...@#EST  as in SMALL --> SMALLEST
#                                or GRAY --> GRAYEST
	    if (tail[1] == "E")
		print $1 "ST"
	    else if (tail[1] == "Y"  &&  !vowel[2])
		print trimmed[1] "IEST"
	    else
		print $1 "EST"
	    }
	else if ($i == "R"  ||  $i == "Z")
	    {
#	        ...E --> ...ER  as in SKATE --> SKATER
#	        if @ .ne. A, E, I, O, or U,
#	                ...@Y --> ...@IER  as in MULTIPLY --> MULTIPLIER
#	        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
#	                ...@# --> ...@#ER  as in BUILD --> BUILDER
#	                                or CONVEY --> CONVEYER
#	   "Z" flag:
#	        ...E --> ...ERS  as in SKATE --> SKATERS
#	        if @ .ne. A, E, I, O, or U,
#	                ...@Y --> ...@IERS  as in MULTIPLY --> MULTIPLIERS
#	        if # .ne. E or Y, or (# = Y and @ = A, E, I, O, or U)
#	                ...@# --> ...@#ERS  as in BUILD --> BUILDERS
#	                                or SLAY --> SLAYERS
	    if ($i == "R")
		plural = ""
	    else
		plural = "S"
	    if (tail[1] == "E")
		print $1 "R" plural
	    else if (tail[1] == "Y"  &&  !vowel[2])
		print trimmed[1] "IER" plural
	    else
		print $1 "ER" plural
	    }
	else if ($i == "S")
	    {
#	        if @ .ne. A, E, I, O, or U,
#	                ...@Y --> ...@IES  as in IMPLY --> IMPLIES
#	        if # .eq. S, X, Z, or H,
#	                ...# --> ...#ES  as in FIX --> FIXES
#	        if # .ne. S, X, Z, H, or Y, or (# = Y and @ = A, E, I, O, or U)
#	                ...@# --> ...@#S  as in BAT --> BATS
#	                                or CONVEY --> CONVEYS
	    if (tail[1] == "Y"  &&  !vowel[2])
		print trimmed[1] "IES"
	    else if (tail[1] == "S")
		print $1 "ES"
	    else
		print $1 "S"
	    }
	else if ($i == "P")
	    {
#	        if @ .ne. A, E, I, O, or U,
#	                ...@Y --> ...@INESS  as in CLOUDY --> CLOUDINESS
#	        if # .ne. Y, or @ = A, E, I, O, or U,
#	                ...@# --> ...@#NESS  as in LATE --> LATENESS
#	                                or GRAY --> GRAYNESS
	    if (tail[1] == "Y"  &&  !vowel[2])
		print trimmed[1] "INESS"
	    else
		print $1 "NESS"
	    }
	else if ($i == "M")
	    {
#	        ... --> ...'S  as in DOG --> DOG'S
		print $1 "'S"
	    }
	}
    }
