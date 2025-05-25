#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUIFrame.h"
#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUIGallery.h"

FlatUIEventManager& FlatUIEventManager::getInstance()
{
    static FlatUIEventManager instance;
    return instance;
}

void FlatUIEventManager::bindFrameEvents(FlatUIFrame* frame)
{
    if (!frame) return;
    
    frame->Bind(wxEVT_LEFT_DOWN, &FlatUIFrame::OnLeftDown, frame);
    frame->Bind(wxEVT_LEFT_UP, &FlatUIFrame::OnLeftUp, frame);
    frame->Bind(wxEVT_MOTION, &FlatUIFrame::OnMotion, frame);
    
}

void FlatUIEventManager::bindBarEvents(FlatUIBar* bar)
{
    if (!bar) return;
    
    bar->Bind(wxEVT_PAINT, &FlatUIBar::OnPaint, bar);
    bar->Bind(wxEVT_SIZE, &FlatUIBar::OnSize, bar);
    bar->Bind(wxEVT_LEFT_DOWN, &FlatUIBar::OnMouseDown, bar);
}

void FlatUIEventManager::bindPageEvents(FlatUIPage* page)
{
    if (!page) return;
    
    page->Bind(wxEVT_SIZE, [page](wxSizeEvent& event) {
        page->Layout();
        event.Skip();
    });
}

void FlatUIEventManager::bindPanelEvents(FlatUIPanel* panel)
{
    if (!panel) return;
    
    panel->Bind(wxEVT_PAINT, &FlatUIPanel::OnPaint, panel);
}

void FlatUIEventManager::bindButtonBarEvents(FlatUIButtonBar* buttonBar)
{
    if (!buttonBar) return;
    
    buttonBar->Bind(wxEVT_PAINT, &FlatUIButtonBar::OnPaint, buttonBar);
    buttonBar->Bind(wxEVT_LEFT_DOWN, &FlatUIButtonBar::OnMouseDown, buttonBar);
}

void FlatUIEventManager::bindHomeSpaceEvents(FlatUIHomeSpace* homeSpace)
{
    if (!homeSpace) return;
    
    homeSpace->Bind(wxEVT_PAINT, &FlatUIHomeSpace::OnPaint, homeSpace);
    homeSpace->Bind(wxEVT_LEFT_DOWN, &FlatUIHomeSpace::OnMouseDown, homeSpace);
    homeSpace->Bind(wxEVT_MOTION, &FlatUIHomeSpace::OnMouseMove, homeSpace);
    homeSpace->Bind(wxEVT_LEAVE_WINDOW, &FlatUIHomeSpace::OnMouseLeave, homeSpace);
    
    homeSpace->Bind(wxEVT_SIZE, [homeSpace](wxSizeEvent& event){
        homeSpace->CalculateButtonRect(event.GetSize());
        homeSpace->Refresh();
        event.Skip();
    });
}

void FlatUIEventManager::bindSystemButtonsEvents(FlatUISystemButtons* systemButtons)
{
    if (!systemButtons) return;
    
    systemButtons->Bind(wxEVT_PAINT, &FlatUISystemButtons::OnPaint, systemButtons);
    systemButtons->Bind(wxEVT_LEFT_DOWN, &FlatUISystemButtons::OnMouseDown, systemButtons);
    systemButtons->Bind(wxEVT_MOTION, &FlatUISystemButtons::OnMouseMove, systemButtons);
    systemButtons->Bind(wxEVT_LEAVE_WINDOW, &FlatUISystemButtons::OnMouseLeave, systemButtons);
}

void FlatUIEventManager::bindFunctionSpaceEvents(FlatUIFunctionSpace* functionSpace)
{
    if (!functionSpace) return;
    
}

void FlatUIEventManager::bindProfileSpaceEvents(FlatUIProfileSpace* profileSpace)
{
    if (!profileSpace) return;
    
}

void FlatUIEventManager::bindGalleryEvents(FlatUIGallery* gallery)
{
    if (!gallery) return;
    
    gallery->Bind(wxEVT_PAINT, &FlatUIGallery::OnPaint, gallery);
    gallery->Bind(wxEVT_LEFT_DOWN, &FlatUIGallery::OnMouseDown, gallery);
}

void FlatUIEventManager::bindSizeEvents(wxWindow* control, std::function<void(wxSizeEvent&)> handler)
{
    if (!control || !handler) return;
    
    control->Bind(wxEVT_SIZE, [handler](wxSizeEvent& event) {
        handler(event);
    });
} 