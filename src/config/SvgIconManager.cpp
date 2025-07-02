#include "config/SvgIconManager.h"
#include "config/ThemeManager.h"
#include "logger/Logger.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/file.h>

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
    
    LOG_INF(wxString::Format("SvgIconManager: Loaded %u SVG icons from '%s'", (unsigned int)iconMap.size(), iconDir.ToStdString()), "SvgIconManager");
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
            // Get themed SVG content
            wxString themedSvgContent = GetThemedSvgContent(name);
            
            if (!themedSvgContent.IsEmpty()) {
                // Create bitmap bundle from themed SVG content
                wxBitmapBundle bundle = wxBitmapBundle::FromSVG(themedSvgContent.ToUTF8().data(), wxSize(16, 16));
                
            if (bundle.IsOk()) {
                // Cache the bundle
                bundleCache[name] = bundle;
                return bundle;
            } else {
                    LOG_WRN(wxString::Format("SvgIconManager: Failed to create bundle from themed SVG for '%s', trying original file.", name.ToStdString()), "SvgIconManager");
                    // Fallback to original file
                    bundle = wxBitmapBundle::FromSVGFile(it->second, wxSize(16, 16));
                    if (bundle.IsOk()) {
                        bundleCache[name] = bundle;
                        return bundle;
                    }
                }
            } else {
                // Fallback to original SVG file if theming failed
                wxBitmapBundle bundle = wxBitmapBundle::FromSVGFile(it->second, wxSize(16, 16));
                if (bundle.IsOk()) {
                    bundleCache[name] = bundle;
                    return bundle;
                }
            }
            
                LOG_ERR(wxString::Format("SvgIconManager: Failed to create bitmap bundle for icon '%s' from path '%s'.",
                       name.ToStdString(), it->second.ToStdString()), "SvgIconManager");
                   
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
    themedSvgCache.clear();
    LOG_DBG("SvgIconManager: All caches cleared", "SvgIconManager");
}

void SvgIconManager::ClearThemeCache()
{
    themedSvgCache.clear();
    // Also clear the rendered caches since they depend on themed SVG
    iconCache.clear();
    bundleCache.clear();
    LOG_DBG("SvgIconManager: Theme cache cleared", "SvgIconManager");
}

wxString SvgIconManager::ReadSvgFile(const wxString& filePath)
{
    if (!wxFile::Exists(filePath)) {
        LOG_WRN(wxString::Format("SvgIconManager: SVG file '%s' does not exist.", filePath.ToStdString()), "SvgIconManager");
        return wxEmptyString;
    }

    wxFile file(filePath, wxFile::read);
    if (!file.IsOpened()) {
        LOG_ERR(wxString::Format("SvgIconManager: Could not open SVG file '%s'.", filePath.ToStdString()), "SvgIconManager");
        return wxEmptyString;
    }

    wxString content;
    if (!file.ReadAll(&content)) {
        LOG_ERR(wxString::Format("SvgIconManager: Failed to read SVG file '%s'.", filePath.ToStdString()), "SvgIconManager");
        return wxEmptyString;
    }

    return content;
}

