#ifndef FLATUIGALLERY_H
#define FLATUIGALLERY_H

#include <wx/wx.h>
#include <wx/vector.h>

// Forward declaration
class FlatUIPanel;

class FlatUIGallery : public wxControl
{
public:
    FlatUIGallery(FlatUIPanel* parent);
    virtual ~FlatUIGallery();

    void AddItem(const wxBitmap& bitmap, int id);
    size_t GetItemCount() const { return m_items.size(); }

    void OnMouseDown(wxMouseEvent& evt);


    void OnPaint(wxPaintEvent& evt);

private:
    struct ItemInfo
    {
        wxBitmap bitmap;
        int id;
        wxRect rect;
    };
    wxVector<ItemInfo> m_items;
};

#endif // FLATUIGALLERY_H 