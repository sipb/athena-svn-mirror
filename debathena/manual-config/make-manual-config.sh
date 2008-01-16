#!/bin/bash

package() {
    perl -0ne 's/^Section: debathena-config/Section: debathena-manual-config/m;
         s/^Package: debathena-(.*)-config$/Package: debathena-manual-$1-config/m;
         $package=$1;
         s/^Version: (.*)~.*$/Version: \1/m;
         $version=$1;
         s/$/\nConflicts: debathena-$package-config\nProvides: debathena-$package-config\nMaintainer: Debian-Athena Project <debathena\@mit.edu>\nStandards-Version: 3.6.2\nCopyright: ..\/common\/copyright\nReadme: ..\/common\/README-manual-config.in\nDescription: Debian-Athena manual configuration for $package\n This is a Debian-Athena manual configuration package.  It provides an\n alternate way to satisfy any dependencies on debathena-$package-config,\n for those who prefer to do this configuration manually.\n .\n If you want $package to be configured automatically, make sure you have\n enabled the debathena-config component of the repository, and install\n the debathena-$package-config package instead.\n/;
         print "manual-$package-config debathena-manual-$package-config_$version.equivs\n$_";' | \
#    cat; cat /dev/null | \
    (
	read -r dir file && mkdir -p "$dir" && chmod 777 "$dir" && cd "$dir" && \
	    if ! [ -e "$file" ]; then
	        cat > "$file"
	        equivs-build --full "$file"
		echo "$dir/$file" | sed 's/\.equivs$/_amd64.changes/' >> ../upload-queue
	    fi
    )
}
