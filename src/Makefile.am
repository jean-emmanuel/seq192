# Makefile.am for seq24

AM_CXXFLAGS = $(GTKMM_CFLAGS) $(JACK_CFLAGS) $(LASH_CFLAGS) -Wall -Werror
seq24_LDADD = $(GTKMM_LIBS) $(ALSA_LIBS) $(JACK_LIBS) $(LASH_LIBS) -llo

bin_PROGRAMS = seq24

seq24_SOURCES = \
	core/globals.h \
	core/controllers.h \
	core/configfile.cpp core/configfile.h \
	core/event.cpp core/event.h \
	core/midibus.cpp core/midibus.h \
	core/midifile.cpp core/midifile.h \
	core/mutex.cpp core/mutex.h \
	core/optionsfile.cpp core/optionsfile.h \
	core/osc.cpp core/osc.h \
	core/perform.cpp core/perform.h \
	core/sequence.cpp core/sequence.h \
	core/userfile.cpp core/userfile.h \
	gui/mainwindow.cpp gui/mainwindow.h \
	gui/sequencebutton.cpp gui/sequencebutton.h \
	seq24.cpp


EXTRA_DIST = xpm/*.xpm

MOSTLYCLEANFILES = *~
