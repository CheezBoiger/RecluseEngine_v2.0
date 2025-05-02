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
    Importer(FileFormat format, const char* ext) : m_format(format), m_ext(ext) { }
    virtual ~Importer() { }

    virtual ResultCode  importFile(const std::string& filePath) = 0;

    FileFormat          getFormat() const { return m_format; }
    const char*         getExtension() const { return m_ext; }
    
    static Importer*    create(FileFormat format);
    static ResultCode   destroy(Importer* importer);

private:
    FileFormat  m_format;
    const char* m_ext;
};


} // Builder
} // Pipeline
} // Recluse