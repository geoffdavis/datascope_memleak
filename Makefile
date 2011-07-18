BIN=dbmemleak
DLIB=$(LIB:.a=$(DSUFFIX))

#MAN1=dbmemleak.1
ldlibs+=$(DBLIBS)

include $(ANTELOPEMAKE)
DIRS=

OBJS=dbmemleak.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
