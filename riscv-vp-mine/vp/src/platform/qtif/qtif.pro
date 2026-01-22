TEMPLATE = lib
VPATH = /home/x/riscv-vp-mine/vp/src/platform/qtif
INCLUDEPATH += $$VPATH/../utilities
INCLUDEPATH += $$VPATH/../pwt
INCLUDEPATH += /usr/include/qwt
INCLUDEPATH += /home/x/riscv-vp-mine/vp/src
LIBS += -lqwt
INCLUDEPATH += /usr/local/qwt-6.1.6/include
LIBS += -L "/usr/local/qwt-6.1.6/lib/" -lqwt
QMAKE_CXXFLAGS += -Wno-unused-parameter -I/usr/include/SDL -fpermissive -I/usr/local/systemc-2.3.3/include

# Input
HEADERS += temp_monitor.hpp temp_map.hpp qt_exiter.hpp sim_brake.hpp
SOURCES += qt_main.cpp temp_monitor.cpp temp_map.cpp qt_exiter.cpp sim_brake.cpp