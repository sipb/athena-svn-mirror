function die(msg) {
    print msg > "/dev/fd/2";
    # awk will still do the END block before exiting...
    justexit = 1;
    exit(1);
}

# Ignore blank lines and comments.
/^ *$/	{ next; }
/^#/	{ next; }

# Process normal lines.
{
    packages[$1] = 1;
    n = 2;

    nparts = split($1, parts, /\//);
    if (expand[parts[nparts]])
	expand[parts[nparts]] = "AMBIGUOUS";
    else
	expand[parts[nparts]] = $1;
    expand[$1] = $1;

    package_names[$1] = "athena-"parts[nparts];

    while (n <= NF) {
	if ($n == "early")
	    early[numearly++] = $1;
	else if ($n == "late")
	    late[numlate++] = $1;
	else if ($n == "requires")
	    prereqs[$1] = $++n;
	else if ($n == "on" || $n == "except") {
	    built[$1] = $n == "on";
	    split($(n+1), oses, /,/);
	    for (ind in oses) {
		if (os == oses[ind])
		    built[$1] = $n == "except";
	    }
	    n++;
	} else if ($n == "package")
	    package_names[$1] = "athena-"$++n;
	else
	    die("Bad packages line: " $0);
	n++;
    }
}

# Output a package name, preceded by all of its dependencies that haven't
# already been output.
function build(pkg) {
    if (built[pkg])
	return;

    if (building[pkg])
	die("Dependency loop for " pkg);
    building[pkg] = 1;

    while (prereqs[pkg]) {
	# We have to keep re-splitting prereqs because we don't have
	# local variables, so if we tried to keep more state the
	# recursive calls would write over it.
	comma = index(prereqs[pkg], ",");
	if (comma) {
	    req = substr(prereqs[pkg], 0, comma - 1);
	    prereqs[pkg] = substr(prereqs[pkg], comma + 1);
	} else {
	    req = prereqs[pkg];
	    delete prereqs[pkg];
	}
	build(expand[req]);
    }

    if (start == pkg)
	start = "";

    # If we're not still waiting to see the start package, output name.
    if (!start) {
      if (pkgnames)
	print pkg " " package_names[pkg]
      else
	print pkg;
    }

    # If that was the end package, we're done.
    if (end == pkg)
	exit(0);

    built[pkg] = 1;
    building[pkg] = 0;
}

# At the end of the file, output the list of packages to build.
END {
    # ...unless we're erroring out.
    if (justexit)
	exit;

    # Check dependencies.
    for (pkg in packages) {
	if (!prereqs[pkg])
	    continue;
	split(prereqs[pkg], reqs, /,/);
	for (ind in reqs) {
	    if (expand[reqs[ind]] == "AMBIGUOUS")
		die("Ambiguous package name " reqs[ind]);
	    else if (!(expand[reqs[ind]] in packages))
		die("No such package " reqs[ind]);
	}
    }

    if (target) {
	build(target);
	exit;
    }

    # Now remove the late packages from the packages list to keep
    # them from being built too soon.
    for (i = 0; i < numlate; i++)
	delete packages[late[i]];


    # Do early packages, in order.
    for (i = 0; i < numearly; i++)
	build(early[i]);

    # Do normal packages.
    for (pkg in packages)
	build(pkg);

    # Do late packages, in order.
    for (i = 0; i < numlate; i++)
	build(late[i]);
}
