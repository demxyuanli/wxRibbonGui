#ifndef SVG_ICON_MANAGER_H
#define SVG_ICON_MANAGER_H

#include <wx/wx.h>
#include <wx/dir.h>
#include <map>

/**
 * @class SvgIconManager
 * @brief A utility class to manage and load SVG icons from a local directory.
 * Provides functionality to retrieve wxBitmap for wxButton usage based on icon name and size.
 */
class SvgIconManager {
private:
    std::map<wxString, wxString> iconMap; // Maps icon names to file paths
    wxString iconDir; // Directory containing SVG files

    /**
     * @brief Loads all SVG files from the specified directory into the icon map.
     */
    void LoadIcons();

public:
    /**
     * @brief Constructor.
     * @param dir The directory containing SVG files.
     */
    SvgIconManager(const wxString& dir);

    /**
     * @brief Gets a wxBitmap for the specified icon name and size.
     * @param name The name of the icon (without .svg extension).
     * @param size The desired size of the bitmap.
     * @return wxBitmap containing the rendered SVG, or an empty bitmap if the icon is not found.
     */
    wxBitmap GetIconBitmap(const wxString& name, const wxSize& size);

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
};

#endif // SVG_ICON_MANAGER_H