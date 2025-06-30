#include "flatui/FlatUIBarSpaceController.h"
#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUIConstants.h"
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"

#define CFG_INT(key, def)    ConstantsConfig::getInstance().getIntValue(key, def)

FlatUIBarSpaceController::FlatUIBarSpaceController(FlatUIBar* bar)
    : m_bar(bar) {}

void FlatUIBarSpaceController::SetSpaceConfig(SpaceType type, const SpaceConfig& config) {
    m_spaceConfigs[type] = config;
    // Note: Actual visibility and width updates are handled by specific methods or layout manager.
}

FlatUIBarSpaceController::SpaceConfig FlatUIBarSpaceController::GetSpaceConfig(SpaceType type) const {
    auto it = m_spaceConfigs.find(type);
    if (it != m_spaceConfigs.end()) {
        return it->second;
    }
    return {false, 0, false};
}

void FlatUIBarSpaceController::SetHomeButtonMenu(wxMenu* menu)
{
    if (m_bar->m_homeSpace) {
        // m_homeSpace->SetMenu(menu); // Assuming FlatUIHomeSpace handles menu internally now
        // If menu setting is still relevant, FlatUIHomeSpace needs a public SetMenu method.
    }
}

void FlatUIBarSpaceController::SetHomeButtonIcon(const wxBitmap& icon)
{
    if (m_bar->m_homeSpace) {
        m_bar->m_homeSpace->SetIcon(icon);
        if (m_bar->IsShown()) {
            m_bar->m_layoutManager->UpdateLayout(m_bar->GetClientSize());
            m_bar->Refresh();
        }
    }
}

void FlatUIBarSpaceController::SetHomeButtonWidth(int width)
{
    if (m_bar->m_homeSpace && width > 0) {
        m_bar->m_homeSpace->SetButtonWidth(width);
        if (m_bar->IsShown()) {
            m_bar->m_layoutManager->UpdateLayout(m_bar->GetClientSize());
            m_bar->Refresh();
        }
    }
}

void FlatUIBarSpaceController::SetFunctionSpaceControl(wxWindow* funcControl, int width)
{
    if (m_bar->m_functionSpace) {
        m_bar->m_functionSpace->SetChildControl(funcControl);
        if (width > 0) m_bar->m_functionSpace->SetSpaceWidth(width);
        // Only show if control exists and user toggle state is visible
        bool shouldShow = (funcControl != nullptr) && m_bar->GetFunctionSpaceUserVisible();
        m_bar->m_functionSpace->Show(shouldShow);
        if (m_bar->IsShown()) {
            m_bar->m_layoutManager->UpdateLayout(m_bar->GetClientSize());
            m_bar->Refresh();
        }
    }
}

void FlatUIBarSpaceController::ToggleFunctionSpaceVisibility()
{
    if (m_bar->m_functionSpace) {
        bool visible = m_bar->m_functionSpace->IsShown();
        bool newVisible = !visible;
        m_bar->SetFunctionSpaceUserVisible(newVisible);  // Update user toggle state
        m_bar->m_functionSpace->Show(newVisible);
        if (m_bar->m_tabFunctionSpacer) {
            m_bar->m_tabFunctionSpacer->Show(newVisible);
        }
        if (m_bar->IsShown()) {
            m_bar->m_layoutManager->UpdateLayout(m_bar->GetClientSize());
            m_bar->Refresh();
        }
    }
}

void FlatUIBarSpaceController::SetProfileSpaceControl(wxWindow* profControl, int width)
{
    if (m_bar->m_profileSpace) {
        m_bar->m_profileSpace->SetChildControl(profControl);
        if (width > 0) m_bar->m_profileSpace->SetSpaceWidth(width);
        // Only show if control exists and user toggle state is visible
        bool shouldShow = (profControl != nullptr) && m_bar->GetProfileSpaceUserVisible();
        m_bar->m_profileSpace->Show(shouldShow);
        if (m_bar->IsShown()) {
            m_bar->m_layoutManager->UpdateLayout(m_bar->GetClientSize());
            m_bar->Refresh();
        }
    }
}

void FlatUIBarSpaceController::ToggleProfileSpaceVisibility()
{
    if (m_bar->m_profileSpace) {
        bool visible = m_bar->m_profileSpace->IsShown();
        bool newVisible = !visible;
        m_bar->SetProfileSpaceUserVisible(newVisible);  // Update user toggle state
        m_bar->m_profileSpace->Show(newVisible);
        if (m_bar->m_functionProfileSpacer) {
            m_bar->m_functionProfileSpacer->Show(newVisible);
        }
        if (m_bar->IsShown()) {
            m_bar->m_layoutManager->UpdateLayout(m_bar->GetClientSize());
            m_bar->Refresh();
        }
    }
}

void FlatUIBarSpaceController::UpdateSpaceVisibility(wxWindow* spaceControl, bool visible) {
    if (spaceControl) {
        spaceControl->Show(visible);
    }
}

void FlatUIBarSpaceController::UpdateSpaceWidth(wxWindow* spaceControl, int width) {
    // This method might need to be more specific depending on the control type
    // For FlatUIFunctionSpace/FlatUIProfileSpace, it might call SetSpaceWidth()
    // For others, it might not be directly applicable or handled by SetSize during layout.
} 