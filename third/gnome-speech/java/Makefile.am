if BUILD_JAVA_PACKAGE
GNOME_SPEECH_CLASS_FILES=org/GNOME/Speech/*.class
GNOME_SPEECH_JAVA_FILES=org/GNOME/Speech/*.java
GNOME_SPEECH_IDL_FILES = \
	$(top_srcdir)/idl/GNOME_Speech_SynthesisDriver.idl 
IDLJ_FLAGS = \
	-emitAll \
	-i $(BONOBO_ACTIVATION_IDL_DIR) \
	-i $(top_srcdir)/idl

	
gnome-speech.jar: $(GNOME_SPEECH_CLASS_FILES)
	$(JAR) -cf gnome-speech.jar $(GNOME_SPEECH_CLASS_FILES)

$(GNOME_SPEECH_CLASS_FILES): $(GNOME_SPEECH_JAVA_FILES)
	$(JAVAC) $(GNOME_SPEECH_JAVA_FILES)

$(GNOME_SPEECH_JAVA_FILES):
	$(IDLJ) \
	$(IDLJ_FLAGS) \
	-pkgPrefix GNOME org \
	-pkgPrefix Bonobo org.GNOME \
	-fallTie $(GNOME_SPEECH_IDL_FILES)

clean:
	rm $(GNOME_SPEECH_JAVA_FILES)
	rm $(GNOME_SPEECH_CLASS_FILES)
	rm gnome-speech.jar

gnome_speech_jardir = \
	$(prefix)/share/jar

gnome_speech_jar_DATA = \
	gnome-speech.jar
endif