wxString SvgIconManager::ApplyThemeToSvg(const wxString& svgContent)
{
    if (svgContent.IsEmpty()) {
        return wxEmptyString;
    }

    wxString themedContent = svgContent;
    
    try {
        // Check if SVG theming is enabled
        bool svgThemeEnabled = CFG_INT("SvgThemeEnabled") != 0;
        LOG_DBG(wxString::Format("SvgIconManager: SVG theming enabled: %s", svgThemeEnabled ? "true" : "false"), "SvgIconManager");
        
        if (!svgThemeEnabled) {
            LOG_DBG("SvgIconManager: SVG theming is disabled", "SvgIconManager");
            return svgContent; // Return original content if theming is disabled
        }

        // Get theme colors
        wxColour primaryIconColor = CFG_COLOUR("SvgPrimaryIconColour");
        wxColour secondaryIconColor = CFG_COLOUR("SvgSecondaryIconColour");
        wxColour disabledIconColor = CFG_COLOUR("SvgDisabledIconColour");
        wxColour highlightIconColor = CFG_COLOUR("SvgHighlightIconColour");
        wxColour primaryBgColor = CFG_COLOUR("PrimaryContentBgColour");
        wxColour secondaryBgColor = CFG_COLOUR("SecondaryBackgroundColour");
        
        // Convert wxColour to hex strings
        wxString primaryIconHex = wxString::Format("#%02x%02x%02x", 
            primaryIconColor.Red(), primaryIconColor.Green(), primaryIconColor.Blue());
        
        LOG_DBG(wxString::Format("SvgIconManager: Primary icon color: %s (R:%d G:%d B:%d)", 
            primaryIconHex, primaryIconColor.Red(), primaryIconColor.Green(), primaryIconColor.Blue()), "SvgIconManager");
        wxString secondaryIconHex = wxString::Format("#%02x%02x%02x", 
            secondaryIconColor.Red(), secondaryIconColor.Green(), secondaryIconColor.Blue());
        wxString disabledIconHex = wxString::Format("#%02x%02x%02x", 
            disabledIconColor.Red(), disabledIconColor.Green(), disabledIconColor.Blue());
        wxString highlightIconHex = wxString::Format("#%02x%02x%02x", 
            highlightIconColor.Red(), highlightIconColor.Green(), highlightIconColor.Blue());
        wxString primaryBgHex = wxString::Format("#%02x%02x%02x", 
            primaryBgColor.Red(), primaryBgColor.Green(), primaryBgColor.Blue());
        wxString secondaryBgHex = wxString::Format("#%02x%02x%02x", 
            secondaryBgColor.Red(), secondaryBgColor.Green(), secondaryBgColor.Blue());

        // Replace common SVG color values with theme colors
        // Replace black colors with primary icon color
        themedContent.Replace("fill=\"#000000\"", wxString::Format("fill=\"%s\"", primaryIconHex));
        themedContent.Replace("fill=\"#000\"", wxString::Format("fill=\"%s\"", primaryIconHex));
        themedContent.Replace("fill=\"black\"", wxString::Format("fill=\"%s\"", primaryIconHex));
        themedContent.Replace("stroke=\"#000000\"", wxString::Format("stroke=\"%s\"", primaryIconHex));
        themedContent.Replace("stroke=\"#000\"", wxString::Format("stroke=\"%s\"", primaryIconHex));
        themedContent.Replace("stroke=\"black\"", wxString::Format("stroke=\"%s\"", primaryIconHex));
        
        // Replace currentColor with primary icon color (common in modern SVGs)
        themedContent.Replace("fill=\"currentColor\"", wxString::Format("fill=\"%s\"", primaryIconHex));
        themedContent.Replace("stroke=\"currentColor\"", wxString::Format("stroke=\"%s\"", primaryIconHex));
        
        // Replace common dark colors found in logs
        themedContent.Replace("fill=\"#202023\"", wxString::Format("fill=\"%s\"", primaryIconHex));
        themedContent.Replace("stroke=\"#202023\"", wxString::Format("stroke=\"%s\"", primaryIconHex));
        
        // Handle path elements without explicit fill attribute (they default to black)
        // Look for <path> tags without fill attribute and add primary icon color
        wxString pathPattern = "<path ";
        size_t pos = 0;
        wxString searchContent = themedContent.ToStdString();
        
        while (true) {
            size_t foundPos = searchContent.find(pathPattern.ToStdString(), pos);
            if (foundPos == std::string::npos) break;
            
            size_t endPos = searchContent.find(">", foundPos);
            if (endPos == std::string::npos) break;
            
            wxString pathTag = themedContent.Mid(foundPos, endPos - foundPos + 1);
            // Check if this path already has a fill attribute
            if (!pathTag.Contains("fill=")) {
                // Add fill attribute with primary icon color
                wxString newPathTag = pathTag;
                newPathTag.Replace(">", wxString::Format(" fill=\"%s\">", primaryIconHex));
                themedContent = themedContent.Left(foundPos) + newPathTag + themedContent.Mid(endPos + 1);
                searchContent = themedContent.ToStdString();
                pos = foundPos + newPathTag.length();
            } else {
                pos = endPos + 1;
            }
        }
        
        // Handle <g> elements without explicit fill attribute
        wxString gPattern = "<g>";
        themedContent.Replace(gPattern, wxString::Format("<g fill=\"%s\">", primaryIconHex));
        
        // Handle <g> elements with attributes but no fill
        wxString gAttrPattern = "<g ";
        pos = 0;
        searchContent = themedContent.ToStdString();
        
        while (true) {
            size_t foundPos = searchContent.find(gAttrPattern.ToStdString(), pos);
            if (foundPos == std::string::npos) break;
            
            size_t endPos = searchContent.find(">", foundPos);
            if (endPos == std::string::npos) break;
            
            wxString gTag = themedContent.Mid(foundPos, endPos - foundPos + 1);
            // Check if this g element already has a fill attribute
            if (!gTag.Contains("fill=")) {
                // Add fill attribute with primary icon color
                wxString newGTag = gTag;
                newGTag.Replace(">", wxString::Format(" fill=\"%s\">", primaryIconHex));
                themedContent = themedContent.Left(foundPos) + newGTag + themedContent.Mid(endPos + 1);
                searchContent = themedContent.ToStdString();
                pos = foundPos + newGTag.length();
            } else {
                pos = endPos + 1;
            }
        }

        // Replace dark gray colors with secondary icon color
        themedContent.Replace("fill=\"#333333\"", wxString::Format("fill=\"%s\"", secondaryIconHex));
        themedContent.Replace("fill=\"#333\"", wxString::Format("fill=\"%s\"", secondaryIconHex));
        themedContent.Replace("fill=\"#555555\"", wxString::Format("fill=\"%s\"", secondaryIconHex));
        themedContent.Replace("fill=\"#555\"", wxString::Format("fill=\"%s\"", secondaryIconHex));
        themedContent.Replace("stroke=\"#333333\"", wxString::Format("stroke=\"%s\"", secondaryIconHex));
        themedContent.Replace("stroke=\"#333\"", wxString::Format("stroke=\"%s\"", secondaryIconHex));
        themedContent.Replace("stroke=\"#555555\"", wxString::Format("stroke=\"%s\"", secondaryIconHex));
        themedContent.Replace("stroke=\"#555\"", wxString::Format("stroke=\"%s\"", secondaryIconHex));

        // Replace medium gray colors with disabled icon color
        themedContent.Replace("fill=\"#808080\"", wxString::Format("fill=\"%s\"", disabledIconHex));
        themedContent.Replace("fill=\"#888888\"", wxString::Format("fill=\"%s\"", disabledIconHex));
        themedContent.Replace("fill=\"#999999\"", wxString::Format("fill=\"%s\"", disabledIconHex));
        themedContent.Replace("stroke=\"#808080\"", wxString::Format("stroke=\"%s\"", disabledIconHex));
        themedContent.Replace("stroke=\"#888888\"", wxString::Format("stroke=\"%s\"", disabledIconHex));
        themedContent.Replace("stroke=\"#999999\"", wxString::Format("stroke=\"%s\"", disabledIconHex));

        // Replace blue/highlight colors with theme highlight color
        themedContent.Replace("fill=\"#0078d4\"", wxString::Format("fill=\"%s\"", highlightIconHex));
        themedContent.Replace("fill=\"#0066cc\"", wxString::Format("fill=\"%s\"", highlightIconHex));
        themedContent.Replace("fill=\"#007acc\"", wxString::Format("fill=\"%s\"", highlightIconHex));
        themedContent.Replace("stroke=\"#0078d4\"", wxString::Format("stroke=\"%s\"", highlightIconHex));
        themedContent.Replace("stroke=\"#0066cc\"", wxString::Format("stroke=\"%s\"", highlightIconHex));
        themedContent.Replace("stroke=\"#007acc\"", wxString::Format("stroke=\"%s\"", highlightIconHex));

        // Replace white/light backgrounds with theme background colors
        themedContent.Replace("fill=\"#ffffff\"", wxString::Format("fill=\"%s\"", secondaryBgHex));
        themedContent.Replace("fill=\"#fff\"", wxString::Format("fill=\"%s\"", secondaryBgHex));
        themedContent.Replace("fill=\"white\"", wxString::Format("fill=\"%s\"", secondaryBgHex));
        
        // Replace light gray backgrounds
        themedContent.Replace("fill=\"#f0f0f0\"", wxString::Format("fill=\"%s\"", primaryBgHex));
        themedContent.Replace("fill=\"#eeeeee\"", wxString::Format("fill=\"%s\"", primaryBgHex));
        themedContent.Replace("fill=\"#eee\"", wxString::Format("fill=\"%s\"", primaryBgHex));

        LOG_DBG(wxString::Format("SvgIconManager: Applied theme colors to SVG content. Original length: %d, Themed length: %d", 
            (int)svgContent.length(), (int)themedContent.length()), "SvgIconManager");
        
        // Log a sample of the themed content for debugging
        if (themedContent.length() > 100) {
            LOG_DBG(wxString::Format("SvgIconManager: Themed SVG sample: %s...", themedContent.Left(200)), "SvgIconManager");
        }
        
        LOG_DBG("SvgIconManager: Applied theme colors to SVG content", "SvgIconManager");
        
    } catch (const std::exception& e) {
        LOG_ERR(wxString::Format("SvgIconManager: Exception while applying theme to SVG: %s", e.what()), "SvgIconManager");
        return svgContent; // Return original content on error
    } catch (...) {
        LOG_ERR("SvgIconManager: Unknown exception while applying theme to SVG", "SvgIconManager");
        return svgContent; // Return original content on error
    }

    return themedContent;
}

