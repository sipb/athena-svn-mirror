I found this in a zip file, the test suite missing, as well as the Makefile.

I hacked together a quick makefile, and altered the musicout code so that if
the destination filename is "stdout" it writes the song to stdout so you can
pipeline it into sox then into /dev/audio or your equivilant.  (Handling
30 meg files takes mucho diskspace I dont have :)

I have both encoded and decoded with this.  I decoded a song off the IUMA 
archives, and encoded a topgun soundtrack I digitzed myself.  One thing to 
note, at the default encoding bitrate of 384 bits, things dont compress hardly
at all, you'll want to input something like 128 bits, which does on average
8-10:1 compression.

Encoding takes a *LONG* time... :)

-Crh
