//
#include "WxContext.hpp"
#include "Recluse/Messaging.hpp"

#include "MainEditorFrame.hpp"

#include "Recluse/Renderer/Renderer.hpp"

namespace Recluse {

#if 1

class RecluseEditorFrame : public wxFrame, public Editor::MainEditorFrame
{
public:

    enum Id 
    {
        ID_HELLO
    };

    RecluseEditorFrame();

    void onHello(wxCommandEvent& ev) { }
    void onExit(wxCommandEvent& ev)
    {
        m_panel->Close(true);
        delete m_panel;
        Close(true);
    }
    void onAbout(wxCommandEvent& ev) { }
    
private:
    wxMenu* m_menu;
    wxMenu* m_menuHelp;
    wxMenuBar* m_menuBar;
    wxPanel* m_panel;
};

RecluseEditorFrame::RecluseEditorFrame()
    : wxFrame(NULL, wxID_ANY, "Hello World")
{
    m_menu = new wxMenu;
    m_menu->Append(ID_HELLO, "HELLO...\tCNTRL-H", "Help string stuff...");
    m_menu->AppendSeparator();
    m_menu->Append(wxID_EXIT);

    m_menuHelp = new wxMenu;
    m_menuHelp->Append(wxID_ABOUT);

    m_menuBar = new wxMenuBar;
    m_menuBar->SetBackgroundColour(*wxRED);
    m_menuBar->Append(m_menu, "File");
    m_menuBar->Append(m_menuHelp, "Help");

    m_panel = new wxPanel(this);

    SetMenuBar(m_menuBar);
    
    CreateStatusBar();

    SetStatusText("Welcome to wxWidgets!");
    
    Bind(wxEVT_MENU, &RecluseEditorFrame::onHello, this, ID_HELLO);
    Bind(wxEVT_MENU, &RecluseEditorFrame::onExit, this, wxID_EXIT);
    wxColour color(50, 50, 50, 255);
    SetBackgroundColour(color);
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