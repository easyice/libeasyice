## Process this file with automake to produce Makefile.in

pkgincludedir = $(includedir)/dvbpsi

lib_LTLIBRARIES = libdvbpsi.la

libdvbpsi_la_SOURCES = dvbpsi.c dvbpsi_private.h \
                       psi.c \
                       demux.c \
                       descriptor.c \
                       $(tables_src) \
                       $(descriptors_src)

libdvbpsi_la_LDFLAGS = -version-info 10:0:0 -no-undefined

pkginclude_HEADERS = dvbpsi.h psi.h descriptor.h demux.h \
                     tables/pat.h tables/pmt.h tables/sdt.h tables/eit.h \
                     tables/cat.h tables/nit.h tables/tot.h tables/sis.h \
		     tables/bat.h tables/rst.h \
		     tables/atsc_vct.h tables/atsc_stt.h \
		     tables/atsc_eit.h tables/atsc_mgt.h \
		     tables/atsc_ett.h \
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
                     descriptors/dr_10.h \
                     descriptors/dr_11.h \
                     descriptors/dr_12.h \
                     descriptors/dr_13.h \
                     descriptors/dr_14.h \
                     descriptors/dr_1b.h \
                     descriptors/dr_1c.h \
		     descriptors/dr_40.h \
		     descriptors/dr_41.h \
                     descriptors/dr_42.h \
                     descriptors/dr_43.h \
                     descriptors/dr_44.h \
                     descriptors/dr_45.h \
                     descriptors/dr_47.h \
                     descriptors/dr_48.h \
                     descriptors/dr_49.h \
                     descriptors/dr_4a.h \
                     descriptors/dr_4b.h \
                     descriptors/dr_4c.h \
                     descriptors/dr_4d.h \
                     descriptors/dr_4e.h \
                     descriptors/dr_4f.h \
                     descriptors/dr_50.h \
                     descriptors/dr_52.h \
                     descriptors/dr_53.h \
                     descriptors/dr_54.h \
                     descriptors/dr_55.h \
                     descriptors/dr_56.h \
                     descriptors/dr_58.h \
                     descriptors/dr_59.h \
                     descriptors/dr_5a.h \
                     descriptors/dr_62.h \
                     descriptors/dr_66.h \
                     descriptors/dr_69.h \
                     descriptors/dr_73.h \
                     descriptors/dr_76.h \
                     descriptors/dr_7c.h \
		     descriptors/dr_81.h \
		     descriptors/dr_83.h \
		     descriptors/dr_86.h \
		     descriptors/dr_8a.h \
		     descriptors/dr_a0.h \
		     descriptors/dr_a1.h \
		     descriptors/types/aac_profile.h \
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
                  descriptors/dr_10.c \
                  descriptors/dr_11.c \
                  descriptors/dr_12.c \
                  descriptors/dr_13.c \
                  descriptors/dr_14.c \
                  descriptors/dr_1b.c \
                  descriptors/dr_1c.c \
		  descriptors/dr_40.c \
		  descriptors/dr_41.c \
                  descriptors/dr_42.c \
                  descriptors/dr_43.c \
                  descriptors/dr_44.c \
                  descriptors/dr_45.c \
                  descriptors/dr_47.c \
                  descriptors/dr_48.c \
                  descriptors/dr_49.c \
                  descriptors/dr_4a.c \
                  descriptors/dr_4b.c \
                  descriptors/dr_4c.c \
                  descriptors/dr_4d.c \
                  descriptors/dr_4e.c \
                  descriptors/dr_4f.c \
                  descriptors/dr_50.c \
                  descriptors/dr_52.c \
                  descriptors/dr_53.c \
                  descriptors/dr_54.c \
                  descriptors/dr_55.c \
                  descriptors/dr_56.c \
                  descriptors/dr_58.c \
                  descriptors/dr_59.c \
                  descriptors/dr_5a.c \
                  descriptors/dr_62.c \
                  descriptors/dr_66.c \
                  descriptors/dr_69.c \
                  descriptors/dr_73.c \
                  descriptors/dr_76.c \
                  descriptors/dr_7c.c \
                  descriptors/dr_81.c \
                  descriptors/dr_83.c \
                  descriptors/dr_86.c \
		  descriptors/dr_8a.c \
		  descriptors/dr_a0.c \
		  descriptors/dr_a1.c

tables_src = tables/pat.c tables/pat_private.h \
             tables/pmt.c tables/pmt_private.h \
             tables/sdt.c tables/sdt_private.h \
             tables/eit.c tables/eit_private.h \
             tables/cat.c tables/cat_private.h \
             tables/nit.c tables/nit_private.h \
             tables/tot.c tables/tot_private.h \
             tables/sis.c tables/sis_private.h \
	     tables/bat.c tables/bat_private.h \
	     tables/rst.c tables/rst_private.h \
	     tables/atsc_vct.c tables/atsc_vct.h \
	     tables/atsc_stt.c tables/atsc_stt.h \
	     tables/atsc_eit.c tables/atsc_eit.h \
	     tables/atsc_ett.c tables/atsc_ett.h \
	     tables/atsc_mgt.c tables/atsc_mgt.h

install-data-local:
	mkdir -p $(DESTDIR)$(pkgincludedir)/types

install-data-hook:
	mv $(DESTDIR)$(pkgincludedir)/aac_profile.h $(DESTDIR)$(pkgincludedir)/types

uninstall-local:
	rm -f $(DESTDIR)$(pkgincludedir)/types/aac_profile.h
	rmdir $(DESTDIR)$(pkgincludedir)/types
