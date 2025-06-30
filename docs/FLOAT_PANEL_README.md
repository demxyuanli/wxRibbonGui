# FlatUIFloatPanel - 定制浮动工具栏实现

## 概述

FlatUIFloatPanel 是一个完全定制的浮动面板实现，用于替代基于 wxPopupTransientWindow 的浮动窗口。它专门为 unpinned 状态下的浮动工具栏而设计，提供了更好的控制能力和跨平台兼容性。

## 主要特性

### 1. 完全定制的外观
- **自定义边框绘制**: 可配置边框颜色、宽度
- **阴影效果**: 支持可配置的阴影偏移和颜色
- **背景定制**: 可设置背景颜色以匹配应用主题
- **无系统依赖**: 不依赖系统弹出窗口样式

### 2. 智能自动隐藏
- **基于鼠标位置**: 当鼠标移出面板区域时自动隐藏
- **定时器机制**: 使用 wxTimer 进行定期检查，比系统事件更可靠
- **父窗口感知**: 当鼠标在父工具栏区域时不会隐藏
- **可配置延迟**: 可调整自动隐藏的检查间隔

### 3. 强大的事件处理
- **标准 wxFrame 事件**: 使用标准的 wxWidgets 事件系统
- **鼠标进入/离开**: 响应鼠标进入和离开事件
- **焦点管理**: 处理窗口激活和失去焦点事件
- **全局鼠标跟踪**: 监控全局鼠标移动以实现自动隐藏

### 4. 内存管理
- **显式生命周期控制**: 不会被系统意外销毁
- **资源清理**: 正确清理所有资源和事件绑定
- **页面管理**: 自动处理页面内容的父子关系变更

## 文件结构

```
include/flatui/FlatUIFloatPanel.h    - 头文件定义
src/flatui/FlatUIFloatPanel.cpp      - 实现文件
src/flatui/FloatPanelDemo.cpp        - 使用示例和说明
```

## 核心类设计

### FlatUIFloatPanel 类

继承自 `wxFrame`，提供以下主要接口：

```cpp
class FlatUIFloatPanel : public wxFrame
{
public:
    // 构造和析构
    FlatUIFloatPanel(wxWindow* parent);
    virtual ~FlatUIFloatPanel();
    
    // 内容管理
    void SetPageContent(FlatUIPage* page);
    FlatUIPage* GetCurrentPage() const;
    
    // 显示控制
    void ShowAt(const wxPoint& position, const wxSize& size = wxDefaultSize);
    void HidePanel();
    void ForceHide();
    
    // 自动隐藏逻辑
    bool ShouldAutoHide(const wxPoint& globalMousePos) const;
};
```

## 与 FlatUIBar 的集成

FlatUIBar 现在使用定制的 FloatPanel 作为唯一的浮动窗口实现，完全替代了原有的 wxPopupTransientWindow 实现。

### 使用方式

```cpp
// 创建 FlatUIBar
FlatUIBar* bar = new FlatUIBar(parent);

// FloatPanel 现在是默认且唯一的实现
// 在 unpinned 模式下点击标签页时会自动使用 FloatPanel
```

### 提供的方法

```cpp
// FlatUIBar 中的 FloatPanel 方法
void ShowPageInFloatPanel(FlatUIPage* page);
void HideFloatPanel();
void OnFloatPanelDismissed(wxCommandEvent& event);
```

## 技术实现细节

### 1. 窗口样式
```cpp
wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT | wxBORDER_NONE
```
- `wxFRAME_NO_TASKBAR`: 不在任务栏显示
- `wxFRAME_FLOAT_ON_PARENT`: 浮动在父窗口之上
- `wxBORDER_NONE`: 无系统边框，使用自定义绘制

### 2. 自定义绘制
```cpp
void OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    DrawShadow(dc);      // 绘制阴影
    DrawCustomBorder(dc); // 绘制边框
}
```

### 3. 自动隐藏机制
```cpp
// 定时器检查
void OnAutoHideTimer(wxTimerEvent& event)
{
    CheckAutoHide();
}

// 判断是否应该隐藏
bool ShouldAutoHide(const wxPoint& globalMousePos) const
{
    wxRect panelRect = GetScreenRect();
    panelRect.Inflate(5, 5); // 添加容差边距
    
    bool mouseOutside = !panelRect.Contains(globalMousePos);
    bool mouseOverParent = false;
    
    if (m_parentWindow) {
        wxRect parentRect = m_parentWindow->GetScreenRect();
        mouseOverParent = parentRect.Contains(globalMousePos);
    }
    
    return mouseOutside && !mouseOverParent;
}
```

## 配置选项

### 外观配置
```cpp
// 在 SetupAppearance() 中可配置
m_borderColour = wxColour(180, 180, 180);    // 边框颜色
m_backgroundColour = wxColour(248, 248, 248); // 背景颜色
m_shadowColour = wxColour(0, 0, 0, 50);      // 阴影颜色
m_borderWidth = 1;                           // 边框宽度
m_shadowOffset = 3;                          // 阴影偏移
```

### 行为配置
```cpp
static const int AUTO_HIDE_DELAY_MS = 100;  // 自动隐藏检查间隔
```

## 使用示例

### 基本使用
```cpp
// 创建浮动面板
FlatUIFloatPanel* panel = new FlatUIFloatPanel(parentWindow);

// 创建页面内容
FlatUIPage* page = new FlatUIPage(parentWindow, "工具页面");

// 设置内容并显示
panel->SetPageContent(page);
panel->ShowAt(wxPoint(100, 50), wxSize(400, 80));

// 面板会在鼠标移开时自动隐藏
```

### 与 FlatUIBar 集成使用
```cpp
// 在应用初始化时
FlatUIBar* toolBar = new FlatUIBar(mainFrame);

// 添加页面
FlatUIPage* homePage = new FlatUIPage(toolBar, "主页");
FlatUIPage* editPage = new FlatUIPage(toolBar, "编辑");
toolBar->AddPage(homePage);
toolBar->AddPage(editPage);

// 设置为 unpinned 模式
toolBar->SetGlobalPinned(false);

// 现在点击标签页时会自动显示定制的浮动面板
```

## 相比 wxPopupTransientWindow 的优势

1. **更好的控制**: 完全控制外观和行为
2. **跨平台一致性**: 不依赖系统特定的弹出窗口实现
3. **可预测的生命周期**: 不会被系统意外销毁
4. **更好的事件处理**: 使用标准 wxFrame 事件系统
5. **可定制的自动隐藏**: 基于应用逻辑而非系统行为
6. **更好的调试支持**: 标准窗口更容易调试和分析
7. **简化的维护**: 单一实现，减少代码复杂性

## 注意事项

1. **全局鼠标跟踪**: 需要正确绑定和解绑全局鼠标事件
2. **页面父子关系**: 正确管理页面在不同父窗口间的重新分配
3. **定时器管理**: 确保在销毁时停止所有定时器
4. **内存清理**: 在析构函数中正确清理所有资源

## 扩展可能性

1. **动画效果**: 可添加显示/隐藏动画
2. **更复杂的自动隐藏逻辑**: 基于用户交互模式
3. **主题支持**: 与应用主题系统集成
4. **多显示器支持**: 改进跨显示器的位置计算
5. **可拖拽支持**: 允许用户拖拽浮动面板 