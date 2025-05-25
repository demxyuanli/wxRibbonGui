#include "MainFrame.h"
#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIHomeMenu.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIGallery.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/UIHierarchyDebugger.h"
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/button.h>

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : FlatFrame(title, pos, size)
{
    SetDoubleBuffered(true);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)); // Set a distinct background for FlatFrame
    OnSetupUI();
}

MainFrame::~MainFrame(size)
{
}

void MainFrame::OnSetupUI(const wxSize& size)
{
    // FlatUIBar should have enough height to contain top bar and page content
    // Get standard height of the bar strip (top bar)
    int barHeight = FlatUIBar::GetBarHeight();
    // Give enough total height when creating, actual page size will be calculated in SetSize
    m_ribbon = new FlatUIBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 90));

    // Font for the entire ribbon and its direct custom children
    wxFont defaultFont = GetFlatUIDefaultFont();

    m_ribbon->SetDoubleBuffered(true);

    // Configure tab style
    m_ribbon->SetTabStyle(FlatUIBar::TabStyle::DEFAULT);  // You can also try UNDERLINE, BUTTON, or FLAT
    m_ribbon->SetTabBorderColour(wxColour(200, 200, 200));
    m_ribbon->SetActiveTabBackgroundColour(wxColour(255, 255, 255));
    m_ribbon->SetActiveTabTextColour(wxColour(50, 50, 50));
    m_ribbon->SetInactiveTabTextColour(wxColour(100, 100, 100));

    // Configure tab border style
    m_ribbon->SetTabBorderStyle(FlatUIBar::TabBorderStyle::SOLID);  // Try DASHED, DOTTED, DOUBLE, GROOVE, RIDGE, or ROUNDED
    m_ribbon->SetTabBorderWidths(2, 0, 1, 0);
    m_ribbon->SetTabBorderTopColour(wxColour(0, 120, 215));
    m_ribbon->SetTabCornerRadius(0);  // For ROUNDED style

    // Configure individual border colors and widths
    // m_ribbon->SetTabBorderTopColour(wxColour(0, 120, 215));     // Blue top border
    // m_ribbon->SetTabBorderBottomColour(wxColour(200, 200, 200)); // Gray bottom
    // m_ribbon->SetTabBorderLeftColour(wxColour(150, 150, 150));   // Gray left
    // m_ribbon->SetTabBorderRightColour(wxColour(150, 150, 150));  // Gray right

    // Set individual border widths
    // m_ribbon->SetTabBorderTopWidth(3);    // Thick top border
    // m_ribbon->SetTabBorderBottomWidth(1); // Thin bottom border
    // m_ribbon->SetTabBorderLeftWidth(2);   // Medium left border
    // m_ribbon->SetTabBorderRightWidth(2);  // Medium right border

    // Custom tab border widths (optional)
    // m_ribbon->SetTabBorderWidths(3, 0, 1, 1);  // Thicker top border

    m_ribbon->SetHomeButtonWidth(30);

    FlatUIHomeSpace* homeSpace = m_ribbon->GetHomeSpace(); // Assuming GetHomeSpace() exists and returns the correct parent
    if (homeSpace) {
        // Create and store the home menu instance
        m_homeMenu = new FlatUIHomeMenu(homeSpace, this);
        m_homeMenu->AddSeparator();
        m_homeMenu->AddMenuItem("&New Project...\tCtrl-N", ID_Menu_NewProject_MainFrame);
        m_homeMenu->AddSeparator();
        m_homeMenu->AddMenuItem("Show UI &Hierarchy\tCtrl-H", ID_ShowUIHierarchy);
        m_homeMenu->AddSeparator();
        m_homeMenu->AddMenuItem("Print Frame All wxCtr", ID_Menu_PrintLayout_MainFrame);
        m_homeMenu->BuildMenuLayout();
        homeSpace->SetHomeMenu(m_homeMenu); // Pass the created menu to HomeSpace
    }
    else {
        // Handle error: HomeSpace not available
        wxLogError("FlatUIHomeSpace is not available to attach the menu.");
    }

    // Use unified spacer management
    m_ribbon->AddSpaceSeparator(FlatUIBar::SPACER_TAB_FUNCTION, 15, false, true, true);  // width=15, auto-expand=true

    wxPanel* searchPanel = new wxPanel(m_ribbon);
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);

    m_searchCtrl = new wxSearchCtrl(searchPanel, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxSize(240, -1), wxTE_PROCESS_ENTER);
    m_searchCtrl->SetFont(defaultFont);
    m_searchCtrl->ShowSearchButton(true);
    m_searchCtrl->ShowCancelButton(true);
    wxBitmapButton* searchButton = new wxBitmapButton(searchPanel, ID_SearchExecute,
        wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON, wxSize(16, 16)));

    searchSizer->Add(m_searchCtrl, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);
    searchSizer->Add(searchButton, 0, wxALIGN_CENTER_VERTICAL);
    searchPanel->SetSizer(searchSizer);
    // It's good practice to set font on containers too if they might draw something or for consistency
    searchPanel->SetFont(defaultFont);

    m_ribbon->SetFunctionSpaceControl(searchPanel, 270);

    // Create ProfileSpace with user and settings buttons
    wxPanel* profilePanel = new wxPanel(m_ribbon);
    wxBoxSizer* profileSizer = new wxBoxSizer(wxHORIZONTAL);

    // User profile button
    wxBitmapButton* userButton = new wxBitmapButton(profilePanel, ID_UserProfile,
        wxArtProvider::GetBitmap(wxART_NEW, wxART_BUTTON, wxSize(16, 16)));
    userButton->SetToolTip("User Profile");

    // Settings button
    wxBitmapButton* settingsButton = new wxBitmapButton(profilePanel, wxID_PREFERENCES,
        wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_BUTTON, wxSize(16, 16)));
    settingsButton->SetToolTip("Settings");

    profileSizer->Add(userButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    profileSizer->Add(settingsButton, 0, wxALIGN_CENTER_VERTICAL);
    profilePanel->SetSizer(profileSizer);

    // Set the profile panel to ProfileSpace with a fixed width
    m_ribbon->SetProfileSpaceControl(profilePanel, 50); // Width enough for 2 buttons with spacing

    // Use unified spacer management
    m_ribbon->AddSpaceSeparator(FlatUIBar::SPACER_TAB_FUNCTION, 15, false, true, true);  // width=15, auto-expand=true
    m_ribbon->AddSpaceSeparator(FlatUIBar::SPACER_FUNCTION_PROFILE, 60, false, true, false);  // fixed width

    FlatUIPage* page1 = new FlatUIPage(m_ribbon, "Home");
    // page1 will inherit font from m_ribbon or should have it set in its own constructor if needed for direct drawing
    FlatUIPanel* panel1 = new FlatUIPanel(page1, "FirstPanel", wxHORIZONTAL);
    panel1->SetFont(FLATUI_DEFAULT_FONT_FACE_NAME); // Set font for panels

    //panel1->SetPanelBackgroundColour(wxColour(245, 245, 245));
    //panel1->SetBorderColour(wxColour(100, 100, 100));
    panel1->SetPanelBorderWidths(0, 0, 0, 1);  // Only bottom border
    panel1->SetHeaderStyle(PanelHeaderStyle::BOTTOM_CENTERED);
    panel1->SetHeaderColour(wxColour(230, 230, 230));
    panel1->SetHeaderTextColour(wxColour(60, 60, 60));

    // Set header border for panel1 - only top border
    panel1->SetHeaderBorderWidths(1, 0, 0, 0);  // top=2px, others=0
    panel1->SetHeaderBorderColour(wxColour(120, 120, 120));

    // Create button bar for Home page with explicit width
    FlatUIButtonBar* buttonBar1 = new FlatUIButtonBar(panel1);

    // buttonBar1 font already set in its constructor
    buttonBar1->AddButton(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_BUTTON, wxSize(16, 16)));
    buttonBar1->AddButton(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_BUTTON, wxSize(16, 16)));

    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(ID_Menu_NewProject_MainFrame, "&New Project...\tCtrl-N");
    fileMenu->Append(ID_Menu_OpenProject_MainFrame, "&Open Project...\tCtrl-O");
    wxMenu* recentFilesMenu = new wxMenu;    recentFilesMenu->Append(wxID_ANY, "File1.txt");
    recentFilesMenu->Append(wxID_ANY, "File2.cpp");
    fileMenu->AppendSubMenu(recentFilesMenu, "Recent &Files");
    fileMenu->AppendSeparator();

    buttonBar1->AddButton(wxID_ANY, "File Menu", wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_BUTTON, wxSize(16, 16)), fileMenu);

    panel1->AddButtonBar(buttonBar1, 0, wxEXPAND | wxALL, 5);

    FlatUIGallery* gallery1 = new FlatUIGallery(panel1);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    panel1->AddGallery(gallery1, 0, wxEXPAND | wxALL, 5);


    page1->AddPanel(panel1);

    FlatUIPanel* panel2 = new FlatUIPanel(page1, "SecondPanel", wxHORIZONTAL);
    panel2->SetFont(FLATUI_DEFAULT_FONT_FACE_NAME);
    panel2->SetPanelBorderWidths(0, 0, 0, 1);  // Only bottom border
    panel2->SetHeaderStyle(PanelHeaderStyle::BOTTOM_CENTERED);
    panel2->SetHeaderColour(wxColour(230, 230, 230));
    panel2->SetHeaderTextColour(wxColour(60, 60, 60));

    // Set header border for panel2 - top and bottom borders
    panel2->SetHeaderBorderWidths(1, 0, 0, 0);  // top=2px, others=0
    panel2->SetHeaderBorderColour(wxColour(120, 120, 120));

    FlatUIButtonBar* buttonBar2 = new FlatUIButtonBar(panel2);
    // buttonBar2 font set in constructor
    buttonBar2->AddButton(wxID_HELP, "Help", wxArtProvider::GetBitmap(wxART_HELP, wxART_BUTTON, wxSize(16, 16)));
    buttonBar2->AddButton(wxID_INFO, "Info", wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_BUTTON, wxSize(16, 16)));
    panel2->AddButtonBar(buttonBar2, 0, wxEXPAND | wxALL, 5);

    FlatUIGallery* gallery2 = new FlatUIGallery(panel2);
    gallery2->AddItem(wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery2->AddItem(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery2->AddItem(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    panel2->AddGallery(gallery2, 0, wxEXPAND | wxALL, 5);

    page1->AddPanel(panel2);
    m_ribbon->AddPage(page1);

    FlatUIPage* page3 = new FlatUIPage(m_ribbon, "Edit");
    FlatUIPanel* panel3 = new FlatUIPanel(page3, "EditPanel", wxHORIZONTAL);
    panel3->SetFont(defaultFont);
    FlatUIButtonBar* buttonBar3 = new FlatUIButtonBar(panel3);
    // buttonBar3 font set in constructor
    buttonBar3->AddButton(wxID_COPY, "Copy", wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON, wxSize(16, 16)));
    buttonBar3->AddButton(wxID_PASTE, "Paste", wxArtProvider::GetBitmap(wxART_PASTE, wxART_BUTTON, wxSize(16, 16)));
    panel3->AddButtonBar(buttonBar3);

    FlatUIGallery* gallery3 = new FlatUIGallery(panel3);
    gallery3->AddItem(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery3->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery3->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    panel3->AddGallery(gallery3, 0, wxEXPAND | wxALL, 5);


    page3->AddPanel(panel3);
    m_ribbon->AddPage(page3);

    FlatUIPage* page4 = new FlatUIPage(m_ribbon, "View");
    FlatUIPanel* panel4 = new FlatUIPanel(page4, "ViewPanel", wxHORIZONTAL);
    panel4->SetFont(defaultFont);
    FlatUIButtonBar* buttonBar4 = new FlatUIButtonBar(panel4);
    // buttonBar4 font set in constructor
    buttonBar4->AddButton(wxID_FIND, "Find", wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON, wxSize(16, 16)));
    buttonBar4->AddButton(wxID_SELECTALL, "SelectAll", wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON, wxSize(16, 16))); // Consider appropriate icon
    panel4->AddButtonBar(buttonBar4);
    page4->AddPanel(panel4);
    m_ribbon->AddPage(page4);

    FlatUIPage* page5 = new FlatUIPage(m_ribbon, "Help");
    FlatUIPanel* panel5 = new FlatUIPanel(page5, "HelpPanel", wxVERTICAL);
    panel5->SetFont(defaultFont);
    FlatUIButtonBar* buttonBar5 = new FlatUIButtonBar(panel5);
    // buttonBar5 font set in constructor
    buttonBar5->AddButton(wxID_ABOUT, "About", wxArtProvider::GetBitmap(wxART_WX_LOGO, wxART_BUTTON, wxSize(16, 16)));
    buttonBar5->AddButton(wxID_STOP, "Stop", wxArtProvider::GetBitmap(wxART_STOP, wxART_BUTTON, wxSize(16, 16)));
    panel5->AddButtonBar(buttonBar5);
    page5->AddPanel(panel5);
    m_ribbon->AddPage(page5);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_ribbon, 0, wxEXPAND);

    // Create a wrapper panel for the messageOutput to ensure margins are visible
    wxPanel* messagePanel = new wxPanel(this, wxID_ANY);
    messagePanel->SetBackgroundColour(this->GetBackgroundColour()); // Match FlatFrame background

    wxBoxSizer* messagePanelSizer = new wxBoxSizer(wxVERTICAL);
    m_messageOutput = new wxTextCtrl(messagePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    // m_messageOutput will expand to fill messagePanel without its own border/margin from this sizer
    messagePanelSizer->Add(m_messageOutput, 1, wxEXPAND);
    messagePanel->SetSizer(messagePanelSizer);

    // Add the wrapper panel to the main sizer with the desired margin
    sizer->Add(messagePanel, 1, wxEXPAND | wxALL, 10);

    SetSizer(sizer);
    SetClientSize(size);
    Layout();


    auto& eventManager = FlatUIEventManager::getInstance();

    eventManager.bindFrameEvents(this);

    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_OPEN);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_SAVE);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_COPY);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_PASTE);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_FIND);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_SELECTALL);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_ABOUT);
    eventManager.bindButtonEvent(this, &MainFrame::OnButtonClick, wxID_STOP);


    eventManager.bindButtonEvent(this, &MainFrame::OnSearchExecute, ID_SearchExecute);
    eventManager.bindButtonEvent(this, &MainFrame::OnUserProfile, ID_UserProfile);
    eventManager.bindButtonEvent(this, &MainFrame::OnSettings, wxID_PREFERENCES);

    m_searchCtrl->Bind(wxEVT_COMMAND_TEXT_ENTER, &MainFrame::OnSearchTextEnter, this);

    eventManager.bindMenuEvent(this, &MainFrame::OnMenuNewProject, ID_Menu_NewProject_MainFrame);
    eventManager.bindMenuEvent(this, &MainFrame::OnMenuOpenProject, ID_Menu_OpenProject_MainFrame);
    eventManager.bindMenuEvent(this, &MainFrame::OnShowUIHierarchy, ID_ShowUIHierarchy);
    eventManager.bindMenuEvent(this, &MainFrame::PrintUILayout, ID_Menu_PrintLayout_MainFrame);
    eventManager.bindMenuEvent(this, &MainFrame::OnMenuExit, wxID_EXIT);

    wxTimer* startupTimer = new wxTimer(this);
    this->Bind(wxEVT_TIMER, &FlatFrame::OnStartupTimer, this);
    startupTimer->StartOnce(1000);
}

