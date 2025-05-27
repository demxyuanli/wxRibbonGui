#ifndef FLATUI_H
#define FLATUI_H

#include <wx/wx.h>
#include <wx/artprov.h>

class FlatUIBar;
class FlatUIPage;
class FlatUIPanel;
class FlatUIButtonBar;
class FlatUIGallery;

class FlatUIBar : public wxControl
{
public:
    FlatUIBar(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~FlatUIBar();

    void AddPage(FlatUIPage* page);
    void OnMouseDown(wxMouseEvent& evt);
    void SetActivePage(size_t index);
    size_t GetPageCount() const;
    FlatUIPage* GetPage(size_t index) const;

protected:
    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);

private:
    wxVector<FlatUIPage*> m_pages;
    size_t m_activePage;
};

class FlatUIPage : public wxControl
{
public:
    FlatUIPage(FlatUIBar* parent, const wxString& label);
    virtual ~FlatUIPage();

    void AddPanel(FlatUIPanel* panel);

    wxString GetLabel() const { return m_label; }

private:
    wxString m_label;
    wxVector<FlatUIPanel*> m_panels;
};

class FlatUIPanel : public wxControl
{
public:
    FlatUIPanel(FlatUIPage* parent, const wxString& label, int orientation = wxVERTICAL);
    virtual ~FlatUIPanel();

    void AddButtonBar(FlatUIButtonBar* buttonBar, int proportion = 0, int flag = wxEXPAND | wxALL, int border = 5);
    void AddGallery(FlatUIGallery* gallery, int proportion = 0, int flag = wxEXPAND | wxALL, int border = 5);
    wxString GetLabel() const { return m_label; }

private:
    void OnPaint(wxPaintEvent& evt);
    wxString m_label;
    wxVector<FlatUIButtonBar*> m_buttonBars;
    wxVector<FlatUIGallery*> m_galleries;
    wxBoxSizer* m_sizer = nullptr;
    int m_orientation;
};

class FlatUIButtonBar : public wxControl
{
public:
    FlatUIButtonBar(FlatUIPanel* parent);
    virtual ~FlatUIButtonBar();

    void AddButton(int id, const wxString& label, const wxBitmap& bitmap, wxMenu* menu = nullptr);

protected:
    void OnPaint(wxPaintEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);

private:
    struct ButtonInfo
    {
        int id;
        wxString label;
        wxBitmap bitmap;
        wxRect rect;
        wxMenu* menu = nullptr;     // Menu associated with the button
        bool isDropDown = false; // True if this button should show a dropdown menu
    };
    wxVector<ButtonInfo> m_buttons;
};

class FlatUIGallery : public wxControl
{
public:
    FlatUIGallery(FlatUIPanel* parent);
    virtual ~FlatUIGallery();

    void AddItem(const wxBitmap& bitmap, int id);

protected:
    void OnPaint(wxPaintEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);

private:
    struct ItemInfo
    {
        wxBitmap bitmap;
        int id;
        wxRect rect;
    };
    wxVector<ItemInfo> m_items;
};

#endif // FLATUI_H 