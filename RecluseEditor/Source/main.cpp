//
#include "Recluse/Algorithms/Bubblesort.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/EditorSDK/EditorDeclarations.hpp"
#include "Recluse/RealtimeTick.hpp"

#define R_EDITOR_CHAN "RecluseEditor"

#if 0
#define wxUSE_MENUBAR 1
#define wxUSE_MENUS 1
#define wxUSE_STATUSBAR 1

#include "wx/wxprec.h"
#include "wx/frame.h"
#include "wx/menu.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class MyApp : public wxApp
{
public:
    virtual bool OnInit() override 
    {
    }
};

class MyFrame : public wxFrame
{
public:

    enum Id 
    {
        ID_HELLO
    };

    MyFrame();

    void onHello(wxCommandEvent& ev) { }
    void onExit(wxCommandEvent& ev)
    {
        Close(true);
    }
    void onAbout(wxCommandEvent& ev) { }
    
private:
    wxMenu m_menu;
    wxMenu m_menuHelp;
    wxMenuBar m_menuBar;
};

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Hello World")
{
    m_menu.Append(ID_HELLO, "%HELLO...\tCNTRL-H", "Help string stuff...");
    m_menu.AppendSeparator();
    m_menu.Append(wxID_EXIT);

    m_menuHelp.Append(wxID_ABOUT);

    m_menuBar.Append(&m_menu, "%File");
    m_menuBar.Append(&m_menuHelp, "%Help");

    SetMenuBar(&m_menuBar);
    
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
    
    Bind(wxEVT_MENU, &MyFrame::onHello, this, ID_HELLO);
    Bind(wxEVT_MENU, &MyFrame::onExit, this, wxID_EXIT);
}

wxIMPLEMENT_APP_NO_MAIN(MyApp);

#endif
int main(int c, char* argv[])
{

    Recluse::Log::initializeLoggingSystem();
    Recluse::RealtimeTick::initializeWatch(0, 0);

    R_VERBOSE(R_EDITOR_CHAN, "Hello World!!");

#if 0
    Recluse::I32 result = wxEntry(c, argv);
#endif
    Recluse::Log::destroyLoggingSystem();
    return 0;
}