void MainFrame::OnButtonClick(wxCommandEvent& event)
{
    wxString message;
    switch (event.GetId())
    {
    case wxID_OPEN: message = "Open Clicked"; break;
    case wxID_SAVE: message = "Save Clicked"; break;
    case wxID_COPY: message = "Copy Clicked"; break;
    case wxID_PASTE: message = "Paste Clicked"; break;
    case wxID_FIND: message = "Find Clicked"; break;
    case wxID_SELECTALL: message = "SelectAll Clicked"; break;
    case wxID_ABOUT: message = "About Clicked"; break;
    case wxID_STOP: message = "Stop Clicked"; break;
    default: message = "Unknown Clicked"; break;
    }
    m_messageOutput->AppendText(message + "\n");
}

void MainFrame::ShowUIHierarchy()
{
    m_messageOutput->Clear();
    m_messageOutput->AppendText("UI Hierarchy Debug:\n");

    UIHierarchyDebugger debugger;
    debugger.SetLogTextCtrl(m_messageOutput);
    debugger.PrintUIHierarchy(this);
}

void MainFrame::PrintUILayout(wxCommandEvent& event)
{
    m_messageOutput->Clear();
    m_messageOutput->AppendText("UI Layout Details:\\n\\n");

    wxLog* oldLog = wxLog::SetActiveTarget(new wxLogTextCtrl(m_messageOutput));
    LogUILayout(this); // Start logging from the FlatFrame itself
    wxLog::SetActiveTarget(oldLog);
    if (oldLog != wxLog::GetActiveTarget()) { // Ensure deletion only if it's not the active one (safety)
        delete oldLog;
    }
}


void MainFrame::LogUILayout(wxWindow* window, int depth)
{
    if (!window) {
        return;
    }

    wxString indent;
    for (int i = 0; i < depth; ++i) {
        indent += "  "; // Two spaces for indentation
    }

    wxString className = window->GetClassInfo()->GetClassName();
    wxString name = window->GetName();
    wxString idStr = wxString::Format("%d", window->GetId());
    wxPoint pos = window->GetPosition();
    wxSize size = window->GetSize();
    bool isShown = window->IsShown();

    wxString logMsg = wxString::Format("%s%s (ID: %s", indent, className, idStr);
    if (!name.IsEmpty()) {
        logMsg += wxString::Format(", Name: \"%s\"", name);
    }
    logMsg += wxString::Format(")\\n%s  Pos: (%d, %d), Size: (%d, %d), Shown: %s",
        indent, pos.x, pos.y, size.GetWidth(), size.GetHeight(),
        isShown ? "Yes" : "No");

    wxLogDebug(logMsg);

    wxWindowList children = window->GetChildren();
    for (wxWindowList::compatibility_iterator node = children.GetFirst(); node; node = node->GetNext()) {
        wxWindow* child = node->GetData();
        LogUILayout(child, depth + 1);
    }
}


void MainFrame::OnMenuNewProject(wxCommandEvent& event)
{
    m_messageOutput->AppendText("File Menu: New Project clicked\n");
}

