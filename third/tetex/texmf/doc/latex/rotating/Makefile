all:	examples.dvi rotating.ps examples.ps

rotating.ps:	rotating.dvi
	dvips -o rotating.ps rotating

examples.ps:	examples.dvi
	dvips -o examples.ps examples

examples.dvi:
	latex examples

clean:
	rm -f rotating.ps examples.ps *.tmp *.log *.aux

realclean: clean
