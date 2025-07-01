#include "flatui/FlatUIFrame.h"
#include <wx/dcbuffer.h> // For wxScreenDC
#include <wx/display.h>  // For wxDisplay
#include "config/ThemeManager.h"

#ifdef __WXMSW__
#include <windows.h>     // For Windows specific GDI calls for rubber band
#endif

// FlatUIFrame does not need its own event table for mouse events if it's overriding
// BorderlessFrameLogic's virtual handlers. If it had its own, distinct events,
// it would have its own wxBEGIN_EVENT_TABLE/wxEND_EVENT_TABLE block.

FlatUIFrame::FlatUIFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : BorderlessFrameLogic(parent, id, title, pos, size, style), // Call new base class constructor
      m_isPseudoMaximized(false)
      // m_dragging, m_resizing, m_resizeMode, m_rubberBandVisible, m_borderThreshold are initialized by BorderlessFrameLogic
{
    InitFrameStyle(); // Specific styling for FlatUIFrame 
}

FlatUIFrame::~FlatUIFrame()
{
    wxLogDebug("FlatUIFrame destruction started.");
    wxLogDebug("FlatUIFrame destruction completed.");
}

void FlatUIFrame::InitFrameStyle()
{
    // Set specific FlatUIFrame styles (e.g., background, etc.)
    // BorderlessFrameLogic constructor already sets DoubleBuffered.
    SetBackgroundColour(CFG_COLOUR("FrameAppWorkspaceColour"));
    // m_borderThreshold is set in BorderlessFrameLogic constructor, can be overridden here if needed for FlatUIFrame
    // e.g., this->m_borderThreshold = 10; // If FlatUIFrame needs a different threshold
}

void FlatUIFrame::OnLeftDown(wxMouseEvent& event)
{
    // Check for conditions that should prevent dragging/resizing (e.g., pseudo-maximized state)
    if (m_isPseudoMaximized) {
        event.Skip(); // Don't initiate drag/resize if maximized
        return;
    }

    // Call the base class implementation to handle the actual dragging/resizing logic
    BorderlessFrameLogic::OnLeftDown(event);
}

void FlatUIFrame::OnLeftUp(wxMouseEvent& event)
{
    // If FlatUIFrame needs specific behavior on mouse up after dragging/resizing,
    // it can add it here. Otherwise, just call base.
    BorderlessFrameLogic::OnLeftUp(event);
}

void FlatUIFrame::OnMotion(wxMouseEvent& event)
{
    // Check for conditions that should prevent cursor changes or rubber banding
    if (m_isPseudoMaximized) {
        // Still call base class to update cursor, but prevent actual resizing/dragging
        BorderlessFrameLogic::OnMotion(event);
        return;
    }

    // Call the base class implementation for cursor updates and rubber banding logic
    BorderlessFrameLogic::OnMotion(event);
}

void FlatUIFrame::PseudoMaximize()
{
    if (m_isPseudoMaximized) return;

    m_preMaximizeRect = GetScreenRect();
    int displayIndex = wxDisplay::GetFromWindow(this);
    wxDisplay display(displayIndex != wxNOT_FOUND ? displayIndex : 0);
    wxRect workArea = display.GetClientArea(); // Use client area to avoid taskbar

    SetSize(workArea);
    Move(workArea.GetPosition());
    m_isPseudoMaximized = true;
    Refresh();
    Update();
}

void FlatUIFrame::RestoreFromPseudoMaximize()
{
    if (!m_isPseudoMaximized) return;

    SetSize(m_preMaximizeRect.GetSize());
    Move(m_preMaximizeRect.GetPosition());
    m_isPseudoMaximized = false;
    Refresh();
    Update();
}

// LogUILayout method remains the same as it was, no changes needed for this refactor.
void FlatUIFrame::LogUILayout(wxWindow* window, int depth)
{
    if (!window) {
        window = this; // Default to logging the FlatUIFrame itself if no window is provided
        wxLogDebug(wxT("--- UI Layout Tree for Frame: %s ---"), GetTitle());
    }

    wxString indent(depth * 2, ' '); // Create indentation string
    wxString windowClass = window->GetClassInfo()->GetClassName();
    wxString windowName = window->GetName();
    wxString windowLabel = window->GetLabel(); // May be empty for non-control windows
    wxRect windowRect = window->GetRect();
    wxSize windowSize = window->GetSize();
    wxPoint windowPos = window->GetPosition();

    wxString logMsg = wxString::Format(wxT("%s%s (Name: %s, Label: '%s') - Pos: (%d,%d), Size: (%dx%d)"),
        indent, windowClass, windowName, windowLabel, windowPos.x, windowPos.y, windowSize.x, windowSize.y);

    wxLogDebug(logMsg);

    // Recursively log children
    wxWindowList children = window->GetChildren();
    for (wxWindowList::Node* node = children.GetFirst(); node; node = node->GetNext()) {
        wxWindow* child = (wxWindow*)node->GetData();
        LogUILayout(child, depth + 1);
    }

    if (depth == 0) {
        wxLogDebug(wxT("--- End of UI Layout Tree ---"));
    }
}

int FlatUIFrame::GetMinWidth() const
{
    return CalculateMinimumWidth();
}

int FlatUIFrame::GetMinHeight() const
{
    return CalculateMinimumHeight();
}

int FlatUIFrame::CalculateMinimumWidth() const
{
    FlatUIBar* ribbon = GetUIBar();
    if (!ribbon) return 400; // Fallback minimum width

    int minWidth = 0;

    // HomeSpace width (30px)
    if (ribbon->GetHomeSpace()) {
        minWidth += 30;
    }

    // System buttons width (estimate 3 buttons * 30px each)
    minWidth += 90;

    // One Tab width (estimate 80px)
    minWidth += 80;

    // Scroll button width (20px)
    minWidth += 20;

    // Add margins and spacing
    minWidth += 20; // Left and right margins

    return minWidth; // Approximately 240px minimum
}

int FlatUIFrame::CalculateMinimumHeight() const
{
    FlatUIBar* ribbon = GetUIBar();
    if (!ribbon) return 200; // Fallback minimum height

    // Get the actual FlatUIBar height (which is 3 times the base height)
    int actualBarHeight = ribbon->GetSize().GetHeight();
    if (actualBarHeight <= 0) {
        // If bar hasn't been sized yet, calculate expected height
        int baseHeight = FlatUIBar::GetBarHeight();
        actualBarHeight = baseHeight * 3; // 90 pixels
    }

    // Minimum height should be bar height plus some content area
    // For example: bar height + 150px for content
    return actualBarHeight + 150; // Approximately 240px minimum
}

void FlatUIFrame::SetSize(const wxRect& rect)
{
    BorderlessFrameLogic::SetSize(rect);
    HandleAdaptiveUIVisibility(rect.GetSize());
}

void FlatUIFrame::SetSize(const wxSize& size)
{
    BorderlessFrameLogic::SetSize(size);
    HandleAdaptiveUIVisibility(size);
}

void FlatUIFrame::HandleAdaptiveUIVisibility(const wxSize& newSize)
{
    FlatUIBar* ribbon = GetUIBar();
    if (!ribbon) return;

    int availableWidth = newSize.GetWidth();
    int baseWidth = CalculateMinimumWidth();

    // Calculate required width for different configurations
    int withFunctionSpace = baseWidth + 270; // FunctionSpace width
    int withProfileSpace = baseWidth + 60;   // ProfileSpace width
    int withBothSpaces = baseWidth + 270 + 60;

    // Adaptive visibility logic
    if (availableWidth >= withBothSpaces) {
        // Show both FunctionSpace and ProfileSpace
        ribbon->SetFunctionSpaceControl(GetFunctionSpaceControl(), 270);
        ribbon->SetProfileSpaceControl(GetProfileSpaceControl(), 60);
        ShowTabFunctionSpacer(true);
        ShowFunctionProfileSpacer(true);
    }
    else if (availableWidth >= withProfileSpace) {
        // Hide FunctionSpace, show ProfileSpace
        ribbon->SetFunctionSpaceControl(nullptr, 0);
        ribbon->SetProfileSpaceControl(GetProfileSpaceControl(), 60);
        ShowTabFunctionSpacer(false);
        ShowFunctionProfileSpacer(false);
    }
    else {
        // Hide both FunctionSpace and ProfileSpace
        ribbon->SetFunctionSpaceControl(nullptr, 0);
        ribbon->SetProfileSpaceControl(nullptr, 0);
        ShowTabFunctionSpacer(false);
        ShowFunctionProfileSpacer(false);
    }
}

void FlatUIFrame::ShowTabFunctionSpacer(bool show)
{
    FlatUIBar* ribbon = GetUIBar();
    if (ribbon && ribbon->GetTabFunctionSpacer()) {
        ribbon->GetTabFunctionSpacer()->Show(show);
    }
}

void FlatUIFrame::ShowFunctionProfileSpacer(bool show)
{
    FlatUIBar* ribbon = GetUIBar();
    if (ribbon && ribbon->GetFunctionProfileSpacer()) {
        ribbon->GetFunctionProfileSpacer()->Show(show);
    }
}
