#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "flatui/FlatFrame.h"
#include <wx/srchctrl.h>

class FlatUIBar;
// Forward declaration
class FlatUIHomeMenu;

enum { ID_Menu_NewProject_MainFrame = wxID_HIGHEST + 100, ID_Menu_OpenProject_MainFrame, ID_Menu_RecentFiles_MainFrame, ID_UserProfile, ID_SearchExecute, ID_ShowUIHierarchy, ID_Menu_PrintLayout_MainFrame };

class MainFrame : public FlatFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    virtual ~MainFrame();

    void OnButtonClick(wxCommandEvent& event);
    void OnMenuNewProject(wxCommandEvent& event);
    void OnMenuOpenProject(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event);

    void OnSearchExecute(wxCommandEvent& event);
    void OnSearchTextEnter(wxCommandEvent& event);
    void OnUserProfile(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnShowUIHierarchy(wxCommandEvent& event);

    void OnSetupUI(const wxSize& size);

    // Public method to show UI hierarchy
    void ShowUIHierarchy();

    // New public method for printing layout
    void PrintUILayout(wxCommandEvent& event);

    void LogUILayout(wxWindow* window = nullptr, int depth = 0);

protected:

    FlatUIBar* m_ribbon = nullptr;
    wxTextCtrl* m_messageOutput = nullptr;
    wxSearchCtrl* m_searchCtrl = nullptr;
    FlatUIHomeMenu* m_homeMenu = nullptr;

};

#endif // MAINFRAME_H 