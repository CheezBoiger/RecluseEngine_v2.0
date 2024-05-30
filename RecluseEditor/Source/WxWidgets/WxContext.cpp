//
#include "WxContext.hpp"
#include "Recluse/Messaging.hpp"

#include "MainEditorFrame.hpp"

#include "Recluse/Renderer/Renderer.hpp"
#include "WxFrame.hpp"

namespace Recluse {

#if 1

class RecluseEditorFrame : public wxFrame, public Editor::MainEditorFrame
{
public:

    enum Id 
    {
        Id_Hello,
        Id_Save,
        Id_Load,
    };

    RecluseEditorFrame();

    void onHello(wxCommandEvent& ev) 
    {
         
    }
    void onExit(wxCommandEvent& ev)
    {
        m_panel->Close(true);
        delete m_panel;
        Close(true);
    }
    void onAbout(wxCommandEvent& ev) { }

    void onDraw(wxPaintEvent& event)
    {
        m_panel->onDraw();
    }

    void onIdle(wxIdleEvent& event)
    {
        m_panel->onDraw();
        event.RequestMore();
    }
    
private:
    wxMenu* m_menu;
    wxMenu* m_menuHelp;
    wxMenuBar* m_menuBar;
    RenderPanel* m_panel;
};

RecluseEditorFrame::RecluseEditorFrame()
    : wxFrame(NULL, wxID_ANY, "Recluse Editor")
{
    m_menu = new wxMenu;
    m_menu->Append(Id_Save, "Save", "Save current scene.");
    m_menu->Append(Id_Load, "Load", "Load a current scene.");
    m_menu->AppendSeparator();
    m_menu->Append(wxID_EXIT);

    m_menuHelp = new wxMenu;
    m_menuHelp->Append(wxID_ABOUT);

    m_menuBar = new wxMenuBar;
    m_menuBar->Append(m_menu, "File");
    m_menuBar->Append(m_menuHelp, "Help");

    m_panel = new RenderPanel(this);
    m_panel->initialize();
    m_panel->Connect(wxEVT_PAINT, wxPaintEventHandler(RecluseEditorFrame::onDraw), NULL, this);
    m_panel->Connect(wxEVT_IDLE, wxIdleEventHandler(RecluseEditorFrame::onIdle), NULL, this);
    m_panel->Show();
    SetMenuBar(m_menuBar);
    
    CreateStatusBar();

    SetStatusText("Welcome to the Editor!");
    
    Bind(wxEVT_MENU, &RecluseEditorFrame::onHello, this, Id_Hello);
    Bind(wxEVT_MENU, &RecluseEditorFrame::onExit, this, wxID_EXIT);
    wxColour color(50, 50, 50, 255);
    SetBackgroundColour(color);

    wxWindow* pWindow = wxTheApp->GetTopWindow();
    if (pWindow)
    {
        
    }
    R_DEBUG("WxWidgets", "Initialized RecluseEditorFrame...");
}


class RecluseEditorApp : public wxApp
{
public:
    typedef wxApp super_t;
    virtual bool OnInit() override;
    virtual int OnExit() override;

private:
    RecluseEditorFrame* pFrame;
};


bool RecluseEditorApp::OnInit()
{
    pFrame = new RecluseEditorFrame();
    pFrame->Show();
    R_DEBUG("WxWidgets", "Created RecluseEditorFrame...");
    return true;
}


int RecluseEditorApp::OnExit()
{
    R_DEBUG("WxWidgets", "Destroyed RecluseEditorFrame...");
    return 0;
}

wxIMPLEMENT_APP_NO_MAIN(RecluseEditorApp);
#endif
ResultCode WxGuiContext::onRun(int c, char* argv[])
{
    I32 result = wxEntry(c, argv);
    return result;
}


ResultCode WxGuiContext::onSetUp()
{
    return 0;
}


ResultCode WxGuiContext::onTearDown()
{
    return 0;
}
} // Recluse