## Process this file with automake to produce Makefile.in

SUBDIRS = dvbinfo
DIST_SUBDIRS = $(SUBDIRS)

noinst_PROGRAMS = decode_pat decode_pmt get_pcr_pid decode_sdt decode_mpeg decode_bat dump_pids check_cc_pid

dump_pids_SOURCES = dump_pids.c
dump_pids_CPPFLAGS =
dump_pids_LDFLAGS =

check_cc_pid_SOURCES = check_cc_pid.c
check_cc_pid_CPPFLAGS =
check_cc_pid_LDFLAGS =

decode_pat_SOURCES = decode_pat.c
decode_pat_CPPFLAGS = -DDVBPSI_DIST
decode_pat_LDFLAGS = -L../src -ldvbpsi

decode_pmt_SOURCES = decode_pmt.c
decode_pmt_CPPFLAGS = -DDVBPSI_DIST
decode_pmt_LDFLAGS = -L../src -ldvbpsi -lm

get_pcr_pid_SOURCES = get_pcr_pid.c
get_pcr_pid_CPPFLAGS = -DDVBPSI_DIST
get_pcr_pid_LDFLAGS = -L../src -ldvbpsi -lm

decode_sdt_SOURCES = decode_sdt.c
decode_sdt_CPPFLAGS = -DDVBPSI_DIST
decode_sdt_LDFLAGS = -L../src -ldvbpsi

decode_mpeg_SOURCES = decode_mpeg.c
if HAVE_SYS_SOCKET_H
decode_mpeg_SOURCES += connect.c connect.h
endif
decode_mpeg_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -DDVBPSI_DIST
decode_mpeg_LDFLAGS = -L../src -ldvbpsi -lm

decode_bat_SOURCES = decode_bat.c
decode_bat_CPPFLAGS = -DDVBPSI_DIST
decode_bat_LDFLAGS = -L../src -ldvbpsi
