#include "flatui/FlatUIFloatPanel.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIPinButton.h"
#include "logger/Logger.h"
#include "config/ConstantsConfig.h"

// Define the custom event
wxDEFINE_EVENT(wxEVT_FLOAT_PANEL_DISMISSED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(FlatUIFloatPanel, wxFrame)
EVT_PAINT(FlatUIFloatPanel::OnPaint)
EVT_SIZE(FlatUIFloatPanel::OnSize)
EVT_ENTER_WINDOW(FlatUIFloatPanel::OnMouseEnter)
EVT_LEAVE_WINDOW(FlatUIFloatPanel::OnMouseLeave)
EVT_ACTIVATE(FlatUIFloatPanel::OnActivate)
EVT_KILL_FOCUS(FlatUIFloatPanel::OnKillFocus)
EVT_TIMER(wxID_ANY, FlatUIFloatPanel::OnAutoHideTimer)
EVT_COMMAND(wxID_ANY, wxEVT_PIN_BUTTON_CLICKED, FlatUIFloatPanel::OnPinButtonClicked)
wxEND_EVENT_TABLE()

FlatUIFloatPanel::FlatUIFloatPanel(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT | wxBORDER_NONE),
    m_currentPage(nullptr),
    m_contentPanel(nullptr),
    m_parentWindow(parent),
    m_pinButton(nullptr),
    m_autoHideTimer(this),
    m_borderWidth(0),
    m_shadowOffset(1)
{
    SetName("FlatUIFloatPanel");

    // Create content panel
    m_contentPanel = new wxPanel(this, wxID_ANY);
    m_contentPanel->SetName("FloatPanelContent");

    // Create sizer for layout
    m_sizer = new wxBoxSizer(wxVERTICAL);
    m_contentPanel->SetSizer(m_sizer);

    // Create pin button for the float panel - make it a child of the content panel to avoid overlap
    m_pinButton = new FlatUIPinButton(m_contentPanel, wxID_ANY);
    m_pinButton->SetName("FloatPanelPinButton");
    m_pinButton->Show(false); // Initially hidden, will be shown when float panel is displayed
    // Ensure pin button has proper layering and visibility settings
    m_pinButton->SetCanFocus(false); // Prevent focus issues
    // Ensure pin button is always on top within its parent
    m_pinButton->SetWindowStyleFlag(m_pinButton->GetWindowStyleFlag() | wxSTAY_ON_TOP);
    LOG_INF("Created pin button for float panel as child of content panel, initially hidden", "FlatUIFloatPanel");

    // Setup appearance and event handlers
    SetupAppearance();
    SetupEventHandlers();

    // Main frame sizer
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(m_contentPanel, 1, wxEXPAND | wxALL, m_borderWidth);
    SetSizer(frameSizer);

    LOG_INF("Created FlatUIFloatPanel", "FlatUIFloatPanel");
}

FlatUIFloatPanel::~FlatUIFloatPanel()
{
    StopAutoHideTimer();

    if (m_currentPage) {
        m_sizer->Detach(m_currentPage);
        m_currentPage = nullptr;
    }

    if (m_pinButton) {
        m_pinButton->Destroy();
        m_pinButton = nullptr;
    }

    LOG_INF("Destroyed FlatUIFloatPanel", "FlatUIFloatPanel");
}

void FlatUIFloatPanel::SetupAppearance()
{
    // Set colors based on configuration or defaults
    m_borderColour = wxColour(*wxRED);
    m_backgroundColour = wxColour(*wxWHITE);
    m_shadowColour = wxColour(255, 255, 255, 100);

    // Try to get colors from parent or config
    if (m_parentWindow) {
        m_backgroundColour = m_parentWindow->GetBackgroundColour();
    }

    SetBackgroundColour(m_backgroundColour);
    m_contentPanel->SetBackgroundColour(m_backgroundColour);
}

void FlatUIFloatPanel::SetupEventHandlers()
{
    // Setup global mouse tracking for auto-hide
    if (wxWindow* topLevel = wxGetTopLevelParent(m_parentWindow)) {
        topLevel->Bind(wxEVT_MOTION, &FlatUIFloatPanel::OnGlobalMouseMove, this);
    }
}

void FlatUIFloatPanel::SetPageContent(FlatUIPage* page)
{
    if (m_currentPage == page) {
        return; // Same page, no change needed
    }

    // Remove current page if any
    if (m_currentPage) {
        m_sizer->Detach(m_currentPage);
        m_currentPage->Reparent(m_parentWindow);
        m_currentPage->Hide();
    }

    // Set new page
    m_currentPage = page;
    if (m_currentPage) {
        m_currentPage->Reparent(m_contentPanel);
        m_sizer->Add(m_currentPage, 1, wxEXPAND | wxALL, 0);
        m_currentPage->Show();

        // Update layout
        m_contentPanel->Layout();
        Layout();

        LOG_INF("Set page content: " + m_currentPage->GetLabel().ToStdString(), "FlatUIFloatPanel");

        m_pinButton->Show(true);
        m_pinButton->Raise();
    }
}

void FlatUIFloatPanel::ShowAt(const wxPoint& position, const wxSize& size)
{
    if (!m_currentPage) {
        LOG_INF("ShowAt: No current page, cannot show float panel", "FlatUIFloatPanel");
        return;
    }

    wxSize panelSize = size;
    if (panelSize == wxDefaultSize) {
        // Calculate size based on page content if no explicit size is given
        wxSize pageSize = m_currentPage->GetBestSize();
        // Add border and shadow space
        pageSize.SetWidth(pageSize.GetWidth() + m_borderWidth * 2 + m_shadowOffset);
        pageSize.SetHeight(pageSize.GetHeight() + m_borderWidth * 2 + m_shadowOffset);
        // Ensure minimum size
        pageSize.SetWidth(wxMax(pageSize.GetWidth(), 300));
        pageSize.SetHeight(wxMax(pageSize.GetHeight(), 100));
        panelSize = pageSize;
    }

    // Check screen boundaries and adjust position if needed
    wxSize screenSize = wxGetDisplaySize();
    wxPoint adjustedPos = position;

    if (adjustedPos.x + panelSize.GetWidth() > screenSize.GetWidth()) {
        adjustedPos.x = screenSize.GetWidth() - panelSize.GetWidth() - 10;
    }
    if (adjustedPos.y + panelSize.GetHeight() > screenSize.GetHeight()) {
        adjustedPos.y = screenSize.GetHeight() - panelSize.GetHeight() - 10;
    }

    // Ensure position is not negative
    adjustedPos.x = wxMax(adjustedPos.x, 0);
    adjustedPos.y = wxMax(adjustedPos.y, 0);

    // Set size and position for the panel
    SetSize(panelSize);
    SetPosition(adjustedPos);

    // Show the panel itself before its children
    Show(true);

    // Update page layout first
    if (m_currentPage) {
        m_currentPage->UpdateLayout();
    }

    // Force a complete layout and refresh cycle first
    Layout();
    Refresh();
    Update();

    // Position and show pin button AFTER all layout operations are complete
    if (m_pinButton) {
        wxSize pinSize = m_pinButton->GetBestSize();
        int margin = 4; // Small margin from edges

        // Since pin button is now a child of content panel, use content panel size
        wxSize contentSize = m_contentPanel->GetSize();
        int pinX = wxMax(0, contentSize.GetWidth() - pinSize.GetWidth() - margin);
        int pinY = wxMax(0, contentSize.GetHeight() - pinSize.GetHeight() - margin);

        m_pinButton->SetPosition(wxPoint(pinX, pinY));
        m_pinButton->SetSize(pinSize);

        // Force pin button to be always visible and on top - AFTER layout
        m_pinButton->Show(true);
        m_pinButton->Enable(true);
        m_pinButton->Raise();

        // Force refresh to ensure pin button is rendered
        m_pinButton->Update();
        m_pinButton->Refresh();

        LOG_INF("Pin button positioned and shown at (" +
            std::to_string(pinX) + ", " + std::to_string(pinY) + ") relative to content panel AFTER layout", "FlatUIFloatPanel");
    }

    // Start auto-hide monitoring
    StartAutoHideTimer();
}

void FlatUIFloatPanel::HidePanel()
{
    if (IsShown()) {
        LOG_INF("HidePanel: Starting to hide float panel", "FlatUIFloatPanel");

        StopAutoHideTimer();

        // Hide pin button first - it should not be visible when panel is hidden
        if (m_pinButton) {
            m_pinButton->Hide();
            LOG_INF("HidePanel: Hidden pin button", "FlatUIFloatPanel");
        }

        // Hide the panel
        Hide();

        // Return page to original parent
        if (m_currentPage) {
            m_sizer->Detach(m_currentPage);
            m_currentPage->Reparent(m_parentWindow);
            m_currentPage->Hide();
            m_currentPage = nullptr;
            LOG_INF("HidePanel: Returned page to original parent", "FlatUIFloatPanel");
        }

        // Notify parent that float panel was hidden
        if (m_parentWindow) {
            wxCommandEvent event(wxEVT_FLOAT_PANEL_DISMISSED, GetId());
            event.SetEventObject(this);
            wxPostEvent(m_parentWindow, event);
        }

        LOG_INF("Hidden float panel", "FlatUIFloatPanel");
    }
}

void FlatUIFloatPanel::ForceHide()
{
    HidePanel();
}

bool FlatUIFloatPanel::ShouldAutoHide(const wxPoint& globalMousePos) const
{
    if (!IsShown()) {
        return false;
    }

    // Get panel bounds in screen coordinates
    wxRect panelRect = GetScreenRect();

    // Add a small margin around the panel to prevent immediate hiding
    panelRect.Inflate(5, 5);

    // Check if mouse is outside the panel area
    bool mouseOutside = !panelRect.Contains(globalMousePos);

    // Also check if mouse is over the parent bar area
    bool mouseOverParent = false;
    if (m_parentWindow) {
        wxRect parentRect = m_parentWindow->GetScreenRect();
        mouseOverParent = parentRect.Contains(globalMousePos);
    }

    // Should auto-hide if mouse is outside panel and not over parent
    return mouseOutside && !mouseOverParent;
}

void FlatUIFloatPanel::StartAutoHideTimer()
{
    if (!m_autoHideTimer.IsRunning()) {
        m_autoHideTimer.Start(AUTO_HIDE_DELAY_MS);
    }
}

void FlatUIFloatPanel::StopAutoHideTimer()
{
    if (m_autoHideTimer.IsRunning()) {
        m_autoHideTimer.Stop();
    }
}

void FlatUIFloatPanel::CheckAutoHide()
{

    wxPoint mousePos = wxGetMousePosition();
    if (ShouldAutoHide(mousePos)) {
        HidePanel();
    }
}

void FlatUIFloatPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    // Draw shadow first (behind the panel)
    DrawShadow(dc);

    // Draw custom border
    DrawCustomBorder(dc);

    event.Skip();
}

