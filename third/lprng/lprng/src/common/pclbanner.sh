#!/bin/sh
# shell for PCL banner printing 
#
# Input to the script are lines of the form
# <class>:<user> [Host: <hostname>] [Job: <jobtitle>] [User: <username>]
#  Example:
# A:papowell Host: host1 Job: test file
# The first field is the class:user information.  Other fields can be:
#   User: user name
#   Host: host name
#   Job:  job title name
#
#echo $0 "$@" 1>&2
/usr/bin/awk '
BEGIN{
	xpos = 0;
	ypos = 0;
	incr = 0;
	margins="\033&l0u0Z"
	lightbar="\033*c1800a100b45g2P"
	darkbar="\033*c1800a100b25g2P"
	fontchange="\033(8U\033(s1p%dv0s0b4148T" 
	position= "\033*p%dx%dY"
	UEL="\033%-12345X"
	UELPJL="\033%-12345X@PJL \n"
	PCLRESETSTR="\033E"
	CRLFSTR="\033&k2G"
}
function moveto( x, y ){
	printf position, x, y ;
}
function fontsize( size ){
	incr = (size*300*1.1)/72;
	printf fontchange, size;
}
function outline(s){
	printf "%s", s
}
function argline(key,value){
	if( value != "" ){
		textline( key , 1, 0 );
		textline( value, 0, 1 );
	}
}

function textline( line, start, end ){
	if( start ){
		moveto( xpos, ypos );
	}
	printf "%s", line
	if( end ){
		ypos += incr;
	}
}
function pcl_banner(){
    outline(UEL);
    outline(PCLRESETSTR);
    outline(UELPJL);
	outline(CRLFSTR);
	outline(margins);

	# do light bar 
	xpos = 0; ypos = 0;
	moveto( xpos, ypos );
	outline( lightbar );
	ypos += 100;

	# set font size 
	fontsize( 24 );
	ypos += incr;
	moveto( xpos, ypos );

	for( key in arg ){
		argline(key,arg[key])
	}

	# smaller font 
	fontsize( 12 );

	moveto(xpos,ypos);
	"date" | getline date;
	textline( "Date: ", 0, 1 );
	textline( date, 0, 1 );
	
	moveto( xpos, ypos );
	outline( darkbar );

	outline(FFEED);
    outline(UEL);
    outline(PCLRESETSTR);
} 
{
	# line is broken up at word:
	line = $0;
	# print "BANNER" $0 >/dev/stderr
	/* skip blank or bad formatted lines */
	firstentry = 0;
	while( line != "" ){
		#printf "Line \"%s\"\n",line
		p = match( line, /^[ \t][ \t]*/);
		while( p ){
			#printf "p %d RSTART %d\n",p, RSTART;
			line = substr(line,RSTART+p);
			p = match( line, /^[ \t][ \t]*/);
		}
		p = match( line, /^[A-Za-z][A-Za-z]*:/ );
		if( p == 0 ) break;
		key = substr(line,RSTART,RLENGTH-1);
		line = substr(line,RSTART+RLENGTH);
		#printf "Key \"%s\" Line \"%s\"\n",key,line
		# get the next colon
		p = match( line, /[A-Za-z][A-Za-z]*:/ );
		if( p == 0 ){
			value = line;
			line = "";
		} else {
			# back up from the colon
			value = substr(line,1,RSTART-1);
			line = substr(line,RSTART);
		}
		lkey = tolower(key);
		arg[lkey] = " " value;
		if( firstentry == 0 && arg["user"] == "" && arg["host"] == "" && arg["job"] == "" ){
			arg["class:"] = " " key;
			arg["user:"] = " " value;
			arg[lkey] = "";
		}
		firstentry = 1;
		#printf "Key \"%s\" value \"%s\" line\n",key,value
	}
	pcl_banner();
	quit;
}
'
