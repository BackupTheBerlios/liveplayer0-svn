# Fichier g���par le module QMake de KDevelop. 
# -------------------------------------------------- 
# Sous dossier relatif au dossier principal du projet: ./src
# Cible: une application??:  ../bin/liveplayer

HEADERS += LP_alsaout.h \
           LP_audiofile.h \
           LP_global_var.h 
SOURCES += liveplayer.cpp \
           LP_alsaout.cpp \
           LP_audiofile.cpp \
           LP_global_var.cpp 
TARGET=../bin/liveplayer

LIBS += -lasound -lpthread -lsndfile -lsamplerate -lSoundTouch -lvorbis -lvorbisfile
