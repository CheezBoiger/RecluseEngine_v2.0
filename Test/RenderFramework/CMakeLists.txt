cmake_minimum_required( VERSION 3.0 )
project( "RenderFramework" )

add_subdirectory(ContextInitialization)
add_subdirectory(SwapchainInitialization)
add_subdirectory(MemoryInitialization)
add_subdirectory(BufferAllocation)
add_subdirectory(TextureAllocation)
add_subdirectory(CommandListInitialization)
add_subdirectory(CommandListSubmit)
add_subdirectory(DescriptorSetInitialization)