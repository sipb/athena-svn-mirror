# $Id: commit.awk,v 1.1.1.1 2004-12-17 17:26:54 ghudson Exp $
#
# Output tid of committed transactions.

/txn_regop/ {
	print $5
}
