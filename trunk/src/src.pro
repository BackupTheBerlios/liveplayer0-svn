# Fichier généré par le module QMake de KDevelop.- plus édition manuelle
# -------------------------------------------------- 
# Sous dossier relatif au dossier principal du projet: ./src
# Cible: une application:  ../bin/liveplayer

HEADERS += LP_alsaout.h \
           LP_audiofile.h \
           LP_global_var.h
SOURCES += liveplayer.cpp \
           LP_alsaout.cpp \
           LP_audiofile.cpp \
           LP_global_var.cpp

# lp_sndfile_in
HEADERS += lp_sndfile_in.h
SOURCES += lp_sndfile_in.cpp

# lp_libdv_in
HEADERS += lp_libdv_in.h
SOURCES += lp_libdv_in.cpp

# lp_sdl_out
HEADERS += lp_sdl_out.h
SOURCES += lp_sdl_out.cpp

# Utils
HEADERS += lp_utils/lp_custom_types/lp_custom_types.h \
           lp_utils/lp_timer/lp_timer.h \
           lp_utils/qstring_char.h \
           lp_utils/treemodel/treeitem.h \
           lp_utils/treemodel/treemodel.h \
	   lp_utils/lp_buffer.h
SOURCES += lp_utils/lp_custom_types/lp_custom_types.cpp \
           lp_utils/lp_timer/lp_timer.cpp \
           lp_utils/qstring_char.cpp \
           lp_utils/treemodel/treeitem.cpp \
           lp_utils/treemodel/treemodel.cpp \
           lp_utils/lp_array_utils.cpp \
	   lp_utils/lp_buffer.cpp

# peackmeter
FORMS += lp_peackmeter/lp_peackmeter_widget_ui.ui
HEADERS += lp_peackmeter/lp_peackmeter.h \
	   lp_peackmeter/lp_peackmeter_core.h \
	   lp_peackmeter/lp_peackmeter_widget.h 
SOURCES += lp_peackmeter/lp_peackmeter.cpp \
	   lp_peackmeter/lp_peackmeter_core.cpp \
	   lp_peackmeter/lp_peackmeter_widget.cpp

# lp_ladspa
LADSPA_DIR = lp_ladspa
# With a custom compiled qwt5
INCLUDEPATH += /usr/include/qwt
FORMS += $$LADSPA_DIR/plugin_ports/lp_ladspa_slider_float.ui \
	 $$LADSPA_DIR/plugin_ports/lp_ladspa_knob_float.ui \
         $$LADSPA_DIR/plugin_ports/lp_ladspa_slider_int.ui \
         $$LADSPA_DIR/plugin_ports/lp_ladspa_toggel.ui \
         $$LADSPA_DIR/plugin_ports/lp_ladspa_meter.ui \
         $$LADSPA_DIR/plugin_lister/lp_ladspa_lister.ui \
	 $$LADSPA_DIR/lp_ladspa_manager.ui \
	 $$LADSPA_DIR/lp_ladspa_manager_item.ui
HEADERS += $$LADSPA_DIR/lp_ladspa_plugin_describer.h \
           $$LADSPA_DIR/lp_ladspa_plugin.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_slider_float.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_knob_float.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_slider_int.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_ctl_port.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_toggel.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_meter.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_audio_ports.h \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_handle.h \
	   $$LADSPA_DIR/plugin_ports/lp_ladspa_layout.h \
           $$LADSPA_DIR/plugin_lister/lp_ladspa_lister.h \
           $$LADSPA_DIR/plugin_lister/lp_ladspa_lister_model.h \
           $$LADSPA_DIR/lp_ladspa_manager_item.h \
           $$LADSPA_DIR/lp_ladspa_manager.h
SOURCES += $$LADSPA_DIR/lp_ladspa_plugin_describer.cpp \
           $$LADSPA_DIR/lp_ladspa_plugin.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_slider_float.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_knob_float.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_slider_int.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_ctl_port.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_toggel.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_meter.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_audio_ports.cpp \
           $$LADSPA_DIR/plugin_ports/lp_ladspa_handle.cpp \
	   $$LADSPA_DIR/plugin_ports/lp_ladspa_layout.cpp \
           $$LADSPA_DIR/plugin_lister/lp_ladspa_lister.cpp \
           $$LADSPA_DIR/plugin_lister/lp_ladspa_lister_model.cpp \
           $$LADSPA_DIR/lp_ladspa_manager.cpp  \
           $$LADSPA_DIR/lp_ladspa_manager_item.cpp
	   
LIBS += -llrdf -lm -lqwt

CONFIG += debug
CONFIG += warn_on

TEMPLATE = app
TARGET=../bin/liveplayer
LIBS += -lasound -lpthread -lsndfile -lsamplerate -lSoundTouch
#CFLAFS += -ldl
LIBS += -lasound -lsndfile -lsamplerate -lSoundTouch -ldv -lSDL