void FlatUIFloatPanel::DrawShadow(wxDC& dc)
{
    if (m_shadowOffset <= 0) return;

    wxSize size = GetSize();
    wxRect shadowRect(m_shadowOffset, m_shadowOffset,
        size.GetWidth() - m_shadowOffset,
        size.GetHeight() - m_shadowOffset);

    // Create a simple shadow effect
    dc.SetBrush(wxBrush(m_shadowColour));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(shadowRect);
}

void FlatUIFloatPanel::DrawCustomBorder(wxDC& dc)
{
    if (m_borderWidth <= 0) return;

    wxSize size = GetSize();
    wxRect borderRect(0, 0, size.GetWidth() - m_shadowOffset, size.GetHeight() - m_shadowOffset);

    // Draw border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(m_borderColour, m_borderWidth));
    dc.DrawRectangle(borderRect);
}

void FlatUIFloatPanel::OnSize(wxSizeEvent& event)
{
    Layout();

    // Reposition and ensure pin button is visible when panel size changes
    if (m_pinButton) {
        wxSize contentSize = m_contentPanel->GetSize();
        wxSize pinSize = m_pinButton->GetBestSize();
        int margin = 4;

        // Ensure pin button is within the content panel bounds
        int maxPinX = contentSize.GetWidth() - pinSize.GetWidth() - margin;
        int maxPinY = contentSize.GetHeight() - pinSize.GetHeight() - margin;

        // Make sure position is not negative
        int pinX = wxMax(0, maxPinX);
        int pinY = wxMax(0, maxPinY);

        m_pinButton->SetPosition(wxPoint(pinX, pinY));

        // Ensure pin button is always visible after repositioning
        m_pinButton->Show(true);
        m_pinButton->Raise();
        m_pinButton->Update();

        LOG_INF("OnSize: Pin button repositioned to (" +
            std::to_string(pinX) + ", " + std::to_string(pinY) + ") relative to content panel", "FlatUIFloatPanel");
    }

    Refresh(); // Redraw custom border and shadow
    event.Skip();
}

void FlatUIFloatPanel::OnMouseEnter(wxMouseEvent& event)
{
    StopAutoHideTimer();
    event.Skip();
}

void FlatUIFloatPanel::OnMouseLeave(wxMouseEvent& event)
{
    StartAutoHideTimer();
    event.Skip();
}

void FlatUIFloatPanel::OnActivate(wxActivateEvent& event)
{
    if (!event.GetActive()) {
        // Panel is being deactivated, start auto-hide timer
        StartAutoHideTimer();
    }
    else {
        // Panel is being activated, stop auto-hide timer
        StopAutoHideTimer();
    }
    event.Skip();
}

void FlatUIFloatPanel::OnKillFocus(wxFocusEvent& event)
{
    StartAutoHideTimer();
    event.Skip();
}

void FlatUIFloatPanel::OnAutoHideTimer(wxTimerEvent& event)
{
    CheckAutoHide();
}

void FlatUIFloatPanel::OnGlobalMouseMove(wxMouseEvent& event)
{
    if (IsShown()) {
        wxPoint globalPos = wxGetMousePosition();
        if (ShouldAutoHide(globalPos)) {
            StartAutoHideTimer();
        }
        else {
            StopAutoHideTimer();
        }
    }
    event.Skip();
}

void FlatUIFloatPanel::OnPinButtonClicked(wxCommandEvent& event)
{
    // Forward the pin button click to the parent window (FlatUIBar)
    if (m_parentWindow) {
        wxCommandEvent pinEvent(wxEVT_PIN_BUTTON_CLICKED, GetId());
        pinEvent.SetEventObject(this);
        wxPostEvent(m_parentWindow, pinEvent);

        LOG_INF("Pin button clicked in float panel, forwarding to parent", "FlatUIFloatPanel");
    }

    event.Skip();
}