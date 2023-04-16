# Переопределяем install, чтобы он ничего не делал
macro(install)
endmacro()  

add_subdirectory(repo)

# Переопределяем версию стандарта C, чтобы подавить предупреждения в Clang:
# warning: redefinition of typedef 'PFN_GetPhysicalDeviceProcAddr' is a C11 feature [-Wtypedef-redefinition]
target_compile_features(vulkan PRIVATE c_std_11)
if(TARGET asm_offset)
    target_compile_features(asm_offset PRIVATE c_std_11)
endif()
