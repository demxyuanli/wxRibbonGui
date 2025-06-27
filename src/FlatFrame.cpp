#include "FlatFrame.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIGallery.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIHomeMenu.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUICustomControl.h"
#include "flatui/UIHierarchyDebugger.h"
#include "config/ConstantsConfig.h"  
#include "config/SvgIconManager.h"
#include <wx/display.h>
#include "logger/Logger.h"
#include <wx/aui/aui.h>
#include <wx/dcbuffer.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>  // Add this line for wxStandardPaths
#include <wx/filename.h>  // Add this line for wxFileName
#include <wx/bmpbndl.h> 
#include <string>

#ifdef __WXMSW__
#include <windows.h>
#endif

#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)
#define CFG_FONTNAME() ConstantsConfig::getInstance().getDefaultFontFaceName()
#define CFG_DEFAULTFONT() ConstantsConfig::getInstance().getDefaultFont()

// These are now defined in FlatFrame.h as enum members.

// Event table for FlatFrame specific events
wxBEGIN_EVENT_TABLE(FlatFrame, FlatUIFrame) // Changed base class in macro
    EVT_COMMAND(wxID_ANY, wxEVT_PIN_STATE_CHANGED, FlatFrame::OnGlobalPinStateChanged)
    // Keep FlatFrame specific event bindings here
    // Mouse events (OnLeftDown, OnLeftUp, OnMotion) are handled by PlatUIFrame's table 
    // unless explicitly overridden and bound here with a different handler.
    // If FlatFrame::OnLeftDown (etc.) are meant to override, they are called virtually by PlatUIFrame's handler.
    // If they are completely different handlers for FlatFrame only, then they would need new EVT_LEFT_DOWN(FlatFrame::SpecificHandler)
wxEND_EVENT_TABLE()

FlatFrame::FlatFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : FlatUIFrame(NULL, wxID_ANY, title, pos, size, wxBORDER_NONE), // Call base class constructor
      m_ribbon(nullptr),
      m_messageOutput(nullptr),
      m_searchCtrl(nullptr), 
      m_homeMenu(nullptr)
{
    wxInitAllImageHandlers();
    // PlatUIFrame::InitFrameStyle() is called by base constructor.
    // FlatFrame specific UI initialization
    InitializeUI(size);

    // Event bindings specific to FlatFrame controls
    auto& eventManager = FlatUIEventManager::getInstance();
    eventManager.bindFrameEvents(this); // General frame events (close, etc.)

    // Button events (Open, Save, etc. are specific to FlatFrame's UI)
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_OPEN);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_SAVE);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_COPY);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_PASTE);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_FIND);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_SELECTALL);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_ABOUT);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_STOP);

    // Events for search, profile, settings (specific to FlatFrame's UI)
    eventManager.bindButtonEvent(this, &FlatFrame::OnSearchExecute, ID_SearchExecute);
    eventManager.bindButtonEvent(this, &FlatFrame::OnUserProfile, ID_UserProfile);
    eventManager.bindButtonEvent(this, &FlatFrame::OnSettings, wxID_PREFERENCES);
    eventManager.bindButtonEvent(this, &FlatFrame::OnToggleFunctionSpace, ID_ToggleFunctionSpace);
    eventManager.bindButtonEvent(this, &FlatFrame::OnToggleProfileSpace, ID_ToggleProfileSpace);

    if (m_searchCtrl) {
        m_searchCtrl->Bind(wxEVT_COMMAND_TEXT_ENTER, &FlatFrame::OnSearchTextEnter, this);
    }

    // Menu events (specific to FlatFrame's menu items)
    eventManager.bindMenuEvent(this, &FlatFrame::OnMenuNewProject, ID_Menu_NewProject_MainFrame);
    eventManager.bindMenuEvent(this, &FlatFrame::OnMenuOpenProject, ID_Menu_OpenProject_MainFrame);
    eventManager.bindMenuEvent(this, &FlatFrame::OnShowUIHierarchy, ID_ShowUIHierarchy);
    eventManager.bindMenuEvent(this, &FlatFrame::PrintUILayout, ID_Menu_PrintLayout_MainFrame);
    eventManager.bindMenuEvent(this, &FlatFrame::OnMenuExit, wxID_EXIT);

    // Startup timer (could be base, but often specific UI needs to be ready)
    wxTimer* startupTimer = new wxTimer(this);
    this->Bind(wxEVT_TIMER, &FlatFrame::OnStartupTimer, this);
    startupTimer->StartOnce(100); // Reduced for faster startup if UI is complex
}

FlatFrame::~FlatFrame()
{
    // m_homeMenu is a child window, wxWidgets handles its deletion.
    // Other child controls (m_ribbon, m_messageOutput, m_searchCtrl) are also managed by wxWidgets.
    wxLogDebug("FlatFrame destruction started.");
    
    // Unbind events to prevent access violations
    auto& eventManager = FlatUIEventManager::getInstance();
    eventManager.unbindFrameEvents(this);
    if (m_ribbon) {
        eventManager.unbindBarEvents(m_ribbon);
        FlatUIHomeSpace* homeSpace = m_ribbon->GetHomeSpace();
        if (homeSpace) {
            eventManager.unbindHomeSpaceEvents(homeSpace);
        }
    }
    
    wxLogDebug("FlatFrame destruction completed.");
}

void FlatFrame::InitializeUI(const wxSize& size)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

    int barHeight = FlatUIBar::GetBarHeight();
    m_ribbon = new FlatUIBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1, barHeight * 3));
    wxFont defaultFont = CFG_DEFAULTFONT();
    m_ribbon->SetDoubleBuffered(true);
    m_ribbon->SetTabStyle(FlatUIBar::TabStyle::DEFAULT);
    m_ribbon->SetTabBorderColour(wxColour(200, 200, 200));
    m_ribbon->SetActiveTabBackgroundColour(wxColour(255, 255, 255));
    m_ribbon->SetActiveTabTextColour(wxColour(50, 50, 50));
    m_ribbon->SetInactiveTabTextColour(wxColour(100, 100, 100));
    m_ribbon->SetTabBorderStyle(FlatUIBar::TabBorderStyle::SOLID);
    m_ribbon->SetTabBorderWidths(2, 0, 1, 1);
    m_ribbon->SetTabBorderTopColour(wxColour(0, 120, 215));
    m_ribbon->SetTabCornerRadius(0);
    m_ribbon->SetHomeButtonWidth(30);

    FlatUIHomeSpace* homeSpace = m_ribbon->GetHomeSpace();
    if (homeSpace) {
        m_homeMenu = new FlatUIHomeMenu(homeSpace, this);
        m_homeMenu->AddMenuItem("&New Project...\tCtrl-N", ID_Menu_NewProject_MainFrame);
        m_homeMenu->AddSeparator();
        m_homeMenu->AddMenuItem("Show UI &Hierarchy\tCtrl-H", ID_ShowUIHierarchy);
        m_homeMenu->AddSeparator();
        m_homeMenu->AddMenuItem("Print Frame All wxCtr", ID_Menu_PrintLayout_MainFrame);
        m_homeMenu->BuildMenuLayout();
        homeSpace->SetHomeMenu(m_homeMenu);
    }
    else {
        wxLogError("FlatUIHomeSpace is not available to attach the menu.");
    }

    m_ribbon->AddSpaceSeparator(FlatUIBar::SPACER_TAB_FUNCTION, 30, false, true, true);

    wxPanel* searchPanel = new wxPanel(m_ribbon);
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    m_searchCtrl = new wxSearchCtrl(searchPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(240, -1), wxTE_PROCESS_ENTER);
    m_searchCtrl->SetFont(defaultFont);
    m_searchCtrl->ShowSearchButton(true);
    m_searchCtrl->ShowCancelButton(true);
    wxBitmapButton* searchButton = new wxBitmapButton(searchPanel, ID_SearchExecute, SVG_ICON("search", wxSize(16, 16)));
    searchSizer->Add(m_searchCtrl, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);
    searchSizer->Add(searchButton, 0, wxALIGN_CENTER_VERTICAL);
    searchPanel->SetSizer(searchSizer);
    searchPanel->SetFont(defaultFont);
    m_ribbon->SetFunctionSpaceControl(searchPanel, 270);

    wxPanel* profilePanel = new wxPanel(m_ribbon);
    wxBoxSizer* profileSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBitmapButton* userButton = new wxBitmapButton(profilePanel, ID_UserProfile, SVG_ICON("user",wxSize(16, 16)));
    userButton->SetToolTip("User Profile");
    wxBitmapButton* settingsButton = new wxBitmapButton(profilePanel, wxID_PREFERENCES, SVG_ICON("settings", wxSize(16, 16)));
    settingsButton->SetToolTip("Settings");
    profileSizer->Add(userButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    profileSizer->Add(settingsButton, 0, wxALIGN_CENTER_VERTICAL);
    profilePanel->SetSizer(profileSizer);
    m_ribbon->SetProfileSpaceControl(profilePanel, 60);

    m_ribbon->AddSpaceSeparator(FlatUIBar::SPACER_FUNCTION_PROFILE, 30, false, true, true);

    // Store reference to search panel
    m_searchPanel = searchPanel;
    // Store reference to profile panel  
    m_profilePanel = profilePanel;

    FlatUIPage* page1 = new FlatUIPage(m_ribbon, "Home");
    FlatUIPanel* panel1 = new FlatUIPanel(page1, "FirstPanel", wxHORIZONTAL);
    panel1->SetFont(CFG_DEFAULTFONT()); 
    panel1->SetPanelBorderWidths(0, 0, 0, 1);
    panel1->SetHeaderStyle(PanelHeaderStyle::BOTTOM_CENTERED);
    panel1->SetHeaderColour(*wxWHITE);
    panel1->SetHeaderTextColour(wxColour(120, 120, 120));
    panel1->SetHeaderBorderWidths(0, 0, 0, 0);
    FlatUIButtonBar* buttonBar1 = new FlatUIButtonBar(panel1);
	buttonBar1->SetDisplayStyle(ButtonDisplayStyle::ICON_ONLY);
    wxBitmap fileMenuBmp("IDP_FILEMENU", wxBITMAP_TYPE_PNG_RESOURCE); 
    buttonBar1->AddButton(wxID_OPEN, "Open", SVG_ICON("open", wxSize(16, 16)));
    buttonBar1->AddButton(wxID_SAVE, "Save", SVG_ICON("save", wxSize(16, 16)));
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(ID_Menu_NewProject_MainFrame, "&New Project...\tCtrl-N");
    fileMenu->Append(ID_Menu_OpenProject_MainFrame, "&Open Project...\tCtrl-O");
    wxMenu* recentFilesMenu = new wxMenu;    recentFilesMenu->Append(wxID_ANY, "File1.txt");
    recentFilesMenu->Append(wxID_ANY, "File2.cpp");
    fileMenu->AppendSubMenu(recentFilesMenu, "Recent &Files");
    fileMenu->AppendSeparator();
    buttonBar1->AddButton(wxID_ANY, "File Menu", SVG_ICON("filemenu", wxSize(16, 16)), fileMenu);
    panel1->AddButtonBar(buttonBar1, 0, wxEXPAND | wxALL, 5);
    FlatUIGallery* gallery1 = new FlatUIGallery(panel1);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)), wxID_ANY);
    panel1->AddGallery(gallery1, 0, wxEXPAND | wxALL, 5);
    page1->AddPanel(panel1);

    FlatUIPanel* panel2 = new FlatUIPanel(page1, "SecondPanel", wxHORIZONTAL);
    panel2->SetFont(CFG_DEFAULTFONT());
    panel2->SetPanelBorderWidths(0, 0, 0, 1);
    panel2->SetHeaderStyle(PanelHeaderStyle::BOTTOM_CENTERED);
    panel2->SetHeaderColour(*wxWHITE);
    panel2->SetHeaderTextColour(wxColour(120, 120, 120));
    panel2->SetHeaderBorderWidths(0, 0, 0, 0);
    FlatUIButtonBar* buttonBar2 = new FlatUIButtonBar(panel2);
    buttonBar2->AddButton(wxID_HELP, "Help", SVG_ICON("help", wxSize(16, 16)));
    buttonBar2->AddButton(wxID_INFO, "Info", SVG_ICON("info", wxSize(16, 16)));
    panel2->AddButtonBar(buttonBar2, 0, wxEXPAND | wxALL, 5);
    FlatUIButtonBar* toggleBar = new FlatUIButtonBar(panel2);
    toggleBar->SetDisplayStyle(ButtonDisplayStyle::TEXT_ONLY);
    toggleBar->AddButton(ID_ToggleFunctionSpace, "ToggleFunc");
    toggleBar->AddButton(ID_ToggleProfileSpace, "ToggleProf");
    panel2->AddButtonBar(toggleBar, 0, wxEXPAND | wxALL, 5);
    page1->AddPanel(panel2);
    m_ribbon->AddPage(page1);

    FlatUIPage* page3 = new FlatUIPage(m_ribbon, "Edit");
    FlatUIPanel* panel3 = new FlatUIPanel(page3, "EditPanel", wxHORIZONTAL);
    FlatUIButtonBar* buttonBar3 = new FlatUIButtonBar(panel3);
    wxBitmap copyBmp("IDP_COPY", wxBITMAP_TYPE_PNG_RESOURCE);
    wxBitmap pasteBmp("IDP_PASTE", wxBITMAP_TYPE_PNG_RESOURCE);
    buttonBar3->AddButton(wxID_COPY, "Copy", copyBmp);
    buttonBar3->AddButton(wxID_PASTE, "Paste", pasteBmp);
    panel3->AddButtonBar(buttonBar3);
    page3->AddPanel(panel3);
    m_ribbon->AddPage(page3);

    FlatUIPage* page4 = new FlatUIPage(m_ribbon, "View");
    FlatUIPanel* panel4 = new FlatUIPanel(page4, "ViewPanel", wxHORIZONTAL);
    panel4->SetFont(defaultFont);
    FlatUIButtonBar* buttonBar4 = new FlatUIButtonBar(panel4);
    wxBitmap findBmp("IDP_FIND", wxBITMAP_TYPE_PNG_RESOURCE);
    wxBitmap selectAllBmp("IDP_SELECT", wxBITMAP_TYPE_PNG_RESOURCE);
    buttonBar4->AddButton(wxID_FIND, "Find", findBmp);
    buttonBar4->AddButton(wxID_SELECTALL, "Select", selectAllBmp);
    panel4->AddButtonBar(buttonBar4);
    page4->AddPanel(panel4);
    m_ribbon->AddPage(page4);

    FlatUIPage* page5 = new FlatUIPage(m_ribbon, "Help");
    FlatUIPanel* panel5 = new FlatUIPanel(page5, "HelpPanel", wxVERTICAL);
    panel5->SetFont(defaultFont);
    FlatUIButtonBar* buttonBar5 = new FlatUIButtonBar(panel5);
    wxBitmap aboutBmp("IDP_ABOUT", wxBITMAP_TYPE_PNG_RESOURCE);
    wxBitmap stopBmp("IDP_STOP", wxBITMAP_TYPE_PNG_RESOURCE);
    buttonBar5->AddButton(wxID_ABOUT, "About", aboutBmp);
    buttonBar5->AddButton(wxID_STOP, "Stop", stopBmp);
    panel5->AddButtonBar(buttonBar5);
    page5->AddPanel(panel5);
    m_ribbon->AddPage(page5);

    // Create main layout with horizontal splitter
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_ribbon, 0, wxEXPAND | wxALL, 2);

    // Create horizontal splitter for content area
    wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    splitter->SetMinimumPaneSize(200);

    // Create SVG display panel (left side)
    wxPanel* svgPanel = new wxPanel(splitter, wxID_ANY);
    svgPanel->SetBackgroundColour(wxColour(245, 245, 245));
    wxBoxSizer* svgSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add title for SVG area
    wxStaticText* svgTitle = new wxStaticText(svgPanel, wxID_ANY, "SVG Icons Display");
    svgTitle->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    svgSizer->Add(svgTitle, 0, wxALL | wxALIGN_CENTER, 10);
    
    // Create scrolled window for SVG icons
    wxScrolledWindow* svgScrolled = new wxScrolledWindow(svgPanel, wxID_ANY);
    svgScrolled->SetScrollRate(5, 5);
    svgScrolled->SetBackgroundColour(*wxWHITE);
    
    // Create sizer for SVG icons (grid layout)
    wxFlexGridSizer* svgGridSizer = new wxFlexGridSizer(0, 3, 10, 10); // 3 columns
    
    // Add some sample SVG icons
    LoadSVGIcons(svgScrolled, svgGridSizer);
    
    svgScrolled->SetSizer(svgGridSizer);
    svgSizer->Add(svgScrolled, 1, wxEXPAND | wxALL, 5);
    svgPanel->SetSizer(svgSizer);

    // Create message panel (right side)
    wxPanel* messagePanel = new wxPanel(splitter, wxID_ANY);
    messagePanel->SetBackgroundColour(this->GetBackgroundColour());
    wxBoxSizer* messagePanelSizer = new wxBoxSizer(wxVERTICAL);
    m_messageOutput = new wxTextCtrl(messagePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    messagePanelSizer->Add(m_messageOutput, 1, wxEXPAND);
    messagePanel->SetSizer(messagePanelSizer);

    // Split the window
    splitter->SplitVertically(svgPanel, messagePanel, 300); // SVG panel width = 300px
    
    mainSizer->Add(splitter, 1, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
    SetClientSize(size); // Default size
    Layout();

    int ribbonMinHeight = FlatUIBar::GetBarHeight() + CFG_INT("PanelTargetHeight") + 10;
    m_ribbon->SetMinSize(wxSize(-1, ribbonMinHeight));

    Layout();
}


wxWindow* FlatFrame::GetFunctionSpaceControl() const
{
    return m_searchPanel;
}

wxWindow* FlatFrame::GetProfileSpaceControl() const
{
    return m_profilePanel;
}

void FlatFrame::LoadSVGIcons(wxWindow* parent, wxSizer* sizer)
{
    // Get the executable directory for SVG file paths
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exeFile(exePath);
    wxString exeDir = exeFile.GetPath();

    // List of SVG files to load
    wxArrayString svgFiles;
    svgFiles.Add("config/icons/svg/home.svg");
    svgFiles.Add("config/icons/svg/settings.svg");
    svgFiles.Add("config/icons/svg/user.svg");
    svgFiles.Add("config/icons/svg/file.svg");
    svgFiles.Add("config/icons/svg/folder.svg");
    svgFiles.Add("config/icons/svg/search.svg");

    for (const wxString& svgFile : svgFiles)
    {
        wxString fullPath = exeDir + wxFILE_SEP_PATH + svgFile;

        // Create a panel for each SVG icon
        wxPanel* iconPanel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(80, 100));
        iconPanel->SetBackgroundColour(*wxWHITE);
        wxBoxSizer* iconSizer = new wxBoxSizer(wxVERTICAL);

        // Try to load SVG file
        if (wxFileExists(fullPath))
        {
            try
            {
                wxBitmapBundle svgBundle = wxBitmapBundle::FromSVGFile(fullPath, wxSize(16, 16));
                wxStaticBitmap* bitmap = new wxStaticBitmap(iconPanel, wxID_ANY, svgBundle);
                iconSizer->Add(bitmap, 0, wxALIGN_CENTER | wxALL, 5);

                wxFileName fn(fullPath);
                wxStaticText* label = new wxStaticText(iconPanel, wxID_ANY, fn.GetName());
                label->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                iconSizer->Add(label, 0, wxALIGN_CENTER | wxALL, 2);
            }
            catch (const std::exception& e)
            {
                // Create error placeholder
                wxStaticText* errorText = new wxStaticText(iconPanel, wxID_ANY, "Error\nLoading\nSVG");
                errorText->SetForegroundColour(*wxRED);
                iconSizer->Add(errorText, 1, wxALIGN_CENTER | wxALL, 5); 

                wxLogError("Failed to load SVG: %s - %s", fullPath, e.what());
            }
        }
        else
        {
            // Create placeholder for missing file
            wxStaticText* missingText = new wxStaticText(iconPanel, wxID_ANY, "SVG\nNot\nFound");
            missingText->SetForegroundColour(wxColour(128, 128, 128));
            iconSizer->Add(missingText, 1, wxALIGN_CENTER | wxALL, 5);

            wxFileName fn(fullPath);
            wxStaticText* label = new wxStaticText(iconPanel, wxID_ANY, fn.GetName());
            label->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            label->SetForegroundColour(wxColour(128, 128, 128));
            iconSizer->Add(label, 0, wxALIGN_CENTER | wxALL, 2);
        }

        iconPanel->SetSizer(iconSizer);
        sizer->Add(iconPanel, 0, wxALL, 5);
    }
}

wxBitmap FlatFrame::LoadHighQualityBitmap(const wxString& resourceName, const wxSize& targetSize) {
    wxBitmap bitmap(resourceName, wxBITMAP_TYPE_PNG_RESOURCE);
    if (bitmap.IsOk() && (bitmap.GetWidth() != targetSize.x || bitmap.GetHeight() != targetSize.y)) {
        wxImage image = bitmap.ConvertToImage();
        image = image.Scale(targetSize.x, targetSize.y, wxIMAGE_QUALITY_HIGH);
        return wxBitmap(image);
    }
    return bitmap;
}

// Override OnLeftDown to prevent dragging if home menu is shown
void FlatFrame::OnLeftDown(wxMouseEvent& event)
{
    if (m_homeMenu && m_homeMenu->IsShown()) {
        event.Skip(); // Allow menu to handle event, prevent frame dragging/resizing
        return;
    }
    // Call base class implementation for actual dragging/resizing logic
    FlatUIFrame::OnLeftDown(event);
}

// Override OnMotion to prevent cursor changes if home menu is shown
void FlatFrame::OnMotion(wxMouseEvent& event)
{
    if (m_homeMenu && m_homeMenu->IsShown()) {
        SetCursor(wxCursor(wxCURSOR_ARROW)); // Ensure default cursor
        event.Skip();
        return;
    }
    // Call base class implementation for cursor updates and rubber banding
    FlatUIFrame::OnMotion(event);
}

// OnLeftUp can often use the base class implementation directly if no special conditions
// void FlatFrame::OnLeftUp(wxMouseEvent& event) { PlatUIFrame::OnLeftUp(event); }

void FlatFrame::OnButtonClick(wxCommandEvent& event)
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
    if (m_messageOutput) m_messageOutput->AppendText(message + "\n");
}

void FlatFrame::OnMenuNewProject(wxCommandEvent& event)
{
    if (m_messageOutput) m_messageOutput->AppendText("File Menu: New Project clicked\n");
}

void FlatFrame::OnMenuOpenProject(wxCommandEvent& event)
{
    if (m_messageOutput) m_messageOutput->AppendText("File Menu: Open Project clicked\n");
}

void FlatFrame::OnMenuExit(wxCommandEvent& event)
{
    Close(true);
}

void FlatFrame::OnStartupTimer(wxTimerEvent& event)
{
    // Example: Forcing a refresh on the first page of the ribbon
    if (m_ribbon) {
        m_ribbon->Refresh();
        if (m_ribbon->GetPageCount() > 0) {
            FlatUIPage* page = m_ribbon->GetPage(0);
            if (page) {
                page->Show();
                page->Layout();
                page->Refresh();
                wxLogDebug("Force refreshed first page: %s", page->GetLabel());
            }
        }
    }
    // Initial UI Hierarchy debug log (optional)
    // UIHierarchyDebugger debugger;
    // debugger.PrintUIHierarchy(this);
}

void FlatFrame::OnSearchExecute(wxCommandEvent& event)
{
    if (!m_searchCtrl || !m_messageOutput) return;
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

void FlatFrame::OnSearchTextEnter(wxCommandEvent& event)
{
    OnSearchExecute(event); // Simply call the other handler
}

void FlatFrame::OnUserProfile(wxCommandEvent& event)
{
    if (m_messageOutput) m_messageOutput->AppendText("Open User Profile\n");
}

void FlatFrame::OnSettings(wxCommandEvent& event)
{
    if (m_messageOutput) m_messageOutput->AppendText("Open Settings\n");
}

void FlatFrame::OnToggleFunctionSpace(wxCommandEvent& event)
{
    if (m_ribbon) m_ribbon->ToggleFunctionSpaceVisibility();
}

void FlatFrame::OnToggleProfileSpace(wxCommandEvent& event)
{
    if (m_ribbon) m_ribbon->ToggleProfileSpaceVisibility();
}

void FlatFrame::OnShowUIHierarchy(wxCommandEvent& event)
{
    ShowUIHierarchy();
}

void FlatFrame::ShowUIHierarchy()
{
    if (!m_messageOutput) return;
    m_messageOutput->Clear();
    m_messageOutput->AppendText("UI Hierarchy Debug:\n");

    UIHierarchyDebugger debugger;
    debugger.SetLogTextCtrl(m_messageOutput); // Direct log to our text control
    debugger.PrintUIHierarchy(this);          // Debug this FlatFrame instance
}

void FlatFrame::PrintUILayout(wxCommandEvent& event)
{
    if (!m_messageOutput) return;
    m_messageOutput->Clear();
    m_messageOutput->AppendText("UI Layout Details:\n\n");

    wxLog* oldLog = wxLog::SetActiveTarget(new wxLogTextCtrl(m_messageOutput));
    LogUILayout(this); // Call inherited LogUILayout, starting from this FlatFrame instance
    wxLog::SetActiveTarget(oldLog);
    if (oldLog != wxLog::GetActiveTarget()) { 
    }
}

void FlatFrame::OnGlobalPinStateChanged(wxCommandEvent& event)
{
    if (!m_ribbon) {
        event.Skip();
        return;
    }

    bool isPinned = event.GetInt() != 0;

    if (isPinned) {
        // Restore original min height for ribbon
        int ribbonMinHeight = FlatUIBar::GetBarHeight() + CFG_INT("PanelTargetHeight") + 10;
        m_ribbon->SetMinSize(wxSize(-1, ribbonMinHeight));
    } else {
        // Set collapsed min height for ribbon
        int unpinnedHeight = ConstantsConfig::getInstance().getIntValue("BarUnpinnedHeight");
        m_ribbon->SetMinSize(wxSize(-1, unpinnedHeight));
    }

    if (GetSizer()) {
        GetSizer()->Layout();
    }
    Refresh();
    event.Skip();
}


