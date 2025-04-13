//
#pragma once

#include "Recluse/Types.hpp"

#include "ReclusePipeline_exports.hpp"

namespace Recluse {
namespace Pipeline {
namespace Builder {

enum FileFormat 
{
    FileFormat_GLTF,
    FileFormat_FBX
};


class ReclusePipeline_PUBLIC_API Importer
{
public:
    Importer(FileFormat format) : m_format(format) { }
    virtual ~Importer() { }

    virtual ResultCode importFile(const std::string& filePath) = 0;

    FileFormat getFormat() const { return m_format; }
    
    static Importer*    create(FileFormat format);
    static ResultCode   destroy(Importer* importer);

private:
    FileFormat m_format;
};


} // Builder
} // Pipeline
} // Recluse