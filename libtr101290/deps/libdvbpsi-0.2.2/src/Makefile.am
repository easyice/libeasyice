## Process this file with automake to produce Makefile.in

pkgincludedir = $(includedir)/dvbpsi

lib_LTLIBRARIES = libdvbpsi.la

libdvbpsi_la_SOURCES = dvbpsi.c dvbpsi_private.h \
                       psi.c \
                       demux.c \
                       descriptor.c \
                       $(tables_src) \
                       $(descriptors_src)

libdvbpsi_la_LDFLAGS = -version-info 7:0:0

if HAVE_WIN32
libdvbpsi_la_LDFLAGS += -no-undefined
else
if HAVE_DARWIN
libdvbpsi_la_LDFLAGS += -no-undefined
else
if HAVE_BSD
libdvbpsi_la_LDFLAGS +=
else
libdvbpsi_la_LDFLAGS += -Wl,--no-undefined
endif
endif
endif

pkginclude_HEADERS = dvbpsi.h psi.h descriptor.h demux.h \
                     tables/pat.h tables/pmt.h tables/sdt.h tables/eit.h \
                     tables/cat.h tables/nit.h tables/tot.h tables/sis.h \
		     tables/bat.h \
                     descriptors/dr_02.h \
                     descriptors/dr_03.h \
                     descriptors/dr_04.h \
                     descriptors/dr_05.h \
                     descriptors/dr_06.h \
                     descriptors/dr_07.h \
                     descriptors/dr_08.h \
                     descriptors/dr_09.h \
                     descriptors/dr_0a.h \
                     descriptors/dr_0b.h \
                     descriptors/dr_0c.h \
                     descriptors/dr_0d.h \
                     descriptors/dr_0e.h \
                     descriptors/dr_0f.h \
                     descriptors/dr_42.h \
                     descriptors/dr_43.h \
                     descriptors/dr_44.h \
                     descriptors/dr_45.h \
                     descriptors/dr_47.h \
                     descriptors/dr_48.h \
                     descriptors/dr_4d.h \
                     descriptors/dr_4e.h \
                     descriptors/dr_52.h \
                     descriptors/dr_55.h \
                     descriptors/dr_56.h \
                     descriptors/dr_58.h \
                     descriptors/dr_59.h \
                     descriptors/dr_5a.h \
                     descriptors/dr_69.h \
		     descriptors/dr_8a.h \
                     descriptors/dr.h

descriptors_src = descriptors/dr_02.c \
                  descriptors/dr_03.c \
                  descriptors/dr_04.c \
                  descriptors/dr_05.c \
                  descriptors/dr_06.c \
                  descriptors/dr_07.c \
                  descriptors/dr_08.c \
                  descriptors/dr_09.c \
                  descriptors/dr_0a.c \
                  descriptors/dr_0b.c \
                  descriptors/dr_0c.c \
                  descriptors/dr_0d.c \
                  descriptors/dr_0e.c \
                  descriptors/dr_0f.c \
                  descriptors/dr_42.c \
                  descriptors/dr_43.c \
                  descriptors/dr_44.c \
                  descriptors/dr_45.c \
                  descriptors/dr_47.c \
                  descriptors/dr_48.c \
                  descriptors/dr_4d.c \
                  descriptors/dr_4e.c \
                  descriptors/dr_52.c \
                  descriptors/dr_55.c \
                  descriptors/dr_56.c \
                  descriptors/dr_58.c \
                  descriptors/dr_59.c \
                  descriptors/dr_5a.c \
                  descriptors/dr_69.c \
		  descriptors/dr_8a.c

tables_src = tables/pat.c tables/pat_private.h \
             tables/pmt.c tables/pmt_private.h \
             tables/sdt.c tables/sdt_private.h \
             tables/eit.c tables/eit_private.h \
             tables/cat.c tables/cat_private.h \
             tables/nit.c tables/nit_private.h \
             tables/tot.c tables/tot_private.h \
             tables/sis.c tables/sis_private.h \
	     tables/bat.c tables/bat_private.h

