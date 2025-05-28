#include <wx/wx.h>
#include <cstdio>  // 用于 sscanf
#include <string>
#include "MainApplication.h"
#include "config/ConfigManager.h"
#include "config/LoggerConfig.h"
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"
#include "FlatFrame.h"

bool MainApplication::OnInit()
{
    ConfigManager& cm = ConfigManager::getInstance();
    cm.initialize("");
    ConstantsConfig::getInstance().initialize(cm);
    
    LOG_INF("Starting application", "MainApplication");
    
    std::string titleStr = cm.getString("MainApplication", "MainFrameTitle", "FlatUI Demo");
    wxString title(titleStr);
    std::string sizeStr = cm.getString("MainApplication", "MainFrameSize", "1200,700");
    int fw = 1200, fh = 700;
    sscanf(sizeStr.c_str(), "%d,%d", &fw, &fh);
    wxSize fsize(fw, fh);
    FlatFrame* frame = new FlatFrame(title, wxDefaultPosition, fsize);
    std::string posStr = cm.getString("MainApplication", "MainFramePosition", "Center");
    if (posStr == "Center") {
        frame->Centre();
    } else {
        int fx = 0, fy = 0;
        if (sscanf(posStr.c_str(), "%d,%d", &fx, &fy) == 2) {
            frame->SetPosition(wxPoint(fx, fy));
        }
    }

    LOG_DBG("Frame created with initial size: " + 
            std::to_string(frame->GetSize().GetWidth()) + "x" + 
            std::to_string(frame->GetSize().GetHeight()), "MainApplication");

    frame->Show(true);

    return true;
}

wxIMPLEMENT_APP(MainApplication);