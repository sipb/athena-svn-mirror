# $Id: count.awk,v 1.1.1.1 2004-12-17 17:26:54 ghudson Exp $
#
# Print out the number of log records for transactions that we
# encountered.

/^\[/{
	if ($5 != 0)
		print $5
}
