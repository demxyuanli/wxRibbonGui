# FlatUI wxWidgets GUI 项目

## 简介

本项目基于 wxWidgets，由人类进行架构设想，逐步提出设想，代码编写部分完全由AI（涉及Claude、Grok、Gemini、DeekSeek等）生成的C++代码构建的 FlatUI 框架组件库。
本项目逐步实现了提供现代化、扁平化风格的多页面 Ribbon 界面。适用于 Windows 平台的桌面应用开发，支持自定义按钮栏、面板、页面、Gallery、系统按钮等丰富控件。
目前本项目还在继续开发中，当前版本代码还有诸多bug正在修改，代码重构和优化程度也还未迭代开展。

## 主要特性

- **FlatUIButtonBar**：自定义按钮栏，支持图标、文字、下拉菜单、悬浮高亮、边框样式等。
- **FlatUIPanel**：可嵌入按钮栏、Gallery、支持自定义边框和标题样式的面板。
- **FlatUIPage**：Ribbon 页签式多页面容器，支持动态添加/切换页面。
- **FlatUIGallery**：图标集控件，适合展示多种操作项。
- **FlatUIHomeMenu**：主菜单支持分组、分隔符、快捷键。
- **UIHierarchyDebugger**：UI 层级调试与布局打印工具。
- **高度可定制**：支持多种配色、边框、圆角、间距、字体等参数设置。

## 目录结构

```
.
├── include/           # 头文件（flatui 组件API、主框架等）
│   └── flatui/
├── src/               # 源码
│   ├── flatui/        # FlatUI 组件实现
│   ├── FlatFrame.cpp  # 主窗口实现
│   ├── MainApplication.cpp # 应用入口
│   └── resource.rc    # 资源文件（图标、位图等）
├── CMakeLists.txt     # CMake 构建脚本
└── README.md          # 项目说明
```

## 编译方法

1. **依赖环境**
   - wxWidgets 3.x（建议 3.1 及以上）
   - CMake 3.10+
   - C++17 编译器（如 MSVC、MinGW、Clang）

2. **编译步骤**
   ```sh
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```
   > Windows 下建议用 Visual Studio 打开 CMake 生成的解决方案。

3. **资源说明**
   - `src/resource.rc` 及相关 PNG 位图需正确嵌入到可执行文件。
   - 若遇到"Couldn't load resource bitmap ..."等错误，请确保资源文件和图片路径无误，并在主程序初始化时调用 `wxInitAllImageHandlers()`。

## 主要类与用法

- **FlatFrame**  
  主窗口类，集成 Ribbon、搜索栏、消息输出、主菜单等，支持多页面和自定义面板。
- **FlatUIButtonBar**  
  用法示例：
  ```cpp
  FlatUIButtonBar* bar = new FlatUIButtonBar(panel);
  wxBitmap bmp("IDP_OPEN", wxBITMAP_TYPE_PNG_RESOURCE);
  bar->AddButton(wxID_OPEN, "打开", bmp);
  ```
  > 按钮图标会自动缩放为 16x16。

- **FlatUIPanel/FlatUIPage**  
  支持横向/纵向布局、嵌套按钮栏、Gallery、标题栏等。

## 常见问题

- **位图无法加载/显示异常**  
  - 请确保 PNG 资源已正确嵌入，并在主程序构造时调用 `wxInitAllImageHandlers()`。
  - 按钮图标会自动缩放为 16x16，无需手动处理尺寸。

- **菜单弹出位置不对**  
  - 弹出菜单已优化为紧贴按钮下边缘弹出，若有特殊需求可调整 `FlatUIButtonBar::OnMouseDown` 相关代码。

## 贡献与许可

- 本项目为学习与交流用途，欢迎二次开发与反馈建议。