wxString SvgIconManager::GetThemedSvgContent(const wxString& name)
{
    // Check themed SVG cache first
    auto cacheIt = themedSvgCache.find(name);
    if (cacheIt != themedSvgCache.end()) {
        return cacheIt->second;
    }

    auto it = iconMap.find(name);
    if (it != iconMap.end()) {
        // Read original SVG content
        wxString originalContent = ReadSvgFile(it->second);
        if (!originalContent.IsEmpty()) {
            LOG_DBG(wxString::Format("SvgIconManager: Original SVG content for '%s' (first 100 chars): %s", 
                name, originalContent.Left(100)), "SvgIconManager");
            
            // Apply theme colors
            wxString themedContent = ApplyThemeToSvg(originalContent);
            
            // Cache the themed content
            themedSvgCache[name] = themedContent;
            
            LOG_DBG(wxString::Format("SvgIconManager: Generated themed SVG for icon '%s'", name.ToStdString()), "SvgIconManager");
            return themedContent;
        } else {
            LOG_WRN(wxString::Format("SvgIconManager: Failed to read SVG file for icon '%s'", name.ToStdString()), "SvgIconManager");
        }
    } else {
        LOG_WRN(wxString::Format("SvgIconManager: Icon '%s' not found in icon map.", name.ToStdString()), "SvgIconManager");
    }

    return wxEmptyString;
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
    commonIcons.Add("thumbtack");
    commonIcons.Add("thumbtack");
    
    for (const wxString& iconName : commonIcons) {
        GetIconBitmap(iconName, size, true); // This will cache the bitmap
    }
    
    LOG_INF(wxString::Format("SvgIconManager: Preloaded %u common icons", (unsigned int)commonIcons.size()), "SvgIconManager");
}