#include <wx/wx.h>
#include "MainApplication.h"
#include "config/ConfigManager.h"
#include "config/LoggerConfig.h"
#include "logger/Logger.h"
#include "FlatFrame.h"

bool MainApplication::OnInit()
{
    ConfigManager::getInstance().initialize("");
    
    LOG_INF("Starting application", "MainApplication");
    
    FlatFrame* frame = new FlatFrame("FlatUI Demo", wxPoint(50, 50), wxSize(800, 600)); // Initial size hint

    LOG_DBG("Frame created with initial size: " + 
            std::to_string(frame->GetSize().GetWidth()) + "x" + 
            std::to_string(frame->GetSize().GetHeight()), "MainApplication");

    frame->Show(true);
    LOG_DBG("Frame shown with size: " + 
            std::to_string(frame->GetSize().GetWidth()) + "x" + 
            std::to_string(frame->GetSize().GetHeight()), "MainApplication");

    return true;
}

wxIMPLEMENT_APP(MainApplication);