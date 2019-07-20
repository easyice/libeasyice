# Makefile.am for dvbinfo
#
noinst_PROGRAMS = dvbinfo

dvbinfo_SOURCES = dvbinfo.c dvbinfo.h libdvbpsi.c libdvbpsi.h buffer.c buffer.h
if HAVE_SYS_SOCKET_H
dvbinfo_SOURCES += tcp.c tcp.h udp.c udp.h
endif
dvbinfo_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -DDVBPSI_DIST
dvbinfo_LDFLAGS = -L../../src -ldvbpsi -pthread -lm

