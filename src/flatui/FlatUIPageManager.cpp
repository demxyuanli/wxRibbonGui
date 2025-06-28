#include "flatui/FlatUIPageManager.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIFixPanel.h"
#include "flatui/FlatUIFloatPanel.h"
#include "logger/Logger.h"

FlatUIPageManager::FlatUIPageManager()
{
    LOG_INF("FlatUIPageManager initialized", "PageManager");
}

void FlatUIPageManager::AddPage(FlatUIPage* page)
{
    if (!CanAddPage(page)) {
        LOG_ERR("Cannot add page - validation failed", "PageManager");
        return;
    }
    
    m_pages.push_back(page);
    NotifyPageAdded(page);
    
    LOG_INF("Added page '" + page->GetLabel().ToStdString() + "', total pages: " + 
            std::to_string(m_pages.size()), "PageManager");
}

void FlatUIPageManager::RemovePage(size_t index)
{
    if (!IsValidPageIndex(index)) {
        LOG_ERR("Cannot remove page - invalid index: " + std::to_string(index), "PageManager");
        return;
    }
    
    FlatUIPage* page = m_pages[index];
    NotifyPageRemoved(page);
    m_pages.erase(m_pages.begin() + index);
    
    LOG_INF("Removed page at index " + std::to_string(index) + 
            ", remaining pages: " + std::to_string(m_pages.size()), "PageManager");
}

void FlatUIPageManager::RemovePage(FlatUIPage* page)
{
    size_t index = FindPageIndex(page);
    if (index != INVALID_INDEX) {
        RemovePage(index);
    } else {
        LOG_ERR("Cannot remove page - page not found", "PageManager");
    }
}

void FlatUIPageManager::ClearPages()
{
    for (auto* page : m_pages) {
        if (page) {
            NotifyPageRemoved(page);
        }
    }
    
    size_t count = m_pages.size();
    m_pages.clear();
    LOG_INF("Cleared all pages, removed " + std::to_string(count) + " pages", "PageManager");
}

FlatUIPage* FlatUIPageManager::GetPage(size_t index) const
{
    if (IsValidPageIndex(index)) {
        return m_pages[index];
    }
    return nullptr;
}

size_t FlatUIPageManager::FindPageIndex(FlatUIPage* page) const
{
    auto it = std::find(m_pages.begin(), m_pages.end(), page);
    if (it != m_pages.end()) {
        return std::distance(m_pages.begin(), it);
    }
    return INVALID_INDEX;
}

bool FlatUIPageManager::ContainsPage(FlatUIPage* page) const
{
    return FindPageIndex(page) != INVALID_INDEX;
}

void FlatUIPageManager::SetPageActive(size_t index, bool active)
{
    FlatUIPage* page = GetPage(index);
    if (page) {
        page->SetActive(active);
        LOG_INF("Set page '" + page->GetLabel().ToStdString() + "' active: " + 
                (active ? "true" : "false"), "PageManager");
    }
}

void FlatUIPageManager::SetAllPagesInactive()
{
    for (auto* page : m_pages) {
        if (page) {
            page->SetActive(false);
        }
    }
    LOG_INF("Set all pages inactive", "PageManager");
}

FlatUIPage* FlatUIPageManager::GetActivePage() const
{
    for (auto* page : m_pages) {
        if (page && page->IsActive()) {
            return page;
        }
    }
    return nullptr;
}

void FlatUIPageManager::ShowPageInFixPanel(FlatUIPage* page, FlatUIFixPanel* fixPanel)
{
    if (!ValidatePagePointer(page) || !fixPanel) {
        LOG_ERR("Invalid parameters for ShowPageInFixPanel", "PageManager");
        return;
    }
    
    PreparePageForContainer(page, fixPanel);
    fixPanel->AddPage(page);
    LOG_INF("Showed page '" + page->GetLabel().ToStdString() + "' in FixPanel", "PageManager");
}

void FlatUIPageManager::ShowPageInFloatPanel(FlatUIPage* page, FlatUIFloatPanel* floatPanel)
{
    if (!ValidatePagePointer(page) || !floatPanel) {
        LOG_ERR("Invalid parameters for ShowPageInFloatPanel", "PageManager");
        return;
    }
    
    PreparePageForContainer(page, floatPanel);
    floatPanel->SetPageContent(page);
    LOG_INF("Showed page '" + page->GetLabel().ToStdString() + "' in FloatPanel", "PageManager");
}

void FlatUIPageManager::ReturnPageToOriginalParent(FlatUIPage* page, wxWindow* originalParent)
{
    if (!ValidatePagePointer(page) || !originalParent) {
        LOG_ERR("Invalid parameters for ReturnPageToOriginalParent", "PageManager");
        return;
    }
    
    SafeReparentPage(page, originalParent);
    page->Hide();
    LOG_INF("Returned page '" + page->GetLabel().ToStdString() + "' to original parent", "PageManager");
}

void FlatUIPageManager::ReturnAllPagesToOriginalParent(wxWindow* originalParent)
{
    if (!originalParent) {
        LOG_ERR("Invalid original parent for ReturnAllPagesToOriginalParent", "PageManager");
        return;
    }
    
    for (auto* page : m_pages) {
        if (page) {
            ReturnPageToOriginalParent(page, originalParent);
        }
    }
    LOG_INF("Returned all pages to original parent", "PageManager");
}

void FlatUIPageManager::TransferPageBetweenContainers(FlatUIPage* page, wxWindow* fromContainer, wxWindow* toContainer)
{
    if (!ValidatePagePointer(page) || !fromContainer || !toContainer) {
        LOG_ERR("Invalid parameters for TransferPageBetweenContainers", "PageManager");
        return;
    }
    
    CleanupPageFromContainer(page, fromContainer);
    PreparePageForContainer(page, toContainer);
    LOG_INF("Transferred page '" + page->GetLabel().ToStdString() + "' between containers", "PageManager");
}

void FlatUIPageManager::SafeReparentPage(FlatUIPage* page, wxWindow* newParent)
{
    if (!ValidatePagePointer(page) || !newParent) {
        LOG_ERR("Invalid parameters for SafeReparentPage", "PageManager");
        return;
    }
    
    try {
        page->Reparent(newParent);
        LOG_INF("Successfully reparented page '" + page->GetLabel().ToStdString() + "'", "PageManager");
    } catch (const std::exception& e) {
        LOG_ERR("Failed to reparent page: " + std::string(e.what()), "PageManager");
    }
}

bool FlatUIPageManager::IsValidPageIndex(size_t index) const
{
    return index < m_pages.size();
}

bool FlatUIPageManager::ValidatePagePointer(FlatUIPage* page) const
{
    return page != nullptr && ContainsPage(page);
}

void FlatUIPageManager::HideAllPages()
{
    for (auto* page : m_pages) {
        if (page && page->IsShown()) {
            page->Hide();
        }
    }
    LOG_INF("Hidden all pages", "PageManager");
}

void FlatUIPageManager::ShowOnlyPage(FlatUIPage* page)
{
    HideAllPages();
    if (ValidatePagePointer(page)) {
        page->Show();
        LOG_INF("Showing only page '" + page->GetLabel().ToStdString() + "'", "PageManager");
    }
}

wxString FlatUIPageManager::GetPageListString() const
{
    wxString result;
    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i]) {
            result += wxString::Format("[%zu] %s", i, m_pages[i]->GetLabel());
            if (i < m_pages.size() - 1) {
                result += ", ";
            }
        }
    }
    return result;
}

void FlatUIPageManager::NotifyPageAdded(FlatUIPage* page)
{
    if (page) {
        page->SetActive(false); // Start inactive
    }
}

void FlatUIPageManager::NotifyPageRemoved(FlatUIPage* page)
{
    if (page) {
        page->SetActive(false);
        page->Hide();
    }
}

void FlatUIPageManager::ValidatePageCollection() const
{
    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (!m_pages[i]) {
            LOG_ERR("Null page found at index " + std::to_string(i), "PageManager");
        }
    }
}

void FlatUIPageManager::PreparePageForContainer(FlatUIPage* page, wxWindow* container)
{
    if (page && container) {
        page->Reparent(container);
        page->Hide(); // Start hidden, container will show when appropriate
    }
}

void FlatUIPageManager::CleanupPageFromContainer(FlatUIPage* page, wxWindow* container)
{
    if (page) {
        page->Hide();
        // Reparenting will be handled by the calling code
    }
}

bool FlatUIPageManager::CanAddPage(FlatUIPage* page) const
{
    return page != nullptr && !ContainsPage(page);
}

bool FlatUIPageManager::CanRemovePage(FlatUIPage* page) const
{
    return page != nullptr && ContainsPage(page);
} 