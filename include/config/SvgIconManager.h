#ifndef SVG_ICON_MANAGER_H
#define SVG_ICON_MANAGER_H

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/bmpbndl.h>  // Use wxBitmapBundle instead of wxSVG
#include <map>
#include <memory>

/**
 * @class SvgIconManager
 * @brief Enhanced utility class to manage and load SVG icons using wxBitmapBundle.
 * Provides functionality to retrieve wxBitmap for wxButton usage based on icon name and size.
 * Supports caching and singleton pattern for better performance.
 */
class SvgIconManager {
private:
    std::map<wxString, wxString> iconMap; // Maps icon names to file paths
    std::map<wxString, wxBitmap> iconCache; // Cache for rendered bitmaps
    std::map<wxString, wxBitmapBundle> bundleCache; // Cache for bitmap bundles
    wxString iconDir; // Directory containing SVG files
    static std::unique_ptr<SvgIconManager> instance;
    static wxString defaultIconDir;

    /**
     * @brief Loads all SVG files from the specified directory into the icon map.
     */
    void LoadIcons();

    /**
     * @brief Generates cache key for bitmap caching.
     */
    wxString GetCacheKey(const wxString& name, const wxSize& size) const;

    /**
     * @brief Gets or creates a bitmap bundle for the specified icon.
     */
    wxBitmapBundle GetBitmapBundle(const wxString& name);

public:
    /**
     * @brief Constructor.
     * @param dir The directory containing SVG files.
     */
    SvgIconManager(const wxString& dir);

    /**
     * @brief Gets the singleton instance.
     */
    static SvgIconManager& GetInstance();

    /**
     * @brief Sets the default icon directory for the singleton instance.
     */
    static void SetDefaultIconDirectory(const wxString& dir);

    /**
     * @brief Gets a wxBitmap for the specified icon name and size.
     * @param name The name of the icon (without .svg extension).
     * @param size The desired size of the bitmap.
     * @param useCache Whether to use cached bitmaps for better performance.
     * @return wxBitmap containing the rendered SVG, or an empty bitmap if the icon is not found.
     */
    wxBitmap GetIconBitmap(const wxString& name, const wxSize& size, bool useCache = true);

    /**
     * @brief Gets a wxBitmap with fallback to default icon if not found.
     * @param name The name of the icon (without .svg extension).
     * @param size The desired size of the bitmap.
     * @param fallbackName Fallback icon name if primary icon is not found.
     * @return wxBitmap containing the rendered SVG.
     */
    wxBitmap GetIconBitmapWithFallback(const wxString& name, const wxSize& size, const wxString& fallbackName = "default");

    /**
     * @brief Gets a wxBitmapBundle for the specified icon name.
     * @param name The name of the icon (without .svg extension).
     * @return wxBitmapBundle containing the SVG, or an empty bundle if not found.
     */
    wxBitmapBundle GetIconBundle(const wxString& name);

    /**
     * @brief Checks if an icon exists in the manager.
     * @param name The name of the icon to check.
     * @return True if the icon exists, false otherwise.
     */
    bool HasIcon(const wxString& name) const;

    /**
     * @brief Gets the list of available icon names.
     * @return A wxArrayString containing all icon names.
     */
    wxArrayString GetAvailableIcons() const;

    /**
     * @brief Clears the bitmap cache.
     */
    void ClearCache();

    /**
     * @brief Preloads commonly used icons into cache.
     */
    void PreloadCommonIcons(const wxSize& size);
};

// Convenience macros for easy icon access
#define SVG_ICON(name, size) SvgIconManager::GetInstance().GetIconBitmap(name, size)
#define SVG_ICON_FALLBACK(name, size, fallback) SvgIconManager::GetInstance().GetIconBitmapWithFallback(name, size, fallback)
#define SVG_BUNDLE(name) SvgIconManager::GetInstance().GetIconBundle(name)

#endif // SVG_ICON_MANAGER_H