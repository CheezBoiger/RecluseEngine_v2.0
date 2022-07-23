// 
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Namespace.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

RECLUSE_BEGIN_NAMESPACE

class FileStreamer
{
public:
	FileStreamer(U64 numStreamableBytes = R_KB(1ull), U64 alignment = R_BYTES(256))
};

RECLUSE_END_NAMESPACE