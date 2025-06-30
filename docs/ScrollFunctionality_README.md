# Scroll Functionality for FlatUIFixPanel and FlatUIFloatPanel

## Overview

The scroll functionality has been added to both FlatUIFixPanel and FlatUIFloatPanel to handle situations where the content width exceeds the available panel width. When this occurs, left and right scroll buttons appear automatically to allow users to navigate through the content.

## Features

### Automatic Scroll Detection
- Panels automatically detect when content width exceeds container width
- Scroll buttons are only shown when scrolling is needed
- When content fits within the container, scroll buttons are hidden

### Scroll Controls
- Left scroll button (`<`) - scrolls content to the left
- Right scroll button (`>`) - scrolls content to the right
- Buttons are automatically enabled/disabled based on scroll position
- Scroll step is configurable (default: 50 pixels)

### Smart Layout Management
- When scrolling is needed:
  - Layout: `[<] [Content Container] [>]`
  - Scroll buttons take minimal space (20px width each)
  - Content container takes remaining space
- When scrolling is not needed:
  - Layout: `[Content Container]`
  - Content container takes full available space

## Implementation Details

### FlatUIFixPanel

#### New Members
- `bool m_scrollingEnabled` - Whether scrolling is currently enabled
- `wxPanel* m_scrollContainer` - Container panel for page content
- `wxButton* m_leftScrollButton` - Left scroll button
- `wxButton* m_rightScrollButton` - Right scroll button
- `int m_scrollOffset` - Current horizontal scroll offset
- `int m_scrollStep` - Pixels to scroll per button click (default: 50)
- `wxBoxSizer* m_mainSizer` - Main horizontal sizer for layout
- `wxBoxSizer* m_scrollSizer` - Sizer for scroll container content

#### New Methods
- `EnableScrolling(bool enable)` - Enable/disable scroll functionality
- `ScrollLeft()` - Scroll content to the left
- `ScrollRight()` - Scroll content to the right
- `UpdateScrollButtons()` - Update button visibility and state
- `NeedsScrolling()` - Check if content requires scrolling
- `CreateScrollControls()` - Create scroll button controls
- `UpdateScrollPosition()` - Update content position based on scroll offset

### FlatUIFloatPanel

Same implementation as FlatUIFixPanel, adapted for the float panel's layout structure.

## Usage Example

```cpp
// Enable scrolling manually (optional - auto-enabled when needed)
fixPanel->EnableScrolling(true);

// Scrolling is automatically managed when content is added
FlatUIPage* page = new FlatUIPage(parent, "Wide Content Page");
// ... add panels with many controls to the page ...
fixPanel->AddPage(page);
fixPanel->SetActivePage(page);

// If page content width > panel width, scroll buttons will appear automatically

// Manual scrolling (optional - users can use the buttons)
fixPanel->ScrollLeft();
fixPanel->ScrollRight();
```

## Automatic Behavior

1. **Content Detection**: When a page is set as active, the panel calculates if the page's best size width exceeds the available container width.

2. **Layout Adjustment**: If scrolling is needed:
   - Main sizer is reconfigured to include scroll buttons
   - Content is positioned within the scroll container
   - Scroll buttons are shown and properly enabled/disabled

3. **Dynamic Updates**: When the panel is resized:
   - Scroll necessity is re-evaluated
   - Layout is automatically adjusted
   - Scroll position is maintained (or reset if no longer needed)

4. **Page Changes**: When switching between pages:
   - Scroll position is reset to the beginning
   - Scroll necessity is re-evaluated for the new page
   - Layout is updated accordingly

## Technical Notes

### Coordinate System
- Scroll offset represents the number of pixels the content is shifted left
- Negative offset moves content to the right (showing left portion)
- Scroll position is clamped between 0 and (content_width - container_width)

### Performance Considerations
- Scroll updates use immediate refresh and update calls for smooth scrolling
- Layout changes are minimized by checking current state before updates
- Button state updates only occur when necessary

### Visual Rendering
- Scroll containers use `wxCLIP_CHILDREN` to ensure content outside bounds is not visible
- Multiple refresh and update calls ensure proper redraw during scrolling
- Both page content and container are refreshed during scroll operations

### Integration with Existing Code
- Existing page positioning and layout code has been adapted to work with the scroll container
- Unpin button positioning remains unchanged and works over the scroll content
- Pin button in float panels is properly layered above scroll content

## Event Handling

The scroll buttons use standard wxWidgets button events:
- `wxID_BACKWARD` - Left scroll button
- `wxID_FORWARD` - Right scroll button

Events are handled by:
- `OnScrollLeft(wxCommandEvent& event)` 
- `OnScrollRight(wxCommandEvent& event)` 