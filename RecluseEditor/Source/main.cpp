//
#include "Recluse/Algorithms/Bubblesort.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/EditorSDK/EditorDeclarations.hpp"
#include "Recluse/Time.hpp"
#include "GUIFactory.hpp"
#include "GUIContext.hpp"

#define R_EDITOR_CHAN "RecluseEditor"

int main(int c, char* argv[])
{
    Recluse::Log::initializeLoggingSystem();
    Recluse::RealtimeTick::initializeWatch(0, 0);

    Recluse::ErrType result = Recluse::GUIFactory::createContext(Recluse::GUIFactory::Context_WxWidgets);

    if (result == Recluse::R_RESULT_OK)
    {
        Recluse::IGUIContext* guiCtx = Recluse::GUIContext::get();
        guiCtx->setUp();
        guiCtx->run(c, argv);
        guiCtx->tearDown();
        Recluse::GUIFactory::destroyContext(guiCtx);
    }
    else
    {
        R_ERR(R_EDITOR_CHAN, "No context was created!");
    }

    Recluse::Log::destroyLoggingSystem();
    return 0;
}