void MainFrame::OnMenuOpenProject(wxCommandEvent& event)
{
    m_messageOutput->AppendText("File Menu: Open Project clicked\n");
}

void MainFrame::OnMenuExit(wxCommandEvent& event)
{
    // When exiting, if the menu is somehow still up, it will be destroyed with the frame.
    // No need to explicitly hide m_homeMenu here unless there's a specific reason (e.g., an animation).
    Close(true);
}

void MainFrame::OnSearchExecute(wxCommandEvent& event)
{
    wxString searchText = m_searchCtrl->GetValue();

    if (!searchText.IsEmpty())
    {
        m_messageOutput->AppendText("Do Searching: " + searchText + "\n");
    }
    else
    {
        m_messageOutput->AppendText("Input Search Keys\n");
    }
}

void MainFrame::OnSearchTextEnter(wxCommandEvent& event)
{
    OnSearchExecute(event);
}

void MainFrame::OnUserProfile(wxCommandEvent& event)
{
    m_messageOutput->AppendText("Open User Profile\n");
}

void MainFrame::OnSettings(wxCommandEvent& event)
{
    m_messageOutput->AppendText("Open Settings\n");
}

void MainFrame::OnShowUIHierarchy(wxCommandEvent& event)
{
    ShowUIHierarchy();
}