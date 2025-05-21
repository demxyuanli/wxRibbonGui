#ifndef FLATFRAME_H
#define FLATFRAME_H

#include <wx/wx.h>
#include <wx/srchctrl.h>  
#include "flatui/FlatUIBar.h" 


enum {
    ID_Menu_NewProject_MainFrame = wxID_HIGHEST + 100,
    ID_Menu_OpenProject_MainFrame,
    ID_Menu_RecentFiles_MainFrame,
    ID_UserProfile,         
    ID_SearchExecute         
};

// Enum to represent window edges/corners for resizing
enum class ResizeMode {
    NONE,
    LEFT,
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    RIGHT,
    BOTTOM_RIGHT,
    BOTTOM,
    BOTTOM_LEFT
};

class FlatFrame : public wxFrame
{
public:
    FlatFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    virtual ~FlatFrame(); // Good practice to have a virtual destructor
    // Event Handlers
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);

    void OnButtonClick(wxCommandEvent& event);
    void OnMenuNewProject(wxCommandEvent& event);
    void OnMenuOpenProject(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event);
    
    void OnSearchExecute(wxCommandEvent& event);
    void OnSearchTextEnter(wxCommandEvent& event);
    void OnUserProfile(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);

    void OnStartupTimer(wxTimerEvent& event);
    void DebugUIHierarchy(wxWindow* window = nullptr, int depth = 0);

private:
    // Helper methods for resizing
    ResizeMode GetResizeModeForPosition(const wxPoint& clientPos);
    void UpdateCursorForResizeMode(ResizeMode mode);
    
    void DrawRubberBand(const wxRect& rect);
    void EraseRubberBand();

    // UI Elements
    FlatUIBar* m_ribbon; // Example: if ribbon needs to be accessed by other methods
    wxTextCtrl* m_messageOutput;
    wxSearchCtrl* m_searchCtrl; 

    // Member variables for custom dragging
    bool m_dragging = false;
    wxPoint m_dragStartPos; // For window move, relative to window's top-left

    // Member variables for custom resizing
    bool m_resizing = false;
    ResizeMode m_resizeMode = ResizeMode::NONE;
    wxPoint m_resizeStartMouseScreenPos; // For window resize, initial mouse pos in screen coords
    wxRect m_resizeStartWindowRect;   // For window resize, initial window rect in screen coords
    wxRect m_currentRubberBandRect;   
    bool m_rubberBandVisible = false; 
    int m_borderThreshold = 8;     // Pixels to detect border proximity
};

#endif // FLATFRAME_H 