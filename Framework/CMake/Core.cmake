# Recluse framework core builds.
#
set ( RECLUSE_CORE_INCLUDE ${RECLUSE_FRAMEWORK_INCLUDE}/Recluse )
set ( RECLUSE_CORE_SOURCE ${RECLUSE_FRAMEWORK_SOURCE} )

set ( RECLUSE_CORE_INCLUDE_MATH ${RECLUSE_CORE_INCLUDE}/Math )
set ( RECLUSE_CORE_INCLUDE_MEMORY ${RECLUSE_CORE_INCLUDE}/Memory )
set ( RECLUSE_CORE_INCLUDE_SERIALIZATION ${RECLUSE_CORE_INCLUDE}/Serialization )
set ( RECLUSE_CORE_INCLUDE_SYSTEM ${RECLUSE_CORE_INCLUDE}/System )
set ( RECLUSE_CORE_INCLUDE_THREADING ${RECLUSE_CORE_INCLUDE}/Threading )

set ( RECLUSE_CORE_SOURCE_LOGGING ${RECLUSE_CORE_SOURCE}/Logging )
set ( RECLUSE_CORE_SOURCE_MATH ${RECLUSE_CORE_SOURCE}/Math )
set ( RECLUSE_CORE_SOURCE_MEMORY ${RECLUSE_CORE_SOURCE}/Memory )

set ( RECLUSE_CORE_INCLUDE_STRUCTURES ${RECLUSE_CORE_INCLUDE}/Structures )
set ( RECLUSE_CORE_SOURCE_STRUCTURES ${RECLUSE_CORE_SOURCE}/Structures )

set ( RECLUSE_CORE_INCLUDE_FILESYSTEM ${RECLUSE_CORE_INCLUDE}/Filesystem )

set ( RECLUSE_CORE_INCLUDE_ALGORITHMS ${RECLUSE_CORE_INCLUDE}/Algorithms )
set ( RECLUSE_CORE_SOURCE_ALGORITHMS ${RELCUSE_CORE_SOURCE}/Algorithms )

set ( RECLUSE_CORE_SOURCE_HASH ${RECLUSE_CORE_SOURCE}/Hash )

# Need to handle Operating system environments.
set ( RECLUSE_CORE_SOURCE_SYSTEM )


if (R_USE_XXHASH)
	add_definitions(-DR_USE_XXHASH=1)
endif()

if (WIN32)
    set ( RECLUSE_WIN32 ${RECLUSE_CORE_SOURCE}/Win32 )
    set ( RECLUSE_WIN32_IO ${RECLUSE_WIN32}/IO )
    set ( RECLUSE_WIN32_THREADING ${RECLUSE_WIN32}/Threading )
    set ( RECLUSE_CORE_SOURCE_SYSTEM
        ${RECLUSE_WIN32}/Win32Common.hpp
        ${RECLUSE_WIN32}/Win32Counter.cpp
        ${RECLUSE_WIN32}/Win32DLLLoader.cpp
        ${RECLUSE_WIN32}/Win32Runtime.hpp
        ${RECLUSE_WIN32}/Win32Runtime.cpp
        ${RECLUSE_WIN32}/Win32Time.cpp
        ${RECLUSE_WIN32_IO}/Win32Keyboard.hpp
        ${RECLUSE_WIN32_IO}/Win32Keyboard.cpp
        ${RECLUSE_WIN32_IO}/Win32Mouse.hpp
        ${RECLUSE_WIN32_IO}/Win32Mouse.cpp
        ${RECLUSE_WIN32_IO}/Win32Window.hpp
        ${RECLUSE_WIN32_IO}/Win32Window.cpp
        ${RECLUSE_WIN32_THREADING}/Win32Thread.hpp
        ${RECLUSE_WIN32_THREADING}/Win32Threading.cpp
		${RECLUSE_WIN32_THREADING}/Win32Process.cpp
        ${RECLUSE_WIN32}/Win32Filesystem.cpp
    )
endif()

set ( RECLUSE_CORE_BUILD 
    ${RECLUSE_CORE_SOURCE_SYSTEM}
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/BVH.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/HashMap.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/HashTable.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/KDTree.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/LinkedList.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/PriorityQueue.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/RBTree.hpp
	${RECLUSE_CORE_INCLUDE_STRUCTURES}/Octree.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Bubblesort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Heapsort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Insertionsort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Mergesort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/QSort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Radixsort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Selectionsort.hpp
	${RECLUSE_CORE_INCLUDE_ALGORITHMS}/Common.hpp
    ${RECLUSE_CORE_INCLUDE_SERIALIZATION}/Hasher.hpp
    ${RECLUSE_CORE_INCLUDE_SERIALIZATION}/Serializable.hpp
    ${RECLUSE_CORE_INCLUDE_SERIALIZATION}/SerialTypes.hpp
    ${RECLUSE_CORE_INCLUDE_SYSTEM}/DLLLoader.hpp
    ${RECLUSE_CORE_INCLUDE_SYSTEM}/Input.hpp
	${RECLUSE_CORE_INCLUDE_SYSTEM}/InputController.hpp
	${RECLUSE_CORE_INCLUDE_SYSTEM}/KeyboardInput.hpp
    ${RECLUSE_CORE_INCLUDE_SYSTEM}/Mouse.hpp
    ${RECLUSE_CORE_INCLUDE_SYSTEM}/Window.hpp
	${RECLUSE_CORE_INCLUDE_SYSTEM}/DateTime.hpp
	${RECLUSE_CORE_INCLUDE_SYSTEM}/Process.hpp
	${RECLUSE_CORE_INCLUDE_SYSTEM}/Limiter.hpp
    ${RECLUSE_CORE_SOURCE_LOGGING}/LogFramework.hpp
    ${RECLUSE_CORE_SOURCE_LOGGING}/Logger.cpp
	${RECLUSE_CORE_SOURCE_LOGGING}/LogOverloads.cpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/Allocator.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/BuddyAllocator.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/FreeListAllocator.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/MemoryPool.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/MemoryScan.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/PoolAllocator.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/LinearAllocator.hpp
    ${RECLUSE_CORE_INCLUDE_MEMORY}/MemoryCommon.hpp
    ${RECLUSE_CORE_SOURCE_MEMORY}/MemoryPool.cpp
    ${RECLUSE_CORE_SOURCE_MEMORY}/BuddyAllocator.cpp
	${RECLUSE_CORE_SOURCE_MEMORY}/AllocatorCommon.cpp
    ${RECLUSE_CORE_INCLUDE_THREADING}/Threading.hpp
    ${RECLUSE_CORE_INCLUDE_THREADING}/ThreadPool.hpp
	${RECLUSE_CORE_INCLUDE}/Utility.hpp
    ${RECLUSE_CORE_INCLUDE}/Types.hpp
    ${RECLUSE_CORE_INCLUDE}/Array.hpp
    ${RECLUSE_CORE_INCLUDE}/Async.hpp
    ${RECLUSE_CORE_INCLUDE}/FwdDeclarations.hpp
    ${RECLUSE_CORE_INCLUDE}/Namespace.hpp
    ${RECLUSE_CORE_INCLUDE}/Queue.hpp
    ${RECLUSE_CORE_INCLUDE}/Time.hpp
    ${RECLUSE_CORE_INCLUDE}/Messaging.hpp
    ${RECLUSE_CORE_INCLUDE}/String.hpp
    ${RECLUSE_CORE_INCLUDE}/Logger.hpp
    ${RECLUSE_CORE_INCLUDE}/Arch.hpp
    ${RECLUSE_CORE_SOURCE}/String.cpp
    ${RECLUSE_CORE_SOURCE}/Archive.cpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Bounds2D.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Bounds3D.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/DualQuaternion.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/MathCommons.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/MathIntrinsics.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Matrix22.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Matrix33.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Matrix44.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Matrix43.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Vector2.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Vector3.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Vector4.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Transformations.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Ray.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Plane.hpp
    ${RECLUSE_CORE_INCLUDE_MATH}/Quaternion.hpp
	${RECLUSE_CORE_INCLUDE_MATH}/Half.hpp
	${RECLUSE_CORE_INCLUDE_MATH}/Frustum.hpp
	${RECLUSE_CORE_SOURCE_MATH}/Half.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Matrix33.cpp
	${RECLUSE_CORE_SOURCE_MATH}/Matrix22.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Ray.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Matrix44.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Quaternion.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/SIMDMatrix44.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/SIMDMatrix33.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Vector2.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Vector3.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Vector4.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/SIMDQuaternion.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Bounds3D.cpp
    ${RECLUSE_CORE_SOURCE_MATH}/Bounds2D.cpp
	${RECLUSE_CORE_SOURCE_MATH}/Frustum.cpp
	${RECLUSE_CORE_SOURCE_MATH}/Matrix43.cpp
    ${RECLUSE_CORE_SOURCE_HASH}/meow_hash_x64_aesni.h
    ${RECLUSE_CORE_SOURCE_HASH}/Hash.cpp
    ${RECLUSE_CORE_INCLUDE_FILESYSTEM}/Filesystem.hpp
    ${RECLUSE_CORE_INCLUDE_FILESYSTEM}/FileSearch.hpp
    ${RECLUSE_CORE_INCLUDE_FILESYSTEM}/Archive.hpp
    ${RECLUSE_CORE_INCLUDE_FILESYSTEM}/FileStreamer.hpp
	${RECLUSE_CORE_INCLUDE}/RGUID.hpp
	${RECLUSE_CORE_INCLUDE}/MessageBus.hpp
	${RECLUSE_CORE_SOURCE}/RGUID.cpp
	${RECLUSE_CORE_SOURCE}/ThreadPool.cpp
	${RECLUSE_CORE_SOURCE}/MessageBus.cpp
)

set ( RECLUSE_FRAMEWORK_COMPILE_FILES
    ${RECLUSE_FRAMEWORK_COMPILE_FILES}
    ${RECLUSE_CORE_BUILD}
)