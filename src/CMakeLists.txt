cmake_minimum_required(VERSION 3.6)
project(ETe C CXX)

Include(CheckTypeSize)
check_type_size("void*" CMAKE_SIZEOF_VOID_P)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message("64-bit compatibility not yet working! Enforce 32-bit with a toolchain")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

#option(USE_SDL2 "Whether to use native backends or SDL2" OFF)
#option(DYNAMIC_RENDERER "Whether to use modular rendering features (builds as shared object instead of library" OFF)

#set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")

set(CMAKE_CXX_FLAGS "-pipe -Wall -fmessage-length=0 -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG} -O1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -march=i686 -Winline -ffast-math -fomit-frame-pointer -finline-functions -fschedule-insns2")

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set(SRCDIR "${CMAKE_SOURCE_DIR}")

set(botlib_files 
    "botlib/be_aas_bspq3.c"
    "botlib/be_aas_cluster.c"
    "botlib/be_aas_debug.c"
    "botlib/be_aas_entity.c"
    "botlib/be_aas_file.c"
    "botlib/be_aas_main.c"
    "botlib/be_aas_move.c"
    "botlib/be_aas_optimize.c"
    "botlib/be_aas_reach.c"
    "botlib/be_aas_route.c"
    "botlib/be_aas_routealt.c"
    "botlib/be_aas_routetable.c"
    "botlib/be_aas_sample.c"
    "botlib/be_ai_char.c"
    "botlib/be_ai_chat.c"
    "botlib/be_ai_gen.c"
    "botlib/be_ai_goal.c"
    "botlib/be_ai_move.c"
    "botlib/be_ai_weap.c"
    "botlib/be_ai_weight.c"
    "botlib/be_ea.c"
    "botlib/be_interface.c"
    "botlib/l_crc.c"
    "botlib/l_libvar.c"
    "botlib/l_log.c"
    "botlib/l_memory.c"
    "botlib/l_precomp.c"
    "botlib/l_script.c"
    "botlib/l_struct.c"
)

set(client_files
    "client/cl_avi.c"
    "client/cl_cgame.c"
    "client/cl_cin.c"
    "client/cl_console.c"
    "client/cl_curl.c"
    "client/cl_discord.c"
    "client/cl_input.c"
    "client/cl_jpeg.c"
    "client/cl_keys.c"
    "client/cl_main.c"
    "client/cl_net_chan.c"
    "client/cl_parse.c"
    "client/cl_scrn.c"
    "client/cl_ui.c"
    "client/snd_adpcm.c"
    "client/snd_codec_wav.c"
    "client/snd_codec.c"
    "client/snd_dma.c"
    "client/snd_main.c"
    "client/snd_mem.c"
    "client/snd_mix.c"
    "client/snd_wavelet.c"
    "qcommon/dl_main_curl.c"
)

set(server_files
    "server/sv_bot.c"
    "server/sv_ccmds.c"
    "server/sv_client.c"
    "server/sv_filter.c"
    "server/sv_game.c"
    "server/sv_init.c"
    "server/sv_main.c"
    "server/sv_net_chan.c"
    "server/sv_snapshot.c"
    "server/sv_world.c"
)

set(qcommon_files
    "qcommon/cm_load.c"
    "qcommon/cm_patch.c"
    "qcommon/cm_polylib.c"
    "qcommon/cm_test.c"
    "qcommon/cm_trace.c"
    "qcommon/cmd.c"
    "qcommon/common.c"
    "qcommon/cvar.c"
    "qcommon/files.c"
    "qcommon/gameinfo.c"
    "qcommon/history.c"
    "qcommon/huffman_static.c"
    "qcommon/huffman.c"
    "qcommon/keys.c"
    "qcommon/md4.c"
    "qcommon/md5.c"
    "qcommon/msg.c"
    "qcommon/net_chan.c"
    "qcommon/net_ip.c"
    "qcommon/puff.c"
    "qcommon/q_math.c"
    "qcommon/q_shared.c"
    "qcommon/unzip.c"
    "qcommon/vm.c"
)

set(splines_files
    "splines/math_angles.cpp"
    "splines/math_matrix.cpp"
    "splines/math_quaternion.cpp"
    "splines/math_vector.cpp"
    "splines/q_parse.cpp"
    "splines/q_shared.c"
    "splines/splines.cpp"
    "splines/util_str.cpp"
)

set(linux_shared_files
    "unix/linux_signals.c"
    "unix/unix_main.c"
    "unix/unix_shared.c"
)

set(linux_client_files
    "unix/linux_glimp.c"
    "unix/linux_qgl.c"
    "unix/linux_qvk.c"
    "unix/linux_snd.c"
    "unix/x11_dga.c"
    "unix/x11_randr.c"
    "unix/x11_vidmode.c"
)

#todo allow system jpeg(turbo)
set(jpeg_local_files
    "jpeg-8c/jaricom.c"
    "jpeg-8c/jcapimin.c"
    "jpeg-8c/jcapistd.c"
    "jpeg-8c/jcarith.c"
    "jpeg-8c/jccoefct.c"
    "jpeg-8c/jccolor.c"
    "jpeg-8c/jcdctmgr.c"
    "jpeg-8c/jchuff.c"
    "jpeg-8c/jcinit.c"
    "jpeg-8c/jcmainct.c"
    "jpeg-8c/jcmarker.c"
    "jpeg-8c/jcmaster.c"
    "jpeg-8c/jcomapi.c"
    "jpeg-8c/jcparam.c"
    "jpeg-8c/jcprepct.c"
    "jpeg-8c/jcsample.c"
    "jpeg-8c/jctrans.c"
    "jpeg-8c/jdapimin.c"
    "jpeg-8c/jdapistd.c"
    "jpeg-8c/jdarith.c"
    "jpeg-8c/jdatadst.c"
    "jpeg-8c/jdatasrc.c"
    "jpeg-8c/jdcoefct.c"
    "jpeg-8c/jdcolor.c"
    "jpeg-8c/jddctmgr.c"
    "jpeg-8c/jdhuff.c"
    "jpeg-8c/jdinput.c"
    "jpeg-8c/jdmainct.c"
    "jpeg-8c/jdmarker.c"
    "jpeg-8c/jdmaster.c"
    "jpeg-8c/jdmerge.c"
    "jpeg-8c/jdpostct.c"
    "jpeg-8c/jdsample.c"
    "jpeg-8c/jdtrans.c"
    "jpeg-8c/jerror.c"
    "jpeg-8c/jfdctflt.c"
    "jpeg-8c/jfdctfst.c"
    "jpeg-8c/jfdctint.c"
    "jpeg-8c/jidctflt.c"
    "jpeg-8c/jidctfst.c"
    "jpeg-8c/jidctint.c"
    "jpeg-8c/jmemmgr.c"
    "jpeg-8c/jmemnobs.c"
    "jpeg-8c/jquant1.c"
    "jpeg-8c/jquant2.c"
    "jpeg-8c/jutils.c"
)

#note renderer needs extra files in modular mode
set(renderer_files
    "renderer/tr_animation_mdm.c"
    "renderer/tr_animation_mds.c"
    "renderer/tr_arb.c"
    "renderer/tr_backend.c"
    "renderer/tr_bsp.c"
    "renderer/tr_cmds.c"
    "renderer/tr_cmesh.c"
    "renderer/tr_curve.c"
    "renderer/tr_decals.c"
    "renderer/tr_flares.c"
    "renderercommon/tr_font.c"
    "renderer/tr_image.c"
    "renderercommon/tr_image_bmp.c"
    "renderercommon/tr_image_jpg.c"
    "renderercommon/tr_image_pcx.c"
    "renderercommon/tr_image_png.c"
    "renderercommon/tr_image_tga.c"
    "renderer/tr_init.c"
    "renderer/tr_light.c"
    "renderer/tr_main.c"
    "renderer/tr_marks.c"
    "renderer/tr_mesh.c"
    "renderer/tr_model_iqm.c"
    "renderer/tr_model.c"
    "renderercommon/tr_noise.c"
    "renderer/tr_scene.c"
    "renderer/tr_shade_calc.c"
    "renderer/tr_shade.c"
    "renderer/tr_shader.c"
    "renderer/tr_shadows.c"
    "renderer/tr_sky.c"
    "renderer/tr_surface.c"
    "renderer/tr_vbo.c"
    "renderer/tr_world.c"
)

add_library(botlib STATIC "${botlib_files}")
target_include_directories(botlib PUBLIC "${SRCDIR}/botlib ${SRCDIR}/qcommon")
target_compile_definitions(botlib PUBLIC "BOTLIB")

add_library(splines STATIC "${splines_files}")
target_include_directories(splines PUBLIC "${SRCDIR}/splines")
#target_link_libraries(splines PRIVATE "libstdc++")

add_library(renderer STATIC "${renderer_files}")
target_include_directories(renderer PUBLIC "${SRCDIR}/renderer ${SRCDIR}/renderercommon ${SRCDIR}/qcommon")

set(core_client_files
    "${client_files}"
    "${qcommon_files}"
    "${server_files}"
    "${jpeg_local_files}"
    "${linux_shared_files}"
    "${linux_client_files}")

set(dedicated_server_files
    "${qcommon_files}"
    "${server_files}"
    "${linux_shared_files}")

#add_custom_target(local_curl
#    COMMAND sh cmake-curl.sh
#    WORKING_DIRECTORY "${SRCDIR}/curl-7.58.0"
#    BYPRODUCTS "${SRCDIR}/curl-7.58.0/lib/.libs/libcurl.a"
#    )

find_package(CURL REQUIRED)

add_executable(ete "${core_client_files}")
target_include_directories(ete PUBLIC "${SRCDIR}/botlib ${SRCDIR}/server ${SRCDIR}/client ${SRCDIR}/qcommon")
target_compile_definitions(ete PUBLIC "USE_CURL")
target_link_libraries(ete PRIVATE ${CMAKE_DL_LIBS} m pthread X11 ${CURL_LIBRARIES})
target_link_libraries(ete PUBLIC botlib splines renderer)
#add_dependencies(ete local_curl)

add_executable(ete-ded "${dedicated_server_files}")
target_include_directories(ete-ded PUBLIC "${SRCDIR}/botlib ${SRCDIR}/server ${SRCDIR}/client ${SRCDIR}/qcommon")
target_compile_definitions(ete-ded PUBLIC "DEDICATED")
target_link_libraries(ete-ded PRIVATE ${CMAKE_DL_LIBS} "m")
target_link_libraries(ete-ded PUBLIC botlib)