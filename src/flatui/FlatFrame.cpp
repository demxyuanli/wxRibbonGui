#include "flatui/FlatFrame.h" 
#include "flatui/FlatUIPanel.h" 
#include "flatui/FlatUIPage.h" 
#include "flatui/FlatUIButtonBar.h" 
#include "flatui/FlatUIGallery.h" 
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUICustomControl.h"

#ifdef __WXMSW__
#include <windows.h>
#endif

FlatFrame::FlatFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(NULL, wxID_ANY, title, pos, size, wxBORDER_NONE)
{
    SetDoubleBuffered(true);
    
    // FlatUIBar should have enough height to contain top bar and page content
    // Get standard height of the bar strip (top bar)
    int barHeight = FlatUIBar::GetBarHeight();
    // Give enough total height when creating, actual page size will be calculated in SetSize
    m_ribbon = new FlatUIBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1, barHeight * 2));
    
    m_ribbon->SetDoubleBuffered(true);

    wxPanel* searchPanel = new wxPanel(m_ribbon);
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    
    m_searchCtrl = new wxSearchCtrl(searchPanel, wxID_ANY, wxEmptyString, 
                   wxDefaultPosition, wxSize(240, -1), wxTE_PROCESS_ENTER);
    m_searchCtrl->ShowSearchButton(true);
    m_searchCtrl->ShowCancelButton(true);
    wxBitmapButton* searchButton = new wxBitmapButton(searchPanel, ID_SearchExecute, 
                                   wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON));
    
    searchSizer->Add(m_searchCtrl, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);
    searchSizer->Add(searchButton, 0, wxALIGN_CENTER_VERTICAL);
    searchPanel->SetSizer(searchSizer);
    
    m_ribbon->SetFunctionSpaceControl(searchPanel, 270);
    
    wxPanel* profilePanel = new wxPanel(m_ribbon);
    wxBoxSizer* profileSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxBitmapButton* userButton = new wxBitmapButton(profilePanel, ID_UserProfile, 
                                wxArtProvider::GetBitmap(wxART_HELP_SIDE_PANEL, wxART_BUTTON));
    
    wxBitmapButton* settingsButton = new wxBitmapButton(profilePanel, wxID_PREFERENCES, 
                                    wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_BUTTON));
    
    profileSizer->Add(userButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    profileSizer->Add(settingsButton, 0, wxALIGN_CENTER_VERTICAL);
    profilePanel->SetSizer(profileSizer);
    
    m_ribbon->SetProfileSpaceControl(profilePanel, 60);
    
    m_ribbon->SetTabFunctionSpacer(15, false);
    m_ribbon->SetTabFunctionSpacerAutoExpand(true);
    
    m_ribbon->SetFunctionProfileSpacer(60, false);
    m_ribbon->SetFunctionProfileSpacerAutoExpand(false);

    FlatUIPage* page1 = new FlatUIPage(m_ribbon, "Home");
    FlatUIPanel* panel1 = new FlatUIPanel(page1, "HemePage", wxVERTICAL);
    
    // Create button bar for Home page with explicit width
    FlatUIButtonBar* buttonBar1 = new FlatUIButtonBar(panel1);
    // Add Open button
    buttonBar1->AddButton(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_BUTTON));
    // Add Save button
    buttonBar1->AddButton(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_BUTTON));

    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(ID_Menu_NewProject_MainFrame, "&New Project...\tCtrl-N");
    fileMenu->Append(ID_Menu_OpenProject_MainFrame, "&Open Project...\tCtrl-O");
    wxMenu* recentFilesMenu = new wxMenu;
    recentFilesMenu->Append(wxID_ANY, "File1.txt");
    recentFilesMenu->Append(wxID_ANY, "File2.cpp");
    fileMenu->AppendSubMenu(recentFilesMenu, "Recent &Files");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X");
    
    m_ribbon->SetHomeButtonMenu(fileMenu);
    m_ribbon->SetHomeButtonWidth(30); 
    
    // Add File Menu button
    buttonBar1->AddButton(wxID_ANY, "File Menu", wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_BUTTON), fileMenu);
    
    // Add the button bar to the panel with explicit sizing flags
    panel1->AddButtonBar(buttonBar1, 0, wxEXPAND | wxALL, 5);

    FlatUIGallery* gallery1 = new FlatUIGallery(panel1);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER), wxID_ANY);
    gallery1->AddItem(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER), wxID_ANY);
    panel1->AddGallery(gallery1, 0, wxEXPAND | wxALL, 5);
    page1->AddPanel(panel1);
    m_ribbon->AddPage(page1);

    FlatUIPage* page3 = new FlatUIPage(m_ribbon, "Edit");
    FlatUIPanel* panel3 = new FlatUIPanel(page3, "EditPanel", wxVERTICAL);
    FlatUIButtonBar* buttonBar3 = new FlatUIButtonBar(panel3);
    buttonBar3->AddButton(wxID_COPY, "Copy", wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON));
    buttonBar3->AddButton(wxID_PASTE, "Paste", wxArtProvider::GetBitmap(wxART_PASTE, wxART_BUTTON));
    panel3->AddButtonBar(buttonBar3);
    page3->AddPanel(panel3);
    m_ribbon->AddPage(page3);

    FlatUIPage* page4 = new FlatUIPage(m_ribbon, "View");
    FlatUIPanel* panel4 = new FlatUIPanel(page4, "ViewPanel", wxVERTICAL);
    FlatUIButtonBar* buttonBar4 = new FlatUIButtonBar(panel4);
    buttonBar4->AddButton(wxID_FIND, "Find", wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON));
    buttonBar4->AddButton(wxID_SELECTALL, "SelectAll", wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON)); // Consider appropriate icon
    panel4->AddButtonBar(buttonBar4);
    page4->AddPanel(panel4);
    m_ribbon->AddPage(page4);

    FlatUIPage* page5 = new FlatUIPage(m_ribbon, "Help");
    FlatUIPanel* panel5 = new FlatUIPanel(page5, "HelpPanel", wxVERTICAL);
    FlatUIButtonBar* buttonBar5 = new FlatUIButtonBar(panel5);
    buttonBar5->AddButton(wxID_ABOUT, "About", wxArtProvider::GetBitmap(wxART_WX_LOGO, wxART_BUTTON));
    buttonBar5->AddButton(wxID_STOP, "Stop", wxArtProvider::GetBitmap(wxART_STOP, wxART_BUTTON));
    panel5->AddButtonBar(buttonBar5);
    page5->AddPanel(panel5);
    m_ribbon->AddPage(page5);

    m_messageOutput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    // Allow ribbon to occupy a proportion of vertical space, not just using minimum height
    sizer->Add(m_ribbon, 1, wxEXPAND);
    sizer->Add(m_messageOutput, 1, wxEXPAND | wxALL, 10);
    SetSizer(sizer); // Use SetSizer instead of SetSizerAndFit to avoid automatic resizing
    SetClientSize(800, 600);
    Layout();
    
    // Explicitly set ribbon size to ensure it has enough space to display content
    int ribbonMinHeight = FlatUIBar::GetBarHeight() * 5; // Give adequate height
    m_ribbon->SetMinSize(wxSize(-1, ribbonMinHeight));
    m_ribbon->InvalidateBestSize();
    Layout(); // Force layout update

    auto& eventManager = FlatUIEventManager::getInstance();
    
    eventManager.bindFrameEvents(this);
    
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_OPEN);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_SAVE);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_COPY);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_PASTE);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_FIND);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_SELECTALL);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_ABOUT);
    eventManager.bindButtonEvent(this, &FlatFrame::OnButtonClick, wxID_STOP);
    

    eventManager.bindButtonEvent(this, &FlatFrame::OnSearchExecute, ID_SearchExecute);
    eventManager.bindButtonEvent(this, &FlatFrame::OnUserProfile, ID_UserProfile);
    eventManager.bindButtonEvent(this, &FlatFrame::OnSettings, wxID_PREFERENCES);
    
    m_searchCtrl->Bind(wxEVT_COMMAND_TEXT_ENTER, &FlatFrame::OnSearchTextEnter, this);
    
    eventManager.bindMenuEvent(this, &FlatFrame::OnMenuNewProject, ID_Menu_NewProject_MainFrame);
    eventManager.bindMenuEvent(this, &FlatFrame::OnMenuOpenProject, ID_Menu_OpenProject_MainFrame);
    eventManager.bindMenuEvent(this, &FlatFrame::OnMenuExit, wxID_EXIT);
    
    wxTimer* startupTimer = new wxTimer(this);
    this->Bind(wxEVT_TIMER, &FlatFrame::OnStartupTimer, this);
    startupTimer->StartOnce(1000);
}

FlatFrame::~FlatFrame()
{
    // It's good practice to unbind events, especially if frame can be recreated
    // or if menus are managed and could be deleted before the frame.
    // However, wxWidgets often handles this for child windows and bound events.
    // For manually created menus passed to buttons, ensure their lifetime is managed.
    // wxMenu* fileMenu = buttonBar1->GetButtonMenu(some_id_or_index); // Need a way to get the menu
    // delete fileMenu; // if MainFrame owns it
}

void FlatFrame::OnLeftDown(wxMouseEvent& event)
{
    wxWindow* eventSource = dynamic_cast<wxWindow*>(event.GetEventObject());
    
    bool isFromSpacerControl = false;
    if (eventSource != this) {
        isFromSpacerControl = true;
        m_dragStartPos = event.GetPosition(); 
    }
    
    ResizeMode hoverMode = GetResizeModeForPosition(event.GetPosition());
    if (hoverMode != ResizeMode::NONE && !isFromSpacerControl)
    {
        m_resizing = true;
        m_resizeMode = hoverMode;
        m_resizeStartMouseScreenPos = wxGetMousePosition();
        m_resizeStartWindowRect = GetScreenRect();
        
        if (!HasCapture()) {
            CaptureMouse();
        }
    }
    else
    {
        m_dragging = true;
        if (!isFromSpacerControl) {
            m_dragStartPos = event.GetPosition();
        }
        m_resizeStartWindowRect = GetScreenRect();
        
        if (!HasCapture()) {
            CaptureMouse();
        }
    }
    event.Skip();
}

void FlatFrame::OnLeftUp(wxMouseEvent& event)
{
    wxWindow* eventSource = dynamic_cast<wxWindow*>(event.GetEventObject());
    bool isFromSpacerControl = (eventSource != this);
    
    if (isFromSpacerControl) {
        wxLogDebug("FlatFrame: Recived SpacerControl Mouse Left Up");
    }
    
    if (m_dragging)
    {
        if (m_rubberBandVisible)
        {
            EraseRubberBand();
        }
        
        wxPoint mousePosOnScreen = wxGetMousePosition();
        int newX = mousePosOnScreen.x - m_dragStartPos.x;
        int newY = mousePosOnScreen.y - m_dragStartPos.y;
        
        wxLogDebug("FlatFrame: Move Window to (%d,%d)", newX, newY);
        Move(newX, newY);
        m_dragging = false;
        
        // 确保释放鼠标
        if (HasCapture())
        {
            wxLogDebug("FlatFrame: Release Mouse");
            ReleaseMouse();
        }
    }
    else if (m_resizing)
    {
        if (m_rubberBandVisible)
        {
            EraseRubberBand();
        }
        wxPoint currentMouseScreenPos = wxGetMousePosition();
        int dx = currentMouseScreenPos.x - m_resizeStartMouseScreenPos.x;
        int dy = currentMouseScreenPos.y - m_resizeStartMouseScreenPos.y;
        wxRect newRect = m_resizeStartWindowRect;
        int minWidth = GetMinWidth() > 0 ? GetMinWidth() : 100;
        int minHeight = GetMinHeight() > 0 ? GetMinHeight() : 100;

        switch (m_resizeMode)
        {
            case ResizeMode::LEFT:
                newRect.x += dx;
                newRect.width -= dx;
                if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
                break;
            case ResizeMode::RIGHT:
                newRect.width += dx;
                if (newRect.width < minWidth) newRect.width = minWidth;
                break;
            case ResizeMode::TOP:
                newRect.y += dy;
                newRect.height -= dy;
                if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
                break;
            case ResizeMode::BOTTOM:
                newRect.height += dy;
                if (newRect.height < minHeight) newRect.height = minHeight;
                break;
            case ResizeMode::TOP_LEFT:
                newRect.x += dx; newRect.width -= dx;
                newRect.y += dy; newRect.height -= dy;
                if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
                if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
                break;
            case ResizeMode::TOP_RIGHT:
                newRect.width += dx;
                newRect.y += dy; newRect.height -= dy;
                if (newRect.width < minWidth) newRect.width = minWidth;
                if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
                break;
            case ResizeMode::BOTTOM_LEFT:
                newRect.x += dx; newRect.width -= dx;
                newRect.height += dy;
                if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
                if (newRect.height < minHeight) newRect.height = minHeight;
                break;
            case ResizeMode::BOTTOM_RIGHT:
                newRect.width += dx; newRect.height += dy;
                if (newRect.width < minWidth) newRect.width = minWidth;
                if (newRect.height < minHeight) newRect.height = minHeight;
                break;
            case ResizeMode::NONE: break;
        }
        SetSize(newRect);
        Layout();
        if (m_ribbon) {
            m_ribbon->Update();
            m_ribbon->Refresh(true);
            
            if (m_ribbon->GetPageCount() > 0) {
                size_t activePageIndex = m_ribbon->GetActivePage();
                FlatUIPage* activePage = m_ribbon->GetPage(activePageIndex);
                if (activePage && activePage->IsShown()) {
                    activePage->Update();
                    activePage->Refresh(true);
                }
            }
        }
        
        Refresh(true);
        Update();
        
        m_resizing = false;
        m_resizeMode = ResizeMode::NONE;
        
        if (HasCapture())
        {
            wxLogDebug("FlatFrame: Release Mouse");
            ReleaseMouse();
        }
        UpdateCursorForResizeMode(ResizeMode::NONE);
    }
    event.Skip();
}

void FlatFrame::OnMotion(wxMouseEvent& event)
{
    wxWindow* eventSource = dynamic_cast<wxWindow*>(event.GetEventObject());
    bool isFromSpacerControl = (eventSource != this);
    
    if (isFromSpacerControl && event.LeftIsDown()) {
        m_dragging = true;
        wxLogDebug("FlatFrame: Left Is Down");
        
        if (!HasCapture()) {
            CaptureMouse();
            wxLogDebug("FlatFrame: Moving Capture Mouse");
        }
    }
    
    if (m_dragging && event.Dragging() && event.LeftIsDown())
    {
        if (m_rubberBandVisible)
        {
            EraseRubberBand();
        }
        
        wxPoint mousePosOnScreen = wxGetMousePosition();
        wxRect newRect(
            mousePosOnScreen.x - m_dragStartPos.x,
            mousePosOnScreen.y - m_dragStartPos.y,
            m_resizeStartWindowRect.GetWidth(),
            m_resizeStartWindowRect.GetHeight()
        );
        
        DrawRubberBand(newRect);
    }
    else if (m_resizing && event.Dragging() && event.LeftIsDown())
    {
        if (m_rubberBandVisible)
        {
            EraseRubberBand();
        }
        
        wxPoint currentMouseScreenPos = wxGetMousePosition();
        int dx = currentMouseScreenPos.x - m_resizeStartMouseScreenPos.x;
        int dy = currentMouseScreenPos.y - m_resizeStartMouseScreenPos.y;
        wxRect newRect = m_resizeStartWindowRect;
        int minWidth = GetMinWidth() > 0 ? GetMinWidth() : 100;
        int minHeight = GetMinHeight() > 0 ? GetMinHeight() : 100;

        switch (m_resizeMode)
        {
            case ResizeMode::LEFT:
                newRect.x += dx;
                newRect.width -= dx;
                if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
                break;
            case ResizeMode::RIGHT:
                newRect.width += dx;
                if (newRect.width < minWidth) newRect.width = minWidth;
                break;
            case ResizeMode::TOP:
                newRect.y += dy;
                newRect.height -= dy;
                if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
                break;
            case ResizeMode::BOTTOM:
                newRect.height += dy;
                if (newRect.height < minHeight) newRect.height = minHeight;
                break;
            case ResizeMode::TOP_LEFT:
                newRect.x += dx; newRect.width -= dx;
                newRect.y += dy; newRect.height -= dy;
                if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
                if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
                break;
            case ResizeMode::TOP_RIGHT:
                newRect.width += dx;
                newRect.y += dy; newRect.height -= dy;
                if (newRect.width < minWidth) newRect.width = minWidth;
                if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
                break;
            case ResizeMode::BOTTOM_LEFT:
                newRect.x += dx; newRect.width -= dx;
                newRect.height += dy;
                if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
                if (newRect.height < minHeight) newRect.height = minHeight;
                break;
            case ResizeMode::BOTTOM_RIGHT:
                newRect.width += dx; newRect.height += dy;
                if (newRect.width < minWidth) newRect.width = minWidth;
                if (newRect.height < minHeight) newRect.height = minHeight;
                break;
            case ResizeMode::NONE: break;
        }
        
        DrawRubberBand(newRect);
    }
    else
    {
        ResizeMode hoverMode = GetResizeModeForPosition(event.GetPosition());
        UpdateCursorForResizeMode(hoverMode);
    }
    event.Skip();
}

ResizeMode FlatFrame::GetResizeModeForPosition(const wxPoint& clientPos)
{
    wxSize clientSize = GetClientSize();
    int x = clientPos.x;
    int y = clientPos.y;
    bool onLeft = (x >= 0 && x < m_borderThreshold);
    bool onRight = (x >= clientSize.GetWidth() - m_borderThreshold && x < clientSize.GetWidth());
    bool onTop = (y >= 0 && y < m_borderThreshold);
    bool onBottom = (y >= clientSize.GetHeight() - m_borderThreshold && y < clientSize.GetHeight());

    if (onTop && onLeft) return ResizeMode::TOP_LEFT;
    if (onBottom && onLeft) return ResizeMode::BOTTOM_LEFT;
    if (onTop && onRight) return ResizeMode::TOP_RIGHT;
    if (onBottom && onRight) return ResizeMode::BOTTOM_RIGHT;
    if (onLeft) return ResizeMode::LEFT;
    if (onRight) return ResizeMode::RIGHT;
    if (onTop) return ResizeMode::TOP;
    if (onBottom) return ResizeMode::BOTTOM;
    return ResizeMode::NONE;
}

void FlatFrame::UpdateCursorForResizeMode(ResizeMode mode)
{
    wxStockCursor cursorId = wxCURSOR_ARROW;
    switch (mode)
    {
        case ResizeMode::LEFT: case ResizeMode::RIGHT: cursorId = wxCURSOR_SIZEWE; break;
        case ResizeMode::TOP: case ResizeMode::BOTTOM: cursorId = wxCURSOR_SIZENS; break;
        case ResizeMode::TOP_LEFT: case ResizeMode::BOTTOM_RIGHT: cursorId = wxCURSOR_SIZENWSE; break;
        case ResizeMode::TOP_RIGHT: case ResizeMode::BOTTOM_LEFT: cursorId = wxCURSOR_SIZENESW; break;
        case ResizeMode::NONE: default: cursorId = wxCURSOR_ARROW; break;
    }
    SetCursor(wxCursor(cursorId));
}

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
    m_messageOutput->AppendText(message + "\n");
}

void FlatFrame::OnMenuNewProject(wxCommandEvent& event)
{
    m_messageOutput->AppendText("File Menu: New Project clicked\n");
}

void FlatFrame::OnMenuOpenProject(wxCommandEvent& event)
{
    m_messageOutput->AppendText("File Menu: Open Project clicked\n");
}

void FlatFrame::OnMenuExit(wxCommandEvent& event)
{
    Close(true);
}

void FlatFrame::OnStartupTimer(wxTimerEvent& event)
{
    DebugUIHierarchy();
    
    if (m_ribbon) {
        m_ribbon->Refresh();
        
        size_t pageCount = m_ribbon->GetPageCount();
        size_t activePage = 0;
        if (pageCount > 0) {
            FlatUIPage* page = m_ribbon->GetPage(0);
            if (page) {
                page->Show();
                page->Layout();
                page->Refresh();
                wxLogDebug("Force refreshed first page: %s", 
                           page->GetLabel());
            }
        }
    }
}

void FlatFrame::DebugUIHierarchy(wxWindow* window, int depth)
{
    if (window == nullptr) {
        window = this;
        wxLogDebug("----- UI HIERARCHY DEBUG -----");
    }
    
    wxString indent;
    for (int i = 0; i < depth; i++) {
        indent += "  ";
    }
    
    // Get basic class info
    wxString basicClassName = window->GetClassInfo()->GetClassName();
    
    // Try to get a more specific class name for our custom controls
    wxString specificClassName = basicClassName;
    
    // Check for our custom control types and get specific names
    if (FlatUIBar* bar = dynamic_cast<FlatUIBar*>(window)) {
        specificClassName = "FlatUIBar";
    }
    else if (FlatUIPage* page = dynamic_cast<FlatUIPage*>(window)) {
        specificClassName = "FlatUIPage:" + page->GetLabel();
    }
    else if (FlatUIPanel* panel = dynamic_cast<FlatUIPanel*>(window)) {
        specificClassName = "FlatUIPanel:" + panel->GetLabel();
    }
    else if (FlatUIButtonBar* buttonBar = dynamic_cast<FlatUIButtonBar*>(window)) {
        specificClassName = "FlatUIButtonBar";
        size_t buttonCount = buttonBar->GetButtonCount();
        if(buttonCount > 0) {
            specificClassName += " (" + wxString::Format("%zu", buttonCount) + " buttons)";
        }
    }
    else if (FlatUIHomeSpace* homeSpace = dynamic_cast<FlatUIHomeSpace*>(window)) {
        specificClassName = "FlatUIHomeSpace";
    }
    else if (FlatUIFunctionSpace* funcSpace = dynamic_cast<FlatUIFunctionSpace*>(window)) {
        specificClassName = "FlatUIFunctionSpace";
    }
    else if (FlatUIProfileSpace* profSpace = dynamic_cast<FlatUIProfileSpace*>(window)) {
        specificClassName = "FlatUIProfileSpace";
    }
    else if (FlatUISystemButtons* sysButtons = dynamic_cast<FlatUISystemButtons*>(window)) {
        specificClassName = "FlatUISystemButtons";
    }
    else if (FlatUIGallery* gallery = dynamic_cast<FlatUIGallery*>(window)) {
        specificClassName = "FlatUIGallery";
        size_t itemCount = gallery->GetItemCount();
        if(itemCount > 0) {
            specificClassName += " (" + wxString::Format("%zu", itemCount) + " items)";
        }
    }
    else if (FlatUICustomControl* custom = dynamic_cast<FlatUICustomControl*>(window)) {
        specificClassName = "FlatUICustomControl";
        if (!custom->GetLabel().IsEmpty()) {
            specificClassName += ":" + custom->GetLabel();
        }
    }
    else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(window)) {
        specificClassName = "wxTextCtrl";
        if (text->GetValue().Length() > 0) {
            specificClassName += " (with text)";
        }
    }
    
    // Additional info for standard controls
    wxString additionalInfo;
    
    // Get window name if available
    if (!window->GetName().IsEmpty()) {
        additionalInfo += " Name:\"" + window->GetName() + "\"";
    }
    
    additionalInfo += wxString::Format(" ID:%d", window->GetId());
    
    if (!window->IsShown()) {
        additionalInfo += " [Hidden]";
    }
    
    // Log the information
    wxLogDebug("%s%s [%p]%s - Pos:(%d,%d) Size:(%d,%d) Shown:%d", 
              indent, 
              specificClassName,
              window,
              additionalInfo,
              window->GetPosition().x, window->GetPosition().y,
              window->GetSize().GetWidth(), window->GetSize().GetHeight(),
              window->IsShown());
              
    // Process children
    for (wxWindowList::compatibility_iterator node = window->GetChildren().GetFirst();
         node;
         node = node->GetNext()) {
        wxWindow* child = node->GetData();
        DebugUIHierarchy(child, depth + 1);
    }
    
    if (depth == 0) {
        wxLogDebug("----- END UI HIERARCHY -----");
    }
}

void FlatFrame::OnSearchExecute(wxCommandEvent& event)
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

void FlatFrame::OnSearchTextEnter(wxCommandEvent& event)
{
    OnSearchExecute(event);
}

void FlatFrame::OnUserProfile(wxCommandEvent& event)
{
    m_messageOutput->AppendText("Open User Profile\n");
}

void FlatFrame::OnSettings(wxCommandEvent& event)
{
    m_messageOutput->AppendText("Open Settings\n");
}

void FlatFrame::DrawRubberBand(const wxRect& rect)
{
#ifdef __WXMSW__
    HDC hdc = ::GetDC(NULL);    
    RECT winRect;
    winRect.left = rect.GetLeft();
    winRect.top = rect.GetTop();
    winRect.right = rect.GetRight() + 1;
    winRect.bottom = rect.GetBottom() + 1;
    
    int oldROP = ::SetROP2(hdc, R2_XORPEN);
    
    HPEN hPen = ::CreatePen(PS_DOT, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
    
    HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    ::Rectangle(hdc, winRect.left, winRect.top, winRect.right, winRect.bottom);
    
    ::SelectObject(hdc, hOldPen);
    ::SelectObject(hdc, hOldBrush);
    ::DeleteObject(hPen);
    ::SetROP2(hdc, oldROP);
    ::ReleaseDC(NULL, hdc);
    
    m_rubberBandVisible = true;
    m_currentRubberBandRect = rect;
#else
    wxScreenDC dc;
    dc.SetLogicalFunction(wxINVERT);
    dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_DOT));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(rect);
    
    m_rubberBandVisible = true;
    m_currentRubberBandRect = rect;
#endif
}

void FlatFrame::EraseRubberBand()
{
    if (m_rubberBandVisible)
    {
        DrawRubberBand(m_currentRubberBandRect);
        m_rubberBandVisible = false;
    }
} 