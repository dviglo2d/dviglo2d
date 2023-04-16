add_subdirectory(repo/VulkanMemoryAllocator)

file(GLOB vma_source_files repo/VulkanMemoryAllocator/include/*.h)
set_target_properties(VulkanMemoryAllocator PROPERTIES
                      SOURCES "${vma_source_files}" # Чтобы таргет отображался в Visual Studio
                      SYSTEM ON) # Подавляем предупреждения

add_subdirectory(repo/include)

file(GLOB vma_hpp_source_files repo/include/*.hpp)
set_target_properties(VulkanMemoryAllocator-Hpp PROPERTIES
                      SOURCES "${vma_hpp_source_files}") # Чтобы таргет отображался в Visual Studio

target_link_libraries(VulkanMemoryAllocator-Hpp INTERFACE GPUOpen::VulkanMemoryAllocator)
