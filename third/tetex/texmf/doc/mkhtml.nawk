#!/bin/sh

: ${MAKE=make}

echo '<TITLE>The TeX System</TITLE>'
echo '<H1>The TeX System</H1>'

for i in *; do
	if [ $i = help ]; then
		echo "<h2>$i</h2><P>"
		echo '<ul>'
		for j in help/*; do
			if [ -f $j/index.html ]; then
				echo "<li><a href=\"$j/index.html\">`basename $j`</a>"
			else
				echo "<li><a href=\"$j\">`basename $j`</a>"
			fi
		done
		echo '</ul>'
		continue
	fi
	if [ -d $i ]; then
		echo "<h2>$i</h2><P>"
		echo '<ul>'
		for j in $i/*.html $i/*.html.gz $i/*.dvi $i/*.ps $i/*.tex $i/*.dvi.gz $i/*.ps.gz $i/*.tex.gz $i/*.pdf; do
			test -f $j && echo "<li><a href=\"$j\">`basename $j`</a>"
		done
		for j in $i/*; do
			test -d $j || continue
			test -f `echo $j/*.html | sed 's/ .*//'` ||
			test -f `echo $j/*.html.gz | sed 's/ .*//'` ||
			test -f `echo $j/*.ps.gz | sed 's/ .*//'` ||
			test -f `echo $j/*.dvi.gz | sed 's/ .*//'` ||
			test -f `echo $j/*.dvi | sed 's/ .*//'` ||
			test -f `echo $j/*.pdf | sed 's/ .*//'` ||
			test -f `echo $j/*.ps | sed 's/ .*//'` || continue
			echo '<li>'`basename $j`
			echo '<ul>'
			for k in $i/*.html $i/*.html.gz $j/*.dvi $j/*.ps $j/*.tex $j/*.dvi.gz $j/*.ps.gz $j/*.tex.gz $j/*.pdf $j/Seminar-Bugs.html $j/Seminar-FAQ.html; do
				test -f $k || continue
				echo "<li><a href=\"$k\">`basename $k`</a>"
			done
			echo '</ul>'
		done
		echo '</ul>'
	fi
done | awk '
initializing { nf = split($1,nm," ");
        $1 = "";
        nfa[nm[1]] = nf;
        if (nf) {
          helps[nm[1]] = $0;
          all[nm[1],1] = nm[1];
          for (i=2; i<=nf; ++i) {
            if (!(nm[i] in helps))
              helps[nm[i]] = "";
            all[nm[1],i] = nm[i];
          }
        }
        next
}

/^<li><a href=[^>]*>[^<>]*<\/a>$/ {
        str = $0;
        sub(/^<li><a href=[^>]*>/,"",str);
        sub(/<.*/,"",str);
        if (str in helps) {
          hstr = helps[str];
          sub (/^<li>/,"", $0);
          nf = nfa[str];
          for (i=1; i<=nf; ++i) {
            arg = $0;
            gsub(str, all[str,i], arg);
            gsub(all[str,i], arg, hstr);
          }
          print hstr;
        } else print;
        next;
}
{ print $0 }
' initializing=1 RS="" FS="\n" OFS="\n" helpfile initializing=0 RS="\n" FS=" " OFS=" " -

test -w help/Catalogue/ctfull.html \
  && (cd help/Catalogue; $MAKE) >&2

exit 0
