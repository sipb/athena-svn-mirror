include Makefile.config

GET_EDID_EXE = ./get-edid.exe
PARSE_EDID_EXE = ./parse-edid.exe

GET_EDID = ./get-edid
PARSE_EDID = ./parse-edid

get_edid_objects = get-edid.o
parse_edid_objects = parse-edid.o
lrmi_objects = lrmi/lrmi.o

all: linux

linux: $(GET_EDID) $(PARSE_EDID)

dos: $(GET_EDID_EXE) $(PARSE_EDID_EXE)

$(GET_EDID_EXE): $(get_edid_objects)
$(PARSE_EDID_EXE): $(parse_edid_objects)

$(GET_EDID): $(get_edid_objects) $(lrmi_objects)
$(PARSE_EDID): $(parse_edid_objects) 

read-edid: all
	$(GET_EDID) | $(PARSE_EDID)

read-edid-dos: dos
	$(GET_EDID_EXE) EDID
	$(PARSE_EDID) EDID

$(lrmi_objects):
	$(MAKE) -C lrmi $(@F)

clean:
	$(MAKE) -C lrmi clean
	$(RM) $(get_edid_objects) $(parse_edid_objects) $(GET_EDID) $(PARSE_EDID) $(GET_EDID_EXE) $(PARSE_EDID_EXE)

.PHONY: clean all linux read-edid dos read-edid-dos
