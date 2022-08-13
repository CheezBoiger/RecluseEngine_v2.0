//
#include "Recluse/Algorithms/Bubblesort.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/EditorSDK/EditorDeclarations.hpp"
#include "Recluse/Time.hpp"

#define R_EDITOR_CHAN "RecluseEditor"

#if 1
#define wxUSE_MENUBAR 1
#define wxUSE_MENUS 1
#define wxUSE_STATUSBAR 1

#include "wx/wxprec.h"
#include "wx/frame.h"
#include "wx/menu.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class RecluseEditorFrame : public wxFrame
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
        Close(true);
    }
    void onAbout(wxCommandEvent& ev) { }
    
private:
    wxMenu* m_menu;
    wxMenu* m_menuHelp;
    wxMenuBar* m_menuBar;
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
    m_menuBar->Append(m_menu, "File");
    m_menuBar->Append(m_menuHelp, "Help");

    SetMenuBar(m_menuBar);
    
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
    
    Bind(wxEVT_MENU, &RecluseEditorFrame::onHello, this, ID_HELLO);
    Bind(wxEVT_MENU, &RecluseEditorFrame::onExit, this, wxID_EXIT);
    //wxColour color(50, 50, 50, 255);
    //SetBackgroundColour(color);
    R_DEBUG(R_EDITOR_CHAN, "Initialized RecluseEditorFrame...");
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
    R_DEBUG(R_EDITOR_CHAN, "Created RecluseEditorFrame...");
    return true;
}


int RecluseEditorApp::OnExit()
{
    R_DEBUG(R_EDITOR_CHAN, "Destroyed RecluseEditorFrame...");
    return 0;
}

wxIMPLEMENT_APP_NO_MAIN(RecluseEditorApp);

#endif
int main(int c, char* argv[])
{
    Recluse::Log::initializeLoggingSystem();
    Recluse::RealtimeTick::initializeWatch(0, 0);

#if 1
    Recluse::I32 result = wxEntry(c, argv);
#endif
    Recluse::Log::destroyLoggingSystem();
    return 0;
}