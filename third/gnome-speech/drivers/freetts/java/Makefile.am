SUBDIRS = org
if BUILD_FREETTS_DRIVER
FREETTS_DRIVER_CLASSES= \
	org/GNOME/Speech/*.class
CLEANFILES = $(FREETTS_DRIVER_CLASSES)
CLEANFILES+=freetts-synthesis-driver.jar

$(FREETTS_DRIVER_CLASSES): org/GNOME/Speech/*.java
	$(JAVAC) \
	-classpath $(top_srcdir)/java/gnome-speech.jar:$(GNOME_SPEECH_JAR_DIR)/gnome-java-bridge.jar:$(FREETTS_CLASSPATH) \
	org/GNOME/Speech/*.java

freetts-synthesis-driver.jar: $(FREETTS_DRIVER_CLASSES)
	$(JAR) -cfm freetts-synthesis-driver.jar \
	org/GNOME/Speech/manifest.mf \
	$(FREETTS_DRIVER_CLASSES)

freetts_driver_jardir = \
	$(prefix)/share/jar

freetts_driver_jar_DATA = \
	freetts-synthesis-driver.jar

endif
