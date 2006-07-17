# Fichier g���par le module QMake de KDevelop. 
# -------------------------------------------------- 
# Sous dossier relatif au dossier principal du projet: ./src
# Cible: une application??:  ../bin/liveplayer

HEADERS += LP_alsaout.h \
           LP_audiofile.h \
           LP_global_var.h \
	   LP_utils.h \
           LP_mad.h 	\
	   lp_peackmeter/lp_peackmeter.h
SOURCES += liveplayer.cpp \
           LP_alsaout.cpp \
           LP_audiofile.cpp \
           LP_global_var.cpp \
	   LP_utils.cpp \
           LP_mad.cpp   \
	   lp_peackmeter/lp_peackmeter.cpp

TARGET=../bin/liveplayer
LIBS += -lasound -lpthread -lsndfile -lsamplerate -lSoundTouch -lvorbis -lvorbisfile -lmad
LIBS += lp_ladspa_cpp/liblp_ladspa_cpp.a
LIBS += -lqwt
CFLAFS += -ldl
