#ifndef FLATFRAME_H
#define FLATFRAME_H

#include "flatui/FlatUIFrame.h" // Changed from PlatUIFrame.h
#include <wx/srchctrl.h>  
#include "flatui/FlatUIBar.h" 
#include "flatui/FlatUIHomeMenu.h"

// IDs for specific FlatFrame controls/menu items remain here
// Any generic IDs could be moved to PlatUIFrame or a common constants file if needed
enum {
    ID_Menu_NewProject_MainFrame = wxID_HIGHEST + 100, 
    ID_Menu_OpenProject_MainFrame,
    ID_Menu_RecentFiles_MainFrame, 
    ID_UserProfile, 
    ID_SearchExecute, 
    ID_ShowUIHierarchy, 
    ID_Menu_PrintLayout_MainFrame,
    ID_ToggleFunctionSpace,
    ID_ToggleProfileSpace
};

// The ResizeMode enum is now defined in FlatUIFrame.h (the new base class header)
// So it should be removed from here to avoid redefinition.

class FlatFrame : public FlatUIFrame // Changed inheritance from PlatUIFrame
{
public:
    FlatFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    virtual ~FlatFrame();

    void LoadSVGIcons(wxWindow* parent, wxSizer* sizer);
    wxBitmap LoadHighQualityBitmap(const wxString& resourceName, const wxSize& targetSize);
    // Override mouse events IF FlatFrame needs specific behavior beyond PlatUIFrame
    // For example, to prevent dragging when m_homeMenu is shown.
    void OnLeftDown(wxMouseEvent& event) override;
    void OnMotion(wxMouseEvent& event) override;
    // OnLeftUp might not need override if base class behavior is sufficient

    // FlatFrame specific event handlers
    void OnButtonClick(wxCommandEvent& event);
    void OnMenuNewProject(wxCommandEvent& event);
    void OnMenuOpenProject(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event); // This could be common, but might have FlatFrame specific cleanup

    void OnSearchExecute(wxCommandEvent& event);
    void OnSearchTextEnter(wxCommandEvent& event);
    void OnUserProfile(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnToggleFunctionSpace(wxCommandEvent& event);
    void OnToggleProfileSpace(wxCommandEvent& event);
    void OnShowUIHierarchy(wxCommandEvent& event);

    void OnStartupTimer(wxTimerEvent& event);

    // Method to show UI hierarchy using UIHierarchyDebugger
    void ShowUIHierarchy();

    // Method for printing layout (uses LogUILayout from base class)
    void PrintUILayout(wxCommandEvent& event);
    
    // Note: IsPseudoMaximized, PseudoMaximize, RestoreFromPseudoMaximize are inherited
    // Note: LogUILayout is inherited

private:
    void InitializeUI(const wxSize& size); // Helper to create ribbon, panels, etc.

    // UI Elements specific to FlatFrame
    FlatUIBar* m_ribbon;
    wxTextCtrl* m_messageOutput; // For logging messages from FlatFrame UI interactions
    wxSearchCtrl* m_searchCtrl;
    FlatUIHomeMenu* m_homeMenu;

    // Note: Dragging/Resizing members (m_dragging, m_resizing, etc.) are now in PlatUIFrame
    // Note: m_borderThreshold, m_isPseudoMaximized, m_preMaximizeRect are now in PlatUIFrame

    wxDECLARE_EVENT_TABLE(); // Specific event table for FlatFrame for its own events
};

#endif // FLATFRAME_H 