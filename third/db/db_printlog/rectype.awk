# $Id: rectype.awk,v 1.1.1.1 2002-02-11 16:24:32 ghudson Exp $
#
# Print out a range of the log
# Command line should set RECTYPE to the record type that is sought.

/^\[/{
	printme = 1
	i = index($1, RECTYPE);
	if (i == 0)
		printme = 0
	else
		printme = 1
}
{
	if (printme == 1)
		print $0
}
