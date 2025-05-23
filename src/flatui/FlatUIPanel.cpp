#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIGallery.h"
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/event.h>

enum {
    TIMER_RESIZE = wxID_HIGHEST + 1000,
    TIMER_ADD_CONTROL = wxID_HIGHEST + 1001
};

FlatUIPanel::FlatUIPanel(FlatUIPage* parent, const wxString& label, int orientation)
    : wxControl(parent, wxID_ANY), m_label(label), m_orientation(orientation),
    m_bgColour(FLATUI_PRIMARY_CONTENT_BG_COLOUR),
    m_borderStyle(PanelBorderStyle::THIN),
    m_borderColour(FLATUI_PANEL_BORDER_COLOUR),
    m_headerStyle(PanelHeaderStyle::EMBEDDED),
    m_headerColour(FLATUI_PANEL_HEADER_COLOUR),
    m_headerTextColour(FLATUI_PANEL_HEADER_TEXT_COLOUR),
    m_resizeTimer(this, TIMER_RESIZE)
{
    SetFont(GetFlatUIDefaultFont());
    SetDoubleBuffered(true);

#ifdef __WXMSW__
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        long exStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_COMPOSITED);
    }
#endif

    SetBackgroundStyle(wxBG_STYLE_PAINT);

    m_sizer = new wxBoxSizer(orientation);
    SetSizer(m_sizer);

    SetAutoLayout(true);

    FlatUIEventManager::getInstance().bindPanelEvents(this);

    Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        Freeze();
        wxEventBlocker blocker(this, wxEVT_SIZE);
        RecalculateBestSize();
        Layout();
        Refresh(false);
        Thaw();
        event.Skip();
        });

    Bind(wxEVT_TIMER, &FlatUIPanel::OnTimer, this);

    RecalculateBestSize();
}

void FlatUIPanel::OnTimer(wxTimerEvent& event)
{
    if (event.GetId() == TIMER_RESIZE || event.GetId() == TIMER_ADD_CONTROL) {
        UpdatePanelSize();
    }
}

void FlatUIPanel::UpdatePanelSize()
{
    Freeze();
    wxEventBlocker blocker(this, wxEVT_SIZE);

    bool wasHidden = !IsShown();
    if (wasHidden)
        Show();

    RecalculateBestSize();
    Layout();

    if (wasHidden)
        Hide();

    wxWindow* parent = GetParent();
    if (parent) {
        FlatUIPage* page = dynamic_cast<FlatUIPage*>(parent);
        if (page) {
            page->RecalculatePageHeight();
        }
        parent->Layout();
    }

    Refresh(false);
    Thaw();

    LOG_INF("Updated panel: " + GetLabel().ToStdString() +
        ", Size: (" + std::to_string(GetSize().GetWidth()) +
        "," + std::to_string(GetSize().GetHeight()) + ")", "FlatUIPanel");
}

void FlatUIPanel::ResizeChildControls(int width, int height)
{
    if (m_sizer) {
        m_sizer->SetDimension(0, 0, width, height);
        Layout();
    }

    LOG_DBG("ResizeChildControls called for panel: " + GetLabel().ToStdString() +
        ", Width: " + std::to_string(width) +
        ", Height: " + std::to_string(height), "FlatUIPanel");
}

void FlatUIPanel::RecalculateBestSize()
{
    static bool isRecalculating = false;
    if (isRecalculating)
        return;

    isRecalculating = true;

    Freeze();

    wxSize bestPanelSize(0, 0); // Renamed from bestSize for clarity with TARGET_PANEL_HEIGHT
    int headerOffsetWidth = 0, headerOffsetHeight = 0;

    if (m_headerStyle == PanelHeaderStyle::TOP || m_headerStyle == PanelHeaderStyle::BOTTOM_CENTERED) {
        headerOffsetHeight = FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE;
    }
    else if (m_headerStyle == PanelHeaderStyle::LEFT) {
        headerOffsetWidth = FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE;
    }

    // Calculate actual width and height needed by children
    // These calculations remain the same as original
    int childrenTotalWidth = 0;
    int childrenTotalHeight = 0;

    if (m_buttonBars.size() + m_galleries.size() > 0) {
        for (auto buttonBar : m_buttonBars) {
            if (buttonBar) {
                bool wasHidden = !buttonBar->IsShown();
                if (wasHidden) buttonBar->Show();
                wxSize barSize = buttonBar->GetBestSize();
                if (m_orientation == wxHORIZONTAL) {
                    childrenTotalWidth += barSize.GetWidth();
                    childrenTotalHeight = wxMax(childrenTotalHeight, barSize.GetHeight());
                } else {
                    childrenTotalWidth = wxMax(childrenTotalWidth, barSize.GetWidth());
                    childrenTotalHeight += barSize.GetHeight();
                }
                if (wasHidden) buttonBar->Hide();
            }
        }

        for (auto gallery : m_galleries) {
            if (gallery) {
                bool wasHidden = !gallery->IsShown();
                if (wasHidden) gallery->Show();
                wxSize gallerySize = gallery->GetBestSize();
                if (m_orientation == wxHORIZONTAL) {
                    childrenTotalWidth += gallerySize.GetWidth();
                    childrenTotalHeight = wxMax(childrenTotalHeight, gallerySize.GetHeight());
                } else {
                    childrenTotalWidth = wxMax(childrenTotalWidth, gallerySize.GetWidth());
                    childrenTotalHeight += gallerySize.GetHeight();
                }
                if (wasHidden) gallery->Hide();
            }
        }
    } else {
        // Default minimum size if no children, to ensure panel is visible
        // Values based on original implicit empty panel size, adjust if needed
        childrenTotalWidth = GetMinSize().GetWidth() > 0 ? GetMinSize().GetWidth() - FLATUI_PANEL_INTERNAL_PADDING_TOTAL - headerOffsetWidth : 20; 
        // childrenTotalHeight calculation for empty panel needs to result in TARGET_PANEL_HEIGHT
        // This part is tricky if we want an empty panel to also be TARGET_PANEL_HEIGHT.
        // Let's assume childrenTotalHeight is 0 if no children, 
        // and TARGET_PANEL_HEIGHT will handle the overall panel height.
        childrenTotalHeight = 0; 
    }
    
    // Panel's best width is based on children content + padding + header
    bestPanelSize.SetWidth(childrenTotalWidth + FLATUI_PANEL_INTERNAL_PADDING_TOTAL + headerOffsetWidth);
    // Panel's best height is now the target height
    bestPanelSize.SetHeight(FLATUI_PANEL_TARGET_HEIGHT);

    SetMinSize(bestPanelSize);
    SetSize(bestPanelSize); // Keep this to enforce size, as in original

    if (m_sizer) {
        int sizerAreaX = headerOffsetWidth;
        int sizerAreaY = 0;
        
        // Width for sizer is panel's content width (children + their collective padding handled by sizer items)
        int sizerAreaWidth = bestPanelSize.GetWidth() - headerOffsetWidth;
        // Height for sizer is what's left of TARGET_PANEL_HEIGHT after header
        int sizerAreaHeight = FLATUI_PANEL_TARGET_HEIGHT - headerOffsetHeight;

        if (sizerAreaWidth < 0) sizerAreaWidth = 0;
        if (sizerAreaHeight < 0) sizerAreaHeight = 0;
        
        m_sizer->SetDimension(sizerAreaX, sizerAreaY, sizerAreaWidth, sizerAreaHeight);
    }

    LOG_INF("RecalculateBestSize for panel: " + GetLabel().ToStdString() +
        " Best Size: (" + std::to_string(bestPanelSize.GetWidth()) + ", " +
        std::to_string(bestPanelSize.GetHeight()) + ")", "FlatUIPanel");

    Thaw();
    isRecalculating = false;
}

FlatUIPanel::~FlatUIPanel()
{
    for (auto buttonBar : m_buttonBars)
        delete buttonBar;
    for (auto gallery : m_galleries)
        delete gallery;
}

void FlatUIPanel::SetPanelBackgroundColour(const wxColour& colour)
{
    m_bgColour = colour;
    Refresh();
}

void FlatUIPanel::SetBorderStyle(PanelBorderStyle style)
{
    m_borderStyle = style;
    Refresh();
}

void FlatUIPanel::SetBorderColour(const wxColour& colour)
{
    m_borderColour = colour;
    Refresh();
}

void FlatUIPanel::SetHeaderStyle(PanelHeaderStyle style)
{
    m_headerStyle = style;
    UpdatePanelSize();
}

void FlatUIPanel::SetHeaderColour(const wxColour& colour)
{
    m_headerColour = colour;
    Refresh();
}

void FlatUIPanel::SetHeaderTextColour(const wxColour& colour)
{
    m_headerTextColour = colour;
    Refresh();
}

void FlatUIPanel::SetLabel(const wxString& label)
{
    m_label = label;
    Refresh();
}

void FlatUIPanel::AddButtonBar(FlatUIButtonBar* buttonBar, int proportion, int flag, int border)
{
    Freeze();
    m_buttonBars.push_back(buttonBar);
    LOG_INF("Added ButtonBar to panel: " + GetLabel().ToStdString(), "FlatUIPanel");
    FlatUIEventManager::getInstance().bindButtonBarEvents(buttonBar);

    flag = wxALL;
    border = 4; // 4-pixel margin
    proportion = 0;

    if (m_sizer) {
        m_sizer->Add(buttonBar, proportion, flag, border);
        UpdatePanelSize();
    }

    buttonBar->Show();
    Thaw();
}

void FlatUIPanel::AddGallery(FlatUIGallery* gallery, int proportion, int flag, int border)
{
    Freeze();
    m_galleries.push_back(gallery);
    LOG_INF("Added Gallery to panel: " + GetLabel().ToStdString() +
        ", Size: " + std::to_string(gallery->GetSize().GetWidth()) +
        "x" + std::to_string(gallery->GetSize().GetHeight()), "FlatUIPanel");
    FlatUIEventManager::getInstance().bindGalleryEvents(gallery);

    gallery->SetMinSize(gallery->GetBestSize());
    flag = wxALL;
    border = 4; // 4-pixel margin
    proportion = 0;

    if (m_sizer) {
        m_sizer->Add(gallery, proportion, flag, border);
        UpdatePanelSize();
    }

    gallery->Show();
    Thaw();
}

void FlatUIPanel::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();

    dc.SetBackground(m_bgColour);
    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (!gc) {
        LOG_ERR("Failed to create wxGraphicsContext in FlatUIPanel::OnPaint", "FlatUIPanel");
        evt.Skip();
        return;
    }

    // Draw borders based on m_borderStyle
    if (m_borderStyle != PanelBorderStyle::NONE) {
        gc->SetPen(wxPen(m_borderColour, 1)); // Use m_borderColour
        switch (m_borderStyle) {
            case PanelBorderStyle::THIN:
            case PanelBorderStyle::MEDIUM: // For now, THIN and MEDIUM are treated the same
            case PanelBorderStyle::THICK:  // THICK could be thicker if needed
            { // Optional: braces for consistency
                // Draw a simple rectangle border
                gc->StrokeLine(0.5, 0.5, size.GetWidth() - 0.5, 0.5); // Top
                gc->StrokeLine(size.GetWidth() - 0.5, 0.5, size.GetWidth() - 0.5, size.GetHeight() - 0.5); // Right
                gc->StrokeLine(size.GetWidth() - 0.5, size.GetHeight() - 0.5, 0.5, size.GetHeight() - 0.5); // Bottom
                gc->StrokeLine(0.5, size.GetHeight() - 0.5, 0.5, 0.5); // Left
                break;
            }
            case PanelBorderStyle::ROUNDED:
            { // Added braces to create a scope for path
                // Rounded rectangle requires a path
                gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
                wxGraphicsPath path = gc->CreatePath(); // Create a path object
                path.AddRoundedRectangle(0.5, 0.5, size.GetWidth() - 1, size.GetHeight() - 1, 5.0); // Add rounded rectangle to the path, radius 5.0
                gc->StrokePath(path); // Stroke the created path
                break;
            }
            case PanelBorderStyle::NONE: // Explicitly do nothing
            { // Optional: braces for consistency
                break;
            }
        }
    }

    if (m_headerStyle != PanelHeaderStyle::NONE && !m_label.IsEmpty()) {
        gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), m_headerTextColour);
        wxDouble textWidth, textHeight;
        gc->GetTextExtent(m_label, &textWidth, &textHeight);

        switch (m_headerStyle) {
        case PanelHeaderStyle::TOP:
            gc->SetBrush(wxBrush(m_headerColour));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRectangle(0, 0, size.GetWidth(), FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE);
            gc->DrawText(m_label, 0, (FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE - textHeight) / 2);
            break;
        case PanelHeaderStyle::LEFT:
            gc->SetBrush(wxBrush(m_headerColour));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRectangle(0, 0, FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE, size.GetHeight());
            gc->PushState();
            gc->Translate(FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE / 2, size.GetHeight() / 2);
            gc->Rotate(-M_PI / 2);
            gc->DrawText(m_label, -textWidth / 2, -textHeight / 2);
            gc->PopState();
            break;
        case PanelHeaderStyle::EMBEDDED:
            gc->DrawText(m_label, 0, 0);
            break;
        case PanelHeaderStyle::BOTTOM_CENTERED:
            gc->SetBrush(wxBrush(m_headerColour));
            gc->SetPen(*wxTRANSPARENT_PEN);
            // Draw header bar at the bottom
            gc->DrawRectangle(0, size.GetHeight() - FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE, size.GetWidth(), FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE);
            
            // Draw text centered in the bottom header bar
            gc->DrawText(m_label, 
                         (size.GetWidth() - textWidth) / 2, 
                         size.GetHeight() - FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE + (FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE - textHeight) / 2);
            break;
        }
    }

    delete gc;
    evt.Skip();
}