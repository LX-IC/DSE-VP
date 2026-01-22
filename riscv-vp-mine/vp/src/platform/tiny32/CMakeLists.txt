file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-SM3
SM3.cpp
SM3_REG_AHB.cpp
AHB_Socket.cpp
TLM_SM3_top.cpp
SM3_Socket.cpp
SM3_top.cpp
../pwt/pwt_module.cpp
../pwt/atmi_wrapper.cpp
${HEADERS})

add_library(platform-SM4
SM4.cpp
SM4_REG.cpp
SM4_REG_AHB.cpp
AHB_Socket.cpp
TLM_SM4_top.cpp
SM4_Socket.cpp
SM4_top.cpp
../pwt/pwt_module.cpp
../pwt/atmi_wrapper.cpp
${HEADERS})

add_executable(tiny32-vp
        tiny32_main.cpp
        ../atmi/src/atmi.c
	../utilities/parameter.cpp
        ${HEADERS})

LINK_LIBRARIES("/home/x/riscv-vp-mine/vp/src/platform/atmi/lib/libatmi.a")

#target_link_libraries(tiny32-vp rv32 platform-SM3 platform-SM4 platform-common gdb-mc
#	${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

target_link_libraries(tiny32-vp rv32 platform-SM3 platform-SM4 platform-common gdb-mc gsl gslcblas SDL /home/x/riscv-vp-mine/vp/src/platform/qtif/libqtif.so /usr/local/systemc-2.3.3/lib-linux64/libsystemc.so
	${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS tiny32-vp RUNTIME DESTINATION bin)
