include_directories(${MVP_DIR}/src)

add_octfile(_init_mvp _init_mvp.cc ${MVP_DIR}/src/mvp/Octave/ov-mvpclass-ref.cc)
add_octfile(_do_homog _do_homog.cc)

add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/gtest)
