## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = decode_pat decode_pmt get_pcr_pid decode_sdt decode_mpeg decode_bat

decode_pat_SOURCES = decode_pat.c
decode_pat_LDFLAGS = -L../src -ldvbpsi

decode_pmt_SOURCES = decode_pmt.c
decode_pmt_LDFLAGS = -L../src -ldvbpsi -lm

get_pcr_pid_SOURCES = get_pcr_pid.c
get_pcr_pid_LDFLAGS = -L../src -ldvbpsi -lm

decode_sdt_SOURCES = decode_sdt.c
decode_sdt_LDFLAGS = -L../src -ldvbpsi

decode_mpeg_SOURCES = decode_mpeg.c
if HAVE_SYS_SOCKET_H
decode_mpeg_SOURCES += connect.c connect.h
endif
decode_mpeg_LDFLAGS = -L../src -ldvbpsi -lm

decode_bat_SOURCES = decode_bat.c
decode_bat_LDFLAGS = -L../src -ldvbpsi
