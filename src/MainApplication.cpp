#include <wx/wx.h>
#include "MainApplication.h"
#include "config/LoggerConfig.h"
#include "flatui/flatui.h"

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, wxSize(800,600))
    {
        FlatUIBar* ribbon = new FlatUIBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 100));

        FlatUIPage* page1 = new FlatUIPage(ribbon, "Home");
        FlatUIPanel* panel1 = new FlatUIPanel(page1, "HemePage", wxVERTICAL);
        FlatUIButtonBar* buttonBar1 = new FlatUIButtonBar(panel1);
        buttonBar1->AddButton(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_BUTTON));
        buttonBar1->AddButton(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_BUTTON));

        // Create a File menu for a dropdown button
        wxMenu* fileMenu = new wxMenu;
        enum { ID_Menu_NewProject = wxID_HIGHEST + 1, ID_Menu_OpenProject, ID_Menu_RecentFiles };
        fileMenu->Append(ID_Menu_NewProject, "&New Project...	Ctrl-N");
        fileMenu->Append(ID_Menu_OpenProject, "&Open Project...	Ctrl-O");
        wxMenu* recentFilesMenu = new wxMenu;
        recentFilesMenu->Append(wxID_ANY, "File1.txt");
        recentFilesMenu->Append(wxID_ANY, "File2.cpp");
        fileMenu->AppendSubMenu(recentFilesMenu, "Recent &Files");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT, "E&xit	Alt-X");

        // Add the dropdown button to buttonBar1
        buttonBar1->AddButton(wxID_ANY, "File Menu", wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_BUTTON), fileMenu);

        panel1->AddButtonBar(buttonBar1);

        FlatUIGallery* gallery1 = new FlatUIGallery(panel1);
        gallery1->AddItem(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER), wxID_ANY);
        gallery1->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER), wxID_ANY);
        panel1->AddGallery(gallery1, 0, wxEXPAND | wxALL, 5);
        page1->AddPanel(panel1);
        ribbon->AddPage(page1);

        FlatUIPage* page3 = new FlatUIPage(ribbon, "Edit");
        FlatUIPanel* panel3 = new FlatUIPanel(page3, "EditPanel", wxVERTICAL);
        FlatUIButtonBar* buttonBar3 = new FlatUIButtonBar(panel3);
        buttonBar3->AddButton(wxID_COPY, "Copy", wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON));
        buttonBar3->AddButton(wxID_PASTE, "Paste", wxArtProvider::GetBitmap(wxART_PASTE, wxART_BUTTON));
        panel3->AddButtonBar(buttonBar3);
        page3->AddPanel(panel3);
        ribbon->AddPage(page3);

        FlatUIPage* page4 = new FlatUIPage(ribbon, "View");
        FlatUIPanel* panel4 = new FlatUIPanel(page4, "ViewPanel");
        FlatUIButtonBar* buttonBar4 = new FlatUIButtonBar(panel4);
        buttonBar4->AddButton(wxID_FIND, "Find", wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON));
        buttonBar4->AddButton(wxID_SELECTALL, "SelectAll", wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON));
        panel4->AddButtonBar(buttonBar4);
        page4->AddPanel(panel4);
        ribbon->AddPage(page4);


        FlatUIPage* page5 = new FlatUIPage(ribbon, "Help");
        FlatUIPanel* panel5 = new FlatUIPanel(page5, "HelpPanel");
        FlatUIButtonBar* buttonBar5 = new FlatUIButtonBar(panel5);
        buttonBar5->AddButton(wxID_ABOUT, "About", wxArtProvider::GetBitmap(wxART_WX_LOGO, wxART_BUTTON));
        buttonBar5->AddButton(wxID_STOP, "Stop", wxArtProvider::GetBitmap(wxART_STOP, wxART_BUTTON));
        panel5->AddButtonBar(buttonBar5);
        page5->AddPanel(panel5);
        ribbon->AddPage(page5);

        // Add a text control for message output
        m_messageOutput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(ribbon, 0, wxEXPAND);
        sizer->Add(m_messageOutput, 1, wxEXPAND | wxALL, 10);
        SetSizer(sizer);

        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_OPEN);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_SAVE);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_COPY);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_PASTE);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_FIND);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_SELECTALL);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_ABOUT);
        Bind(wxEVT_BUTTON, &MainFrame::OnButtonClick, this, wxID_STOP);

        // Bind menu events
        Bind(wxEVT_MENU, &MainFrame::OnMenuNewProject, this, ID_Menu_NewProject);
        Bind(wxEVT_MENU, &MainFrame::OnMenuOpenProject, this, ID_Menu_OpenProject);
        Bind(wxEVT_MENU, &MainFrame::OnMenuExit, this, wxID_EXIT);
    }

private:
    void OnButtonClick(wxCommandEvent& event)
    {
        wxString message;
        switch (event.GetId())
        {
        case wxID_OPEN:
            message = "Open Clicked";
            break;
        case wxID_SAVE:
            message = "Save Clicked";
            break;
        case wxID_COPY:
            message = "Copy Clicked";
            break;
        case wxID_PASTE:
            message = "Paste Clicked";
            break;
        case wxID_FIND:
            message = "Find Clicked";
            break;
        case wxID_SELECTALL:
            message = "SelectAll Clicked";
            break;
        case wxID_ABOUT:
            message = "About Clicked";
            break;
        case wxID_STOP:
            message = "Stop Clicked";
            break;
        default:
            message = "Unknown Clicked";
            break;
        }
        // Output message to the text control
        m_messageOutput->AppendText(message + "\n");
    }

    // Menu event handlers
    void OnMenuNewProject(wxCommandEvent& event)
    {
        m_messageOutput->AppendText("File Menu: New Project clicked\n");
    }

    void OnMenuOpenProject(wxCommandEvent& event)
    {
        m_messageOutput->AppendText("File Menu: Open Project clicked\n");
    }

    void OnMenuExit(wxCommandEvent& event)
    {
        Close(true);
    }

    wxTextCtrl* m_messageOutput;
};

bool MainApplication::OnInit()
{
    ConfigManager::getInstance().initialize("");
    MainFrame* frame = new MainFrame("FlatUI Demo", wxPoint(50, 50), wxSize(450, 340));
    frame->Show(true);
    return true;
}
wxIMPLEMENT_APP(MainApplication);