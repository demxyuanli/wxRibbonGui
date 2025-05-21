#include <wx/wx.h>
#include "MainApplication.h"
#include "config/LoggerConfig.h"
#include "flatui/FlatFrame.h"

bool MainApplication::OnInit()
{
    // ConfigManager::getInstance().initialize(""); // If used
    FlatFrame* frame = new FlatFrame("FlatUI Demo", wxPoint(50, 50), wxSize(800, 600)); // Initial size hint
    wxLogDebug(wxT("MainApplication::OnInit: After frame creation, Frame Size = %d x %d, Frame ClientSize = %d x %d"),
               frame->GetSize().GetWidth(), frame->GetSize().GetHeight(),
               frame->GetClientSize().GetWidth(), frame->GetClientSize().GetHeight());

    frame->Show(true);
    wxLogDebug(wxT("MainApplication::OnInit: After frame->Show(true), Frame Size = %d x %d, Frame ClientSize = %d x %d"),
               frame->GetSize().GetWidth(), frame->GetSize().GetHeight(),
               frame->GetClientSize().GetWidth(), frame->GetClientSize().GetHeight());

    // Optionally, try setting size again here if constructor settings don't stick
    // frame->SetClientSize(800, 600);
    // frame->Layout();
    // wxLogDebug(wxT("MainApplication::OnInit: After re-setting size, Frame Size = %d x %d, Frame ClientSize = %d x %d"),
    //            frame->GetSize().GetWidth(), frame->GetSize().GetHeight(),
    //            frame->GetClientSize().GetWidth(), frame->GetClientSize().GetHeight());

    return true;
}

wxIMPLEMENT_APP(MainApplication);