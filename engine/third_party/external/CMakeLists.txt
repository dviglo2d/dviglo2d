#add_subdirectory(freetype)
#add_subdirectory(glm)
#add_subdirectory(miniz)
#add_subdirectory(pugixml)
#add_subdirectory(sdl)
#add_subdirectory(sdl_mixer)
#add_subdirectory(stb)
#add_subdirectory(vulkan-headers)
#add_subdirectory(vma-hpp)

# Добавляем подпапки.
# EXCLUDE_FROM_ALL означает, что библиотеки будут компилироваться,
# только если они подключаются другими таргетами
dv_add_all_subdirs(EXCLUDE_FROM_ALL)
