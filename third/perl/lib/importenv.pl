;# $Header: /afs/dev.mit.edu/source/repository/third/perl/lib/importenv.pl,v 1.1.1.1 1996-10-02 06:40:27 ghudson Exp $

;# This file, when interpreted, pulls the environment into normal variables.
;# Usage:
;#	require 'importenv.pl';
;# or
;#	#include <importenv.pl>

local($tmp,$key) = '';

foreach $key (keys(ENV)) {
    $tmp .= "\$$key = \$ENV{'$key'};" if $key =~ /^[A-Za-z]\w*$/;
}
eval $tmp;

1;
