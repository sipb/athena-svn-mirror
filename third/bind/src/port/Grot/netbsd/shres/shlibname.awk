BEGIN {
	FS = ".";
	MAJOR = 0;
	MINOR = 0;
	CUSTOM = 0;
}

NF <= 5 {
	if ($3 > MAJOR)
		MAJOR = $3;
	if ($4 > MINOR)
		MINOR = $4;
	if ($5 > CUSTOM)
		CUSTOM = $5;
}

END {
	CUSTOM = CUSTOM + 1;
	printf("%d.%d.%d", MAJOR, MINOR, CUSTOM);
}
