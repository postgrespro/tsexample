# contrib/tsexample/Makefile

MODULES = tsexample

EXTENSION = tsexample
DATA = tsexample--1.0.sql
PGFILEDESC = "tsexample - example of custom postgresql full text search parser, dictionaries and configuration"

REGRESS = tsexample

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/tsexample
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
