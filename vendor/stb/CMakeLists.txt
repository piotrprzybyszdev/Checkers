add_library(stb STATIC stb/stb_image.h stb/stb_image_resize2.h stb/stb_image_write.h stb.cpp)

set_target_properties(stb PROPERTIES LINKER_LANGUAGE C)

target_include_directories(stb PUBLIC ${CMAKE_SOURCE_DIR}/vendor/stb/stb)
