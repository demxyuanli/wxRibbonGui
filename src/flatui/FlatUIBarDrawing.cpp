#include "flatui/FlatUIBar.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <string>
#include <memory>
#include "logger/Logger.h"
#include "config/ConstantsConfig.h"
#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key) ConstantsConfig::getInstance().getIntValue(key)

void FlatUIBar::DrawBackground(wxDC& dc)
{
    wxSize clientSize = GetClientSize();
    int padding = (CFG_INT("BarPadding"));
    int barH = GetBarHeight();
    dc.SetBrush(CFG_COLOUR("BarBackgroundColour"));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(padding, 0, clientSize.GetWidth() - 2 * padding, barH);
}

void FlatUIBar::DrawBarSeparator(wxDC& dc)
{
    wxSize clientSize = GetClientSize();
    int padding = (CFG_INT("BarPadding"));
    int barH = GetBarHeight() - m_barBottomMargin;;
    dc.SetPen(wxPen(CFG_COLOUR("BarBorderColour"), 1));
    dc.DrawLine(padding, barH, clientSize.GetWidth() - padding, barH);
}

void FlatUIBar::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    DrawBackground(dc);
    DrawBarSeparator(dc);

    LOG_DBG("FlatUIBar The Border Pos: (" + std::to_string(GetSize().GetWidth()) + "," + std::to_string(GetBarHeight()) + ")", "FlatUIBar");

    if (!m_pages.empty() && m_tabAreaRect.GetWidth() > 0) {
        wxDCClipper clip(dc, m_tabAreaRect);
        int offset = m_tabAreaRect.GetX();
        PaintTabsArea(dc, m_tabAreaRect.GetWidth(), offset);
    }
}

void FlatUIBar::DrawTabBorder(wxDC& dc, const wxRect& tabRect, bool isActive)
{
    // For SOLID style, it's handled in PaintTabs directly
    if (m_tabBorderStyle == TabBorderStyle::SOLID) {
        return;
    }

    std::unique_ptr<wxGraphicsContext> gc;

    if (wxAutoBufferedPaintDC* paintDC = dynamic_cast<wxAutoBufferedPaintDC*>(&dc)) {
        gc.reset(wxGraphicsContext::Create(*paintDC));
    }
    else if (wxClientDC* clientDC = dynamic_cast<wxClientDC*>(&dc)) {
        gc.reset(wxGraphicsContext::Create(*clientDC));
    }
    else if (wxMemoryDC* memDC = dynamic_cast<wxMemoryDC*>(&dc)) {
        gc.reset(wxGraphicsContext::Create(*memDC));
    }
    else if (wxWindowDC* winDC = dynamic_cast<wxWindowDC*>(&dc)) {
        gc.reset(wxGraphicsContext::Create(*winDC));
    }
    else if (wxAutoBufferedPaintDC* bufferedDC = dynamic_cast<wxAutoBufferedPaintDC*>(&dc)) {
        gc.reset(wxGraphicsContext::Create(*bufferedDC));
    }
    else {
        wxWindow* win = dc.GetWindow();
        if (win) {
            gc.reset(wxGraphicsContext::Create(win));
        }
    }

    if (!gc) return;

    // Use individual border colors
    wxColour topColour = isActive ? m_tabBorderTopColour : m_tabBorderColour;
    wxColour bottomColour = m_tabBorderBottomColour;
    wxColour leftColour = m_tabBorderLeftColour;
    wxColour rightColour = m_tabBorderRightColour;

    switch (m_tabBorderStyle) {
    case TabBorderStyle::DASHED:
    {
        if (m_tabBorderTop > 0) {
            wxPen dashedPen(topColour, m_tabBorderTop, wxPENSTYLE_SHORT_DASH);
            gc->SetPen(dashedPen);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetTop());
        }
        if (m_tabBorderBottom > 0) {
            wxPen dashedPen(bottomColour, m_tabBorderBottom, wxPENSTYLE_SHORT_DASH);
            gc->SetPen(dashedPen);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetBottom(), tabRect.GetRight(), tabRect.GetBottom());
        }
        if (m_tabBorderLeft > 0) {
            wxPen dashedPen(leftColour, m_tabBorderLeft, wxPENSTYLE_SHORT_DASH);
            gc->SetPen(dashedPen);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetLeft(), tabRect.GetBottom());
        }
        if (m_tabBorderRight > 0) {
            wxPen dashedPen(rightColour, m_tabBorderRight, wxPENSTYLE_SHORT_DASH);
            gc->SetPen(dashedPen);
            gc->StrokeLine(tabRect.GetRight(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetBottom());
        }
    }
    break;

    case TabBorderStyle::DOTTED:
    {
        if (m_tabBorderTop > 0) {
            wxPen dottedPen(topColour, m_tabBorderTop, wxPENSTYLE_DOT);
            gc->SetPen(dottedPen);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetTop());
        }
        if (m_tabBorderBottom > 0) {
            wxPen dottedPen(bottomColour, m_tabBorderBottom, wxPENSTYLE_DOT);
            gc->SetPen(dottedPen);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetBottom(), tabRect.GetRight(), tabRect.GetBottom());
        }
        if (m_tabBorderLeft > 0) {
            wxPen dottedPen(leftColour, m_tabBorderLeft, wxPENSTYLE_DOT);
            gc->SetPen(dottedPen);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetLeft(), tabRect.GetBottom());
        }
        if (m_tabBorderRight > 0) {
            wxPen dottedPen(rightColour, m_tabBorderRight, wxPENSTYLE_DOT);
            gc->SetPen(dottedPen);
            gc->StrokeLine(tabRect.GetRight(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetBottom());
        }
    }
    break;

    case TabBorderStyle::DOUBLE:
    {
        int gap = 2; // Gap between double lines
        if (m_tabBorderTop > 0) {
            gc->SetPen(wxPen(topColour, 1));
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetTop());
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop() + gap, tabRect.GetRight(), tabRect.GetTop() + gap);
        }
        if (m_tabBorderBottom > 0) {
            gc->SetPen(wxPen(bottomColour, 1));
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetBottom() - gap, tabRect.GetRight(), tabRect.GetBottom() - gap);
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetBottom(), tabRect.GetRight(), tabRect.GetBottom());
        }
        if (m_tabBorderLeft > 0) {
            gc->SetPen(wxPen(leftColour, 1));
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetLeft(), tabRect.GetBottom());
            gc->StrokeLine(tabRect.GetLeft() + gap, tabRect.GetTop(), tabRect.GetLeft() + gap, tabRect.GetBottom());
        }
        if (m_tabBorderRight > 0) {
            gc->SetPen(wxPen(rightColour, 1));
            gc->StrokeLine(tabRect.GetRight() - gap, tabRect.GetTop(), tabRect.GetRight() - gap, tabRect.GetBottom());
            gc->StrokeLine(tabRect.GetRight(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetBottom());
        }
    }
    break;

    case TabBorderStyle::GROOVE:
    case TabBorderStyle::RIDGE:
    {
        // For groove/ridge effect, use two colors
        wxColour lightColour = topColour.ChangeLightness(150);
        wxColour darkColour = topColour.ChangeLightness(50);

        if (m_tabBorderStyle == TabBorderStyle::RIDGE) {
            // Swap colors for ridge effect
            wxColour temp = lightColour;
            lightColour = darkColour;
            darkColour = temp;
        }

        if (m_tabBorderTop > 0 && isActive) {
            gc->SetPen(wxPen(darkColour, m_tabBorderTop / 2));
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetRight(), tabRect.GetTop());
            gc->SetPen(wxPen(lightColour, m_tabBorderTop / 2));
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop() + m_tabBorderTop / 2, tabRect.GetRight(), tabRect.GetTop() + m_tabBorderTop / 2);
        }
        if (m_tabBorderLeft > 0) {
            gc->SetPen(wxPen(darkColour, m_tabBorderLeft / 2));
            gc->StrokeLine(tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetLeft(), tabRect.GetBottom());
            gc->SetPen(wxPen(lightColour, m_tabBorderLeft / 2));
            gc->StrokeLine(tabRect.GetLeft() + m_tabBorderLeft / 2, tabRect.GetTop(), tabRect.GetLeft() + m_tabBorderLeft / 2, tabRect.GetBottom());
        }
    }
    break;

    case TabBorderStyle::ROUNDED:
    {
        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        gc->SetPen(wxPen(topColour, wxMax(wxMax(m_tabBorderTop, m_tabBorderBottom),
            wxMax(m_tabBorderLeft, m_tabBorderRight))));
        wxGraphicsPath path = gc->CreatePath();
        path.AddRoundedRectangle(tabRect.x, tabRect.y, tabRect.width, tabRect.height, m_tabCornerRadius);
        gc->StrokePath(path);
    }
    break;
    }
} 