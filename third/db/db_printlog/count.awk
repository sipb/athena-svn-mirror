# $Id: count.awk,v 1.1.1.2 2002-02-11 16:24:32 ghudson Exp $
#
# Print out the number of log records for transactions that we
# encountered.

/^\[/{
	if ($5 != 0)
		print $5
}
