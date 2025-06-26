/*
 * FloatPanelDemo.cpp
 * 
 * This file demonstrates how to use the new FlatUIFloatPanel
 * as a replacement for wxPopupTransientWindow-based floating windows.
 * 
 * The FlatUIFloatPanel provides:
 * - Custom drawing with borders and shadows
 * - Auto-hide functionality based on mouse position
 * - Complete control over appearance and behavior
 * - No dependency on wxPopupTransientWindow
 */

#include "flatui/FlatUIFloatPanel.h"
#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h"

/*
 * Example usage in your application:
 * 
 * // In your FlatUIBar setup:
 * FlatUIBar* bar = new FlatUIBar(parent);
 * 
 * // The FloatPanel is now the default and only implementation
 * 
 * // You can also create and use FloatPanel directly:
 * FlatUIFloatPanel* floatPanel = new FlatUIFloatPanel(parent);
 * FlatUIPage* page = new FlatUIPage(parent, "Demo Page");
 * 
 * // Set content and show at specific position
 * floatPanel->SetPageContent(page);
 * floatPanel->ShowAt(wxPoint(100, 100), wxSize(400, 200));
 * 
 * // The panel will auto-hide when mouse moves away
 * // or you can hide it manually:
 * floatPanel->HidePanel();
 */

/*
 * Key differences from wxPopupTransientWindow:
 * 
 * 1. Custom Drawing:
 *    - FloatPanel draws its own borders and shadows
 *    - Fully customizable appearance
 *    - No reliance on system popup window styling
 * 
 * 2. Auto-hide Logic:
 *    - Uses timer-based checking instead of system events
 *    - More reliable across different platforms
 *    - Can be customized based on specific requirements
 * 
 * 3. Event Handling:
 *    - Uses standard wxFrame events
 *    - More predictable behavior
 *    - Better integration with parent window
 * 
 * 4. Memory Management:
 *    - Explicit control over lifetime
 *    - No unexpected destruction by system
 *    - Proper cleanup of resources
 */

/*
 * Configuration options available:
 * 
 * The FloatPanel appearance can be customized by modifying:
 * - m_borderColour: Color of the border
 * - m_backgroundColour: Background color of the panel
 * - m_borderWidth: Width of the border in pixels
 * - m_shadowOffset: Offset for drop shadow effect
 * - m_shadowColour: Color of the drop shadow
 * 
 * Auto-hide behavior can be adjusted by changing:
 * - AUTO_HIDE_DELAY_MS: Delay before checking for auto-hide
 * - Margin around panel for mouse detection
 * - Logic for determining when to hide
 */

/*
 * Integration with existing FlatUIBar:
 * 
 * The FlatUIBar class now uses the custom FloatPanel implementation
 * exclusively, providing the following interface:
 * - SetPageContent() to set the displayed page
 * - ShowAt() to show at a specific position
 * - Hide functionality for dismissing the panel
 * - Event notifications when dismissed
 */ 