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

    Recluse::ErrType result = Recluse::Editor::GUIFactory::createContext(Recluse::Editor::GUIFactory::Context_WxWidgets);

    if (result == Recluse::R_RESULT_OK)
    {
        Recluse::Editor::IGUIContext* guiCtx = Recluse::Editor::GUIContext::get();
        guiCtx->setUp();
        guiCtx->run(c, argv);
        guiCtx->tearDown();
        Recluse::Editor::GUIFactory::destroyContext(guiCtx);
    }
    else
    {
        R_ERR(R_EDITOR_CHAN, "No context was created!");
    }

    Recluse::Log::destroyLoggingSystem();
    return 0;
}