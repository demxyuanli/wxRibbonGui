#include "config/SvgIconManager.h"
#include "logger/Logger.h"
#include <wx/filename.h>
#include <wx/svg/wxsvg.h> 

SvgIconManager::SvgIconManager(const wxString& dir)
    : iconDir(dir)
{
    LoadIcons();
}

void SvgIconManager::LoadIcons()
{
    if (!wxDir::Exists(iconDir)) {
        LOG_ERR("SvgIconManager: Icon directory '%s' does not exist.", iconDir.ToStdString(), "SvgIconManager");
        return;
    }

    wxDir dir(iconDir);
    if (!dir.IsOpened()) {
        LOG_ERR("SvgIconManager: Could not open icon directory '%s'",  iconDir.ToStdString(), "SvgIconManager");
        return;
    }

    wxString filename;
    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (cont) {
        if (filename.EndsWith(".svg")) {
            wxFileName fn(filename);
            iconMap[fn.GetName()] = wxPathOnly(iconDir) + wxFILE_SEP_PATH + filename;
            LOG_DBG("SvgIconManager: Loaded icon '%s' from '%s'", fn.GetName().ToStdString(), (wxPathOnly(iconDir) + wxFILE_SEP_PATH + filename).ToStdString(), "SvgIconManager");
        }
        cont = dir.GetNext(&filename);
    }
}

wxBitmap SvgIconManager::GetIconBitmap(const wxString& name, const wxSize& size)
{
    auto it = iconMap.find(name);
    if (it != iconMap.end()) {
        wxSVGDocument svgDoc;
        if (svgDoc.Load(it->second)) {
            // Ensure size is valid
            wxSize validSize = size;
            if (validSize.GetWidth() <= 0 || validSize.GetHeight() <= 0) {
                // Attempt to get default size from SVG if specified size is invalid
                double width, height;
                if (svgDoc.GetSize(&width, &height)) {
                    validSize.SetWidth(static_cast<int>(width));
                    validSize.SetHeight(static_cast<int>(height));
                } else {
                    // Fallback to a default reasonable size if SVG has no intrinsic size
                    validSize.SetWidth(24); 
                    validSize.SetHeight(24);
                    LOG_WRN("SvgIconManager: Icon '%s' has no intrinsic size and no valid size provided. Falling back to %dx%d.", name.ToStdString(), std::string(validSize.GetWidth()), std::string(validSize.GetHeight()), "SvgIconManager");
                }
            }
            
            // Check again if validSize is still non-positive after attempting to get intrinsic size
            if (validSize.GetWidth() <= 0 || validSize.GetHeight() <= 0) {
                 LOG_ERR("SvgIconManager: Cannot render icon '%s' due to invalid size (%dx%d) after attempting to use intrinsic size.", name.ToStdString(), std::string(validSize.GetWidth()), std::string(validSize.GetHeight()), "SvgIconManager");
                 return wxBitmap(); // Return empty bitmap
            }

            return svgDoc.Render(validSize);
        }
        else {
            LOG_ERR("SvgIconManager: Failed to load SVG document for icon '%s' from path '%s'." + name.ToStdString(), it->second.ToStdString(), "SvgIconManager");
        }
    }
    else {
        LOG_WRN("SvgIconManager: Icon '%s' not found." + name.ToStdString(), "SvgIconManager");
    }
    return wxBitmap(); // Return an empty bitmap if icon not found or failed to load/render
}

bool SvgIconManager::HasIcon(const wxString& name) const
{
    return iconMap.count(name) > 0;
}

wxArrayString SvgIconManager::GetAvailableIcons() const
{
    wxArrayString names;
    for (const auto& pair : iconMap) {
        names.Add(pair.first);
    }
    return names;
}
