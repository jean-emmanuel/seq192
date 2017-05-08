# Makefile.am for seq24

AM_CXXFLAGS = $(GTKMM_CFLAGS) $(JACK_CFLAGS) $(LASH_CFLAGS) -Wall
seq24_LDADD = $(GTKMM_LIBS) $(ALSA_LIBS) $(JACK_LIBS) $(LASH_LIBS)

bin_PROGRAMS = seq24

seq24_SOURCES = \
	configfile.cpp configfile.h \
	controllers.h \
	event.cpp event.h \
	font.cpp font.h \
	globals.h \
	lash.cpp lash.h \
	maintime.cpp maintime.h \
	mainwid.cpp mainwid.h \
	mainwnd.cpp mainwnd.h \
	midibus.cpp midibus.h \
	midibus_portmidi.cpp midibus_portmidi.h \
	midifile.cpp midifile.h \
	mutex.cpp mutex.h \
	options.cpp options.h \
	optionsfile.cpp optionsfile.h \
	perfedit.cpp perfedit.h \
	perfnames.cpp perfnames.h \
	perform.cpp perform.h \
	perfroll.cpp perfroll.h \
	perftime.cpp perftime.h \
	seq24.cpp \
	seqdata.cpp seqdata.h \
	seqedit.cpp seqedit.h \
	seqevent.cpp seqevent.h \
	seqkeys.cpp seqkeys.h \
	seqmenu.cpp seqmenu.h \
	seqroll.cpp seqroll.h \
	seqtime.cpp seqtime.h \
	sequence.cpp sequence.h \
	userfile.cpp userfile.h

EXTRA_DIST = configwin32.h *.xpm 

MOSTLYCLEANFILES = *~

