# wxRibbonGui - 扁平化 UI 框架

## 注：本项目的代码编写完全由AI生成，使用的AI包括(Grok3、Gemini、gpt4、DeekSeek3.1、Claude3.7)，其中主力智能体使用了Grok3的免费版本，IDE使用的是Visual Studio 2022，AI编程工具Trae，Cousor主要用于获得智能体的访问途径，为此付费了Cousor的192美元的Pro版本(得说一句，Cousor的付费版本真正是个坑，秉承了阿三的骗钱风格)

## 项目简介

wxRibbonGui 是一个基于 [wxWidgets](https://www.wxwidgets.org/) 库开发的图形用户界面 (GUI) 组件，旨在提供一个高度可定制和现代化的扁平化用户界面体验，主要实现了基于原生wxWidgets基础控件（主要是wxControl）的Ribbon菜单界面，后续后增加更多的组件和控件。

开发这个组件的目标主要是解决开源或商用的美化GUI界面和OpenGL应用之间存在的因为渲染冲突导致的性能下降，渲染失败等问题，而尝试开发的更为基础和基本特效的GUI组件。

项目通过一系列自定义控件和事件处理机制，实现了传统 wxWidgets 应用程序之外的独特视觉效果和交互行为，例如无边框窗口、自定义标题栏和丰富的 UI 布局。

本项目的核心目标是为 wxWidgets 应用程序提供一个现代、美观且易于扩展的 UI 框架，特别适用于需要自定义外观和复杂交互的应用。

## 主要特性

*   **无边框窗口**: 提供自定义的窗口拖动、调整大小和伪最大化功能，摆脱传统窗口边框的束缚，实现更自由的界面设计。
*   **扁平化 UI 组件**: 包含自定义的导航栏 (`FlatUIBar`)、页面 (`FlatUIPage`)、面板 (`FlatUIPanel`) 以及各种扁平化风格的按钮和画廊控件，所有组件都经过精心设计，以提供一致的视觉体验。
*   **模块化设计**: UI 组件被设计为独立的模块，每个模块负责特定的 UI 功能，易于理解、扩展和重用，降低了开发复杂性。
*   **高度可定制性**: 丰富的配置选项允许开发者调整 UI 元素的颜色、样式、布局、边框等，以满足不同的设计需求和品牌指南。
*   **事件驱动架构**: 基于 wxWidgets 强大的事件处理机制，实现灵活的用户交互和组件间通信。
*   **UI 调试工具**: 内置 UI 层次结构调试功能，帮助开发者快速定位和解决布局问题。

## 核心组件概览

项目结构清晰，主要功能由以下核心组件实现：

### 框架与主窗口

*   **`MainApplication`** (<mcfile name="MainApplication.h" path="d:\source\repos\wxgui\include\MainApplication.h"></mcfile>): 应用程序的入口点，负责 wxWidgets 环境的初始化和主窗口的创建。
*   **`MainFrame`** (<mcfile name="MainFrame.h" path="d:\source\repos\wxgui\include\MainFrame.h"></mcfile>): 应用程序的主窗口，继承自 `FlatFrame`，是所有 UI 交互的中心枢纽，负责集成各个 UI 组件和处理顶层事件。
*   **`FlatFrame`** (<mcfile name="FlatFrame.h" path="d:\source\repos\wxgui\include\FlatFrame.h"></mcfile>): 一个自定义的无边框窗口框架，提供了窗口拖动、调整大小和伪最大化等核心功能，是 `MainFrame` 的直接基类。
*   **`FlatUIFrame`** (<mcfile name="FlatUIFrame.h" path="d:\source\repos\wxgui\include\flatui\FlatUIFrame.h"></mcfile>): `FlatFrame` 的基类，处理无边框窗口的基本逻辑和鼠标事件，确保窗口行为的流畅性。
*   **`BorderlessFrameLogic`** (<mcfile name="BorderlessFrameLogic.h" path="d:\source\repos\wxgui\include\flatui\BorderlessFrameLogic.h"></mcfile>): 提供了无边框窗口拖动和调整大小的底层逻辑，是 `FlatUIFrame` 的基石。

### UI 布局与导航

*   **`FlatUIBar`** (<mcfile name="FlatUIBar.h" path="d:\source\repos\wxgui\include\flatui\FlatUIBar.h"></mcfile>): 位于应用程序顶部的导航栏，集成了主页按钮、可切换的页面标签、功能区和配置文件区，支持多种样式和高度定制。
*   **`FlatUIPage`** (<mcfile name="FlatUIPage.h" path="d:\source\repos\wxgui\include\flatui\FlatUIPage.h"></mcfile>): 用于组织应用程序内容的逻辑单元，每个页面可以包含多个 `FlatUIPanel`，实现多页面的应用结构。
*   **`FlatUIPanel`** (<mcfile name="FlatUIPanel.h" path="d:\source\repos\wxgui\include\flatui\FlatUIPanel.h"></mcfile>): 用于组织和布局 UI 元素的容器，可以容纳 `FlatUIButtonBar` 和 `FlatUIGallery` 等控件，支持自定义背景、边框和头部样式。
*   **`FlatUISpacerControl`** (<mcfile name="FlatUISpacerControl.h" path="d:\source\repos\wxgui\include\flatui\FlatUISpacerControl.h"></mcfile>): 用于在布局中创建可拖动或固定宽度的间隔，增强布局的灵活性和动态调整能力。

### 交互式控件

*   **`FlatUIButtonBar`** (<mcfile name="FlatUIButtonBar.h" path="d:\source\repos\wxgui\include\flatui\FlatUIButtonBar.h"></mcfile>): 包含一组扁平化风格按钮的水平或垂直布局容器，支持按钮的自定义图标、文本和事件处理。
*   **`FlatUIGallery`** (<mcfile name="FlatUIGallery.h" path="d:\source\repos\wxgui\include\flatui\FlatUIGallery.h"></mcfile>): 用于显示图像或图标集合的画廊控件，支持多种布局（水平、网格、流式）、样式和选择功能。
*   **`FlatUISystemButtons`** (<mcfile name="FlatUISystemButtons.h" path="d:\source\repos\wxgui\include\flatui\FlatUISystemButtons.h"></mcfile>): 自定义的窗口系统按钮（最小化、最大化、关闭），与无边框窗口无缝集成，提供统一的视觉风格。
*   **`FlatUIHomeMenu`** (<mcfile name="FlatUIHomeMenu.h" path="d:\source\repos\wxgui\include\flatui\FlatUIHomeMenu.h"></mcfile>): 当点击主页按钮时弹出的菜单，提供应用程序级别的操作入口，支持菜单项的图标和文本。

### 辅助与配置

*   **`FlatUIHomeSpace`, `FlatUIFunctionSpace`, `FlatUIProfileSpace`** (<mcfile name="FlatUIHomeSpace.h" path="d:\source\repos\wxgui\include\flatui\FlatUIHomeSpace.h"></mcfile>, <mcfile name="FlatUIFunctionSpace.h" path="d:\source\repos\wxgui\include\flatui\FlatUIFunctionSpace.h"></mcfile>, <mcfile name="FlatUIProfileSpace.h" path="d:\source\repos\wxgui\include\flatui\FlatUIProfileSpace.h"></mcfile>): `FlatUIBar` 内部用于划分不同功能区域的辅助控件，例如主页按钮区域、搜索功能区和用户配置文件显示区。
*   **`UIHierarchyDebugger`** (<mcfile name="UIHierarchyDebugger.h" path="d:\source\repos\wxgui\include\flatui\UIHierarchyDebugger.h"></mcfile>): 一个用于可视化和调试 wxWidgets UI 控件层次结构的工具，帮助开发者理解和排查布局问题。
*   **`ConfigManager.h`** (<mcfile name="ConfigManager.h" path="d:\source\repos\wxgui\include\config\ConfigManager.h"></mcfile>): 可能用于管理应用程序的配置设置。
*   **`Logger.h`** (<mcfile name="Logger.h" path="d:\source\repos\wxgui\include\logger\Logger.h"></mcfile>): 应用程序的日志系统，用于记录运行时信息和错误。

## 项目结构

```
FlatFrame.h
MainApplication.h
MainFrame.h
config/                 # 配置文件相关头文件
  Coin3DConfig.h
  ConfigManager.h
  LoggerConfig.h
flatui/                 # 扁平化 UI 框架的核心组件
  BorderlessFrameLogic.h
  FlatUIBar.h
  FlatUIButtonBar.h
  FlatUIConstants.h
  FlatUICustomControl.h
  FlatUIEventManager.h
  FlatUIFrame.h
  FlatUIFunctionSpace.h
  FlatUIGallery.h
  FlatUIHomeMenu.h
  FlatUIHomeSpace.h
  FlatUIPage.h
  FlatUIPanel.h
  FlatUIProfileSpace.h
  FlatUISpacerControl.h
  FlatUISystemButtons.h
  UIHierarchyDebugger.h
language/               # 语言管理相关头文件
  LanguageManager.h
logger/                 # 日志系统相关头文件
  Logger.h
```

## 构建与运行

本项目使用 C++ 和 wxWidgets 库开发。以下是通用的构建和运行指南：

### 前置条件

*   **C++ 编译器**: 支持 C++11 或更高标准的编译器 (如 GCC, Clang, MSVC)。
*   **wxWidgets 库**: 确保您的系统上已安装 wxWidgets 库。建议从 [wxWidgets 官方网站](https://www.wxwidgets.org/downloads/) 下载并根据其文档进行安装和配置。

### 构建步骤 (以 CMake 为例)

1.  **克隆仓库**: 如果您尚未克隆本项目，请先克隆：
    ```bash
    git clone <repository_url>
    cd wxgui
    ```

2.  **创建构建目录**: 建议在项目根目录外创建一个 `build` 目录：
    ```bash
    mkdir build
    cd build
    ```

3.  **配置 CMake**: 运行 CMake 配置项目。请根据您的 wxWidgets 安装路径调整 `CMAKE_PREFIX_PATH` 或 `wxWidgets_ROOT_DIR` 变量。
    ```bash
    cmake .. -DCMAKE_BUILD_TYPE=Release # 或 Debug
    # 如果需要指定 wxWidgets 路径，例如：
    # cmake .. -DCMAKE_BUILD_TYPE=Release -DwxWidgets_ROOT_DIR="C:/path/to/wxWidgets"
    ```

4.  **编译项目**: 使用 CMake 生成的构建系统进行编译：
    ```bash
    cmake --build .
    ```

### 运行应用程序

编译成功后，您可以在 `build` 目录（或其子目录，取决于您的构建系统配置）中找到生成的可执行文件。直接运行该文件即可启动应用程序。

```bash
.\Debug\wxapp.exe # Windows 示例
# 或
./wxapp # Linux/macOS 示例
```

## 使用与扩展

*   **创建新页面和面板**: 您可以通过继承 `FlatUIPage` 和 `FlatUIPanel` 来创建新的 UI 页面和面板，并将其添加到 `FlatUIBar` 中。
*   **自定义控件样式**: `FlatUIBar`, `FlatUIPanel`, `FlatUIGallery` 等组件提供了丰富的 `Set...Style` 和 `Set...Colour` 方法，允许您完全自定义其外观。
*   **集成新功能**: 在 `MainFrame` 中添加新的事件处理函数和 UI 元素，以扩展应用程序的功能。
*   **日志和配置**: 利用 `Logger` 和 `ConfigManager` 进行应用程序的日志记录和配置管理。

## 贡献

我们非常欢迎对本项目进行贡献！如果您有任何改进建议、新功能想法或发现 Bug，请随时通过以下方式参与：

1.  **提交 Issue**: 报告 Bug 或提出功能请求。
2.  **Fork 仓库**: 并在您的分支上进行修改。
3.  **提交 Pull Request**: 附上详细的修改说明和测试结果。

请确保您的代码符合项目现有的编码风格，并包含必要的注释。

## 许可证

本项目采用 [MIT 许可证](https://opensource.org/licenses/MIT) 发布。详情请参阅 `LICENSE` 文件。

```

这个版本包含了更详细的项目描述、核心组件功能、项目结构、构建和运行指南，以及如何使用和扩展项目的建议。它还包含了贡献指南和许可证信息。您觉得这个版本如何？
        