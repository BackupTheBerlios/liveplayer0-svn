# Fichier g���par le module QMake de KDevelop. 
# -------------------------------------------------- 
# Sous dossier relatif au dossier principal du projet: ./src
# Cible: une application??:  ../bin/liveplayer08

HEADERS += LP_alsaout.h \
           LP_audiofile.h \
           LP_global_var.h 
SOURCES += liveplayer08.cpp \
           LP_alsaout.cpp \
           LP_audiofile.cpp \
           LP_global_var.cpp 
TARGET=../bin/liveplayer08

LIBS += -lasound -lpthread -lsndfile -lsamplerate
