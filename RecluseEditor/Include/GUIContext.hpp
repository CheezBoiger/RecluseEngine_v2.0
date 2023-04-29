//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"
#include "Recluse/RGUID.hpp"

namespace Recluse {
namespace Editor {

class MainEditorFrame;


class IGUIDriver;
class IGUIContext;

namespace GUIContext {
extern IGUIDriver*     getDriver();
extern IGUIContext*    get();
} // GUIContext

// Interface driver handles the communication of modules and whatnot, from the high level perspective.
class IGUIDriver
{
public:

private:
    RGUID m_id;
};

class IGUIContext 
{
public:
    ~IGUIContext() { }

    ResultCode             setUp()
    {

        m_id = generateRGUID();
        return onSetUp();
    }

    ResultCode             tearDown()
    {
        return onTearDown();
    }

    virtual void        run(int c, char* argv[])
    {
        onRun(c, argv);
    }

    // Get the main frame from this context.
    MainEditorFrame*    getMainFrame() { return m_mainFrame; }

protected:
    virtual ResultCode     onSetUp() = 0;
    virtual ResultCode     onTearDown() = 0;
    virtual ResultCode     onRun(int c, char* argv[]) = 0;
private:
    RGUID m_id;
    MainEditorFrame*    m_mainFrame;
};
} // Editor
} // Recluse