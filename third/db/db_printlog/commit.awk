# $Id: commit.awk,v 1.1.1.2 2002-02-11 16:24:32 ghudson Exp $
#
# Output tid of committed transactions.

/txn_regop/ {
	print $5
}
