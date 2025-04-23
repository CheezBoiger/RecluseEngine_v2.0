//
#include "Recluse/Pipeline/Importer.hpp"

#include "FBX/FBXImporter.hpp"

namespace Recluse {
namespace Pipeline {
namespace Builder {


Importer* Importer::create(FileFormat fileFormat)
{
    Importer* importer = nullptr;
    switch (fileFormat)
    {
        case FileFormat_GLTF:
            break;

        case FileFormat_FBX:
        default:
            importer = FBX::FbxImport::create();
            break;
    }
    return importer;
}


ResultCode Importer::destroy(Importer* importer)
{
    ResultCode result = RecluseResult_Failed;
    switch (importer->getFormat())
    {
        case FileFormat_GLTF:
            break;

        case FileFormat_FBX:
        default:
            result = FBX::FbxImport::destroy(importer);
            break;
    }
    return result;
}
} // Builder
} // Pipeline
} // Recluse