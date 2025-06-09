#include "config/SvgIconManager.h"
#include "logger/Logger.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

// Static member definitions
std::unique_ptr<SvgIconManager> SvgIconManager::instance = nullptr;
wxString SvgIconManager::defaultIconDir = "";

SvgIconManager::SvgIconManager(const wxString& dir)
    : iconDir(dir)
{
    LoadIcons();
}

SvgIconManager& SvgIconManager::GetInstance()
{
    if (!instance) {
        if (defaultIconDir.IsEmpty()) {
            // Default to executable directory + icons/svg
            wxString exePath = wxStandardPaths::Get().GetExecutablePath();
            wxFileName exeFile(exePath);
            defaultIconDir = exeFile.GetPath() + wxFILE_SEP_PATH + "config" + wxFILE_SEP_PATH + "icons" + wxFILE_SEP_PATH + "svg";
        }
        instance = std::make_unique<SvgIconManager>(defaultIconDir);
    }
    return *instance;
}

void SvgIconManager::SetDefaultIconDirectory(const wxString& dir)
{
    defaultIconDir = dir;
    // Reset instance to reload with new directory
    instance.reset();
}

void SvgIconManager::LoadIcons()
{
    if (!wxDir::Exists(iconDir)) {
        LOG_ERR(wxString::Format("SvgIconManager: Icon directory '%s' does not exist.", iconDir.ToStdString()), "SvgIconManager");
        return;
    }

    wxDir dir(iconDir);
    if (!dir.IsOpened()) {
        LOG_ERR(wxString::Format("SvgIconManager: Could not open icon directory '%s'", iconDir.ToStdString()), "SvgIconManager");
        return;
    }

    wxString filename;
    bool cont = dir.GetFirst(&filename, "*.svg", wxDIR_FILES);
    while (cont) {
        wxFileName fn(filename);
        wxString fullPath = iconDir + wxFILE_SEP_PATH + filename;
        iconMap[fn.GetName()] = fullPath;
        LOG_DBG(wxString::Format("SvgIconManager: Loaded icon '%s' from '%s'", fn.GetName().ToStdString(), fullPath.ToStdString()), "SvgIconManager");
        cont = dir.GetNext(&filename);
    }
    
    LOG_INF(wxString::Format("SvgIconManager: Loaded %zu SVG icons from '%s'", iconMap.size(), iconDir.ToStdString()), "SvgIconManager");
}

wxString SvgIconManager::GetCacheKey(const wxString& name, const wxSize& size) const
{
    return wxString::Format("%s_%dx%d", name, size.GetWidth(), size.GetHeight());
}

wxBitmapBundle SvgIconManager::GetBitmapBundle(const wxString& name)
{
    // Check bundle cache first
    auto bundleIt = bundleCache.find(name);
    if (bundleIt != bundleCache.end()) {
        return bundleIt->second;
    }

    auto it = iconMap.find(name);
    if (it != iconMap.end()) {
        try {
            // Create bitmap bundle from SVG file
            wxBitmapBundle bundle = wxBitmapBundle::FromSVGFile(it->second, wxSize(16, 16));
            if (bundle.IsOk()) {
                // Cache the bundle
                bundleCache[name] = bundle;
                return bundle;
            } else {
                LOG_ERR(wxString::Format("SvgIconManager: Failed to create bitmap bundle for icon '%s' from path '%s'.",
                       name.ToStdString(), it->second.ToStdString()), "SvgIconManager");
            }
        } catch (const std::exception& e) {
            LOG_ERR(wxString::Format("SvgIconManager: Exception while loading SVG icon '%s': %s",
                   name.ToStdString(), e.what()), "SvgIconManager");
        } catch (...) {
            LOG_ERR(wxString::Format("SvgIconManager: Unknown exception while loading SVG icon '%s'",
                   name.ToStdString()), "SvgIconManager");
        }
    } else {
        LOG_WRN(wxString::Format("SvgIconManager: Icon '%s' not found.", name.ToStdString()), "SvgIconManager");
    }
    
    return wxBitmapBundle(); // Return empty bundle
}

wxBitmap SvgIconManager::GetIconBitmap(const wxString& name, const wxSize& size, bool useCache)
{
    // Check cache first if enabled
    if (useCache) {
        wxString cacheKey = GetCacheKey(name, size);
        auto cacheIt = iconCache.find(cacheKey);
        if (cacheIt != iconCache.end()) {
            return cacheIt->second;
        }
    }

    // Get bitmap bundle and extract bitmap at desired size
    wxBitmapBundle bundle = GetBitmapBundle(name);
    if (bundle.IsOk()) {
        wxBitmap bitmap = bundle.GetBitmap(size);
        if (bitmap.IsOk()) {
            // Cache the rendered bitmap if enabled
            if (useCache) {
                wxString cacheKey = GetCacheKey(name, size);
                iconCache[cacheKey] = bitmap;
            }
            return bitmap;
        } else {
            LOG_ERR(wxString::Format("SvgIconManager: Failed to get bitmap from bundle for icon '%s' at size %dx%d.",
                   name.ToStdString(), size.GetWidth(), size.GetHeight()), "SvgIconManager");
        }
    }
    
    return wxBitmap(); // Return empty bitmap
}

wxBitmap SvgIconManager::GetIconBitmapWithFallback(const wxString& name, const wxSize& size, const wxString& fallbackName)
{
    wxBitmap bitmap = GetIconBitmap(name, size);
    if (!bitmap.IsOk() && !fallbackName.IsEmpty() && fallbackName != name) {
        LOG_DBG(wxString::Format("SvgIconManager: Using fallback icon '%s' for missing icon '%s'",
               fallbackName.ToStdString(), name.ToStdString()), "SvgIconManager");
        bitmap = GetIconBitmap(fallbackName, size);
    }
    return bitmap;
}

wxBitmapBundle SvgIconManager::GetIconBundle(const wxString& name)
{
    return GetBitmapBundle(name);
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

void SvgIconManager::ClearCache()
{
    iconCache.clear();
    bundleCache.clear();
    LOG_DBG("SvgIconManager: Icon cache cleared", "SvgIconManager");
}

void SvgIconManager::PreloadCommonIcons(const wxSize& size)
{
    // Preload commonly used icons
    wxArrayString commonIcons;
    commonIcons.Add("home");
    commonIcons.Add("settings");
    commonIcons.Add("user");
    commonIcons.Add("file");
    commonIcons.Add("folder");
    commonIcons.Add("search");
    commonIcons.Add("open");
    commonIcons.Add("save");
    commonIcons.Add("filemenu");
    commonIcons.Add("copy");
    commonIcons.Add("paste");
    commonIcons.Add("find");
    commonIcons.Add("help");
    commonIcons.Add("person");
    commonIcons.Add("info");
    commonIcons.Add("about");
    commonIcons.Add("exit");
    
    for (const wxString& iconName : commonIcons) {
        GetIconBitmap(iconName, size, true); // This will cache the bitmap
    }
    
    LOG_INF(wxString::Format("SvgIconManager: Preloaded %zu common icons", commonIcons.size()), "SvgIconManager");
}