LIBRARY=dmglib.a

OBJECTS=$(SOURCES:.c=.o)

SOURCES_IPSW=\
	8900.c \
	ibootim.c \
	img2.c \
	img3.c \
	libxpwn.c \
	lzss.c \
	lzssfile.c \
	nor_files.c \
	
SOURCES_DMG=\
	base64.c \
	checksum.c \
	dmgfile.c \
	dmglib.c \
	filevault.c \
	io.c \
	partition.c \
	resources.c \
	udif.c \
	
SOURCES_HFS=\
    btree.c \
    extents.c \
    flatfile.c \
    hfscompress.c \
    rawfile.c \
    volume.c \
    catalog.c \
    fastunicodecompare.c \
    hfs.c \
    hfslib.c \
    utility.c \
    xattr.c \
    
SOURCES_DLL=\
	hfsdll.c \
	xpwndll.c \
    
SOURCES_COMMON= \
    abstractfile.c \
    
SOURCES=$(patsubst %.c, ipsw-patch/%.c, $(SOURCES_IPSW)) \
    $(patsubst %.c, dmg/%.c,  $(SOURCES_DMG)) \
    $(patsubst %.c, common/%.c,  $(SOURCES_COMMON)) \
    $(patsubst %.c, hfs/%.c,  $(SOURCES_HFS)) \
    $(patsubst %.c, dll/%.c,  $(SOURCES_DLL)) \
    

CFLAGS=-Iincludes -force_cpusubtype_ALL -mmacosx-version-min=13.0 -arch x86_64

OBJS = $(patsubst %.c, %.o, $(SOURCES))

LT=libtool -static

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(LIBRARY)
	
$(LIBRARY): $(OBJECTS) 
	$(LT) $(LDFLAGS) -o $@ $(OBJECTS) 

clean:
	rm $(LIBRARY)
	rm $(OBJECTS)
