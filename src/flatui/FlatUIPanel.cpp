#include "flatui/FlatUIPanel.h"
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
    m_bgColour(wxColour(240, 240, 240)),
    m_borderStyle(PanelBorderStyle::THIN),
    m_borderColour(*wxBLACK),
    m_headerStyle(PanelHeaderStyle::EMBEDDED),
    m_headerColour(wxColour(230, 230, 230)),
    m_headerTextColour(*wxBLACK),
    m_resizeTimer(this, TIMER_RESIZE)
{
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

    // Temporarily show hidden controls to compute sizes
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

    wxSize bestSize(0, 0);
    int headerOffsetWidth = 0, headerOffsetHeight = 0;

    if (m_headerStyle == PanelHeaderStyle::TOP) {
        headerOffsetHeight = 20;
    }
    else if (m_headerStyle == PanelHeaderStyle::LEFT) {
        headerOffsetWidth = 20;
    }

    bestSize.SetWidth(10 + headerOffsetWidth);
    bestSize.SetHeight(10 + headerOffsetHeight);

    int totalControls = m_buttonBars.size() + m_galleries.size();
    if (totalControls > 0) {
        int controlSpacing = 5;
        int totalWidth = 0;
        int totalHeight = 0;

        for (auto buttonBar : m_buttonBars) {
            if (buttonBar) {
                // Ensure button bar is laid out
                bool wasHidden = !buttonBar->IsShown();
                if (wasHidden)
                    buttonBar->Show();

                wxSize barSize = buttonBar->GetBestSize();
                if (m_orientation == wxHORIZONTAL) {
                    totalWidth += barSize.GetWidth() + controlSpacing;
                    totalHeight = wxMax(totalHeight, barSize.GetHeight());
                }
                else {
                    totalWidth = wxMax(totalWidth, barSize.GetWidth());
                    totalHeight += barSize.GetHeight() + controlSpacing;
                }

                if (wasHidden)
                    buttonBar->Hide();
            }
        }

        for (auto gallery : m_galleries) {
            if (gallery) {
                // Ensure gallery is laid out
                bool wasHidden = !gallery->IsShown();
                if (wasHidden)
                    gallery->Show();

                wxSize gallerySize = gallery->GetBestSize();
                if (m_orientation == wxHORIZONTAL) {
                    totalWidth += gallerySize.GetWidth() + controlSpacing;
                    totalHeight = wxMax(totalHeight, gallerySize.GetHeight());
                }
                else {
                    totalWidth = wxMax(totalWidth, gallerySize.GetWidth());
                    totalHeight += gallerySize.GetHeight() + controlSpacing;
                }

                if (wasHidden)
                    gallery->Hide();
            }
        }

        if (m_orientation == wxHORIZONTAL && totalWidth > 0) {
            totalWidth -= controlSpacing;
        }
        else if (m_orientation == wxVERTICAL && totalHeight > 0) {
            totalHeight -= controlSpacing;
        }

        bestSize.SetWidth(totalWidth + 10 + headerOffsetWidth);
        bestSize.SetHeight(totalHeight + 10 + headerOffsetHeight);
    }

    SetMinSize(bestSize);
    SetSize(bestSize);

    if (m_sizer) {
        m_sizer->SetDimension(headerOffsetWidth, headerOffsetHeight,
            bestSize.GetWidth() - headerOffsetWidth,
            bestSize.GetHeight() - headerOffsetHeight);
    }

    LOG_INF("RecalculateBestSize for panel: " + GetLabel().ToStdString() +
        " Best Size: (" + std::to_string(bestSize.GetWidth()) + ", " +
        std::to_string(bestSize.GetHeight()) + ")", "FlatUIPanel");

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

    if (flag == 0) {
        flag = wxALL;
    }
    if (border == 0) {
        border = 5;
    }
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
    if (border == 0) {
        border = 5;
    }
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
        DrawWithDC(dc, size);
        return;
    }

    gc->SetBrush(wxBrush(m_bgColour));

    switch (m_borderStyle) {
    case PanelBorderStyle::NONE:
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
        break;
    case PanelBorderStyle::THIN:
        gc->SetPen(wxPen(m_borderColour, 1));
        gc->DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
        break;
    case PanelBorderStyle::MEDIUM:
        gc->SetPen(wxPen(m_borderColour, 2));
        gc->DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
        break;
    case PanelBorderStyle::THICK:
        gc->SetPen(wxPen(m_borderColour, 3));
        gc->DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
        break;
    case PanelBorderStyle::ROUNDED:
        gc->SetPen(wxPen(m_borderColour, 1));
        gc->DrawRoundedRectangle(0, 0, size.GetWidth(), size.GetHeight(), 8);
        break;
    }

    if (m_headerStyle != PanelHeaderStyle::NONE && !m_label.IsEmpty()) {
        gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), m_headerTextColour);
        wxDouble textWidth, textHeight;
        gc->GetTextExtent(m_label, &textWidth, &textHeight);

        switch (m_headerStyle) {
        case PanelHeaderStyle::TOP:
            gc->SetBrush(wxBrush(m_headerColour));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRectangle(0, 0, size.GetWidth(), 20);
            gc->SetPen(wxPen(m_headerTextColour));
            gc->DrawText(m_label, 5, (20 - textHeight) / 2);
            break;
        case PanelHeaderStyle::LEFT:
            gc->SetBrush(wxBrush(m_headerColour));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRectangle(0, 0, 20, size.GetHeight());
            gc->PushState();
            gc->Translate(10, size.GetHeight() / 2);
            gc->Rotate(-M_PI / 2);
            gc->DrawText(m_label, -textWidth / 2, -textHeight / 2);
            gc->PopState();
            break;
        case PanelHeaderStyle::EMBEDDED:
            gc->DrawText(m_label, 5, 5);
            break;
        }
    }

    delete gc;
}

void FlatUIPanel::DrawWithDC(wxDC& dc, const wxSize& size)
{
    int penWidth = 1;
    switch (m_borderStyle) {
    case PanelBorderStyle::NONE:
        dc.SetPen(*wxTRANSPARENT_PEN);
        break;
    case PanelBorderStyle::THIN:
        penWidth = 1;
        break;
    case PanelBorderStyle::MEDIUM:
        penWidth = 2;
        break;
    case PanelBorderStyle::THICK:
        penWidth = 3;
        break;
    case PanelBorderStyle::ROUNDED:
        penWidth = 1;
        break;
    }

    if (m_borderStyle != PanelBorderStyle::NONE) {
        dc.SetPen(wxPen(m_borderColour, penWidth));
    }

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    if (m_headerStyle != PanelHeaderStyle::NONE && !m_label.IsEmpty()) {
        dc.SetTextForeground(m_headerTextColour);
        dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

        switch (m_headerStyle) {
        case PanelHeaderStyle::TOP:
            dc.SetBrush(wxBrush(m_headerColour));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(0, 0, size.GetWidth(), 20);
            dc.DrawText(m_label, 5, 2);
            break;
        case PanelHeaderStyle::LEFT:
            dc.SetBrush(wxBrush(m_headerColour));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(0, 0, 20, size.GetHeight());
            if (!m_label.IsEmpty()) {
                dc.DrawText(m_label.Left(1), 5, 5);
            }
            break;
        case PanelHeaderStyle::EMBEDDED:
            dc.DrawText(m_label, 5, 2);
            break;
        }
    }
}