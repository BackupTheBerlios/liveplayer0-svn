TEMPLATE = app
TARGET   = liveplayer-v0

HEADERS = lp_audio_stream.h lp_ringbuffer.h lp_libsndfile_in.h lp_portaudio_IO.h

SOURCES = lp_audio_stream.cpp lp_ringbuffer.cpp lp_libsndfile_in.cpp lp_portaudio_IO.cpp main.cpp

LIBS += -lsndfile -lportaudio