Array.o: \
 $(srcdir)/Array.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h
Catalog.o: \
 $(srcdir)/Catalog.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Page.h \
 $(srcdir)/Error.h \
 $(srcdir)/Link.h \
 $(srcdir)/Catalog.h
Dict.o: \
 $(srcdir)/Dict.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/XRef.h
Error.o: \
 $(srcdir)/Error.cc \
 $(srcdir)/Params.h \
 $(srcdir)/Error.h
Lexer.o: \
 $(srcdir)/Lexer.cc \
 $(srcdir)/Lexer.h \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Error.h
Link.o: \
 $(srcdir)/Link.cc \
 $(srcdir)/Error.h \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Link.h
Object.o: \
 $(srcdir)/Object.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Error.h \
 $(srcdir)/XRef.h
OutputDev.o: \
 $(srcdir)/OutputDev.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/GfxState.h \
 $(srcdir)/OutputDev.h
Page.o: \
 $(srcdir)/Page.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/XRef.h \
 $(srcdir)/OutputDev.h \
 $(srcdir)/Error.h \
 $(srcdir)/Params.h \
 $(srcdir)/Page.h
Params.o: \
 $(srcdir)/Params.cc \
 $(srcdir)/Params.h
Parser.o: \
 $(srcdir)/Parser.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Parser.h \
 $(srcdir)/Lexer.h \
 $(srcdir)/Error.h
PDFDoc.o: \
 $(srcdir)/PDFDoc.cc \
 $(srcdir)/config.h \
 $(srcdir)/Page.h \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Catalog.h \
 $(srcdir)/XRef.h \
 $(srcdir)/Link.h \
 $(srcdir)/OutputDev.h \
 $(srcdir)/Params.h \
 $(srcdir)/Error.h \
 $(srcdir)/PDFDoc.h
Stream.o: \
 $(srcdir)/Stream.cc \
 $(srcdir)/config.h \
 $(srcdir)/Error.h \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Stream-CCITT.h
XRef.o: \
 $(srcdir)/XRef.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/Lexer.h \
 $(srcdir)/Parser.h \
 $(srcdir)/Error.h \
 $(srcdir)/XRef.h
pdftoepdf.o: pdftoepdf.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/XRef.h \
 $(srcdir)/Catalog.h \
 $(srcdir)/Page.h \
 $(srcdir)/PDFDoc.h \
 $(srcdir)/Link.h \
 $(srcdir)/Params.h \
 $(srcdir)/Error.h \
 epdf.h
pdftoepdf.o: pdftoepdf.cc \
 $(srcdir)/Object.h \
 $(srcdir)/Array.h \
 $(srcdir)/Dict.h \
 $(srcdir)/Stream.h \
 $(srcdir)/XRef.h \
 $(srcdir)/Catalog.h \
 $(srcdir)/Page.h \
 $(srcdir)/PDFDoc.h \
 $(srcdir)/Link.h \
 $(srcdir)/Params.h \
 $(srcdir)/Error.h \
 epdf.h
