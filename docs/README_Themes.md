# wxGUI 主题配置系统使用说明

## 概述
wxGUI支持三种主题：**亮色 (Default Light)**、**暗色 (Dark)** 和 **淡蓝色 (Modern Blue)**。所有UI颜色和SVG图标都会根据选择的主题自动调整。

## 主题切换
在 `config.ini` 文件中修改以下设置：
```ini
[Theme]
CurrentTheme=dark    # 可选值: default, dark, blue
```

## 配置格式
所有主题颜色使用统一格式：`配置键=默认值,暗色值,蓝色值`

示例：
```ini
DefaultTextColour=100,100,100;220,220,220;80,80,100
```

## 三种主题配色方案

### 🌟 亮色主题 (default)
- **背景色**: 浅灰白色系 (250,250,250)
- **文字色**: 深灰色 (100,100,100)
- **强调色**: 标准蓝 (0,120,215)
- **适用**: 白天使用，清晰明亮

### 🌙 暗色主题 (dark)
- **背景色**: 深灰色系 (45,45,48)
- **文字色**: 浅灰色 (220,220,220)
- **强调色**: 稍浅蓝 (0,122,204)
- **适用**: 夜间使用，护眼舒适

### 💙 淡蓝色主题 (blue)
- **背景色**: 淡蓝白色系 (240,245,250)
- **文字色**: 深蓝灰 (80,80,100)
- **强调色**: 标准蓝 (0,120,215)
- **适用**: 现代清新风格

## SVG图标主题化
SVG图标会根据主题自动调整颜色：

### 颜色映射规则
- **黑色** (#000000) → 主图标色
- **深灰** (#333333) → 次要图标色
- **中灰** (#808080) → 禁用图标色
- **蓝色** (#0078d4) → 强调图标色

### SVG配置
```ini
[SvgTheme]
SvgThemeEnabled=true
SvgPrimaryIconColour=100,100,100;220,220,220;80,80,100
SvgSecondaryIconColour=70,70,70;170,170,170;100,120,140
SvgDisabledIconColour=180,180,180;120,120,120;160,170,180
SvgHighlightIconColour=0,120,215;0,122,204;0,120,215
```

## 主要配色区域

### 📋 界面框架
- `PrimaryContentBgColour` - 主内容背景
- `DefaultBgColour` - 默认背景
- `FrameAppWorkspaceColour` - 应用工作区

### 📝 文字显示
- `DefaultTextColour` - 默认文字
- `MenuTextColour` - 菜单文字
- `ErrorTextColour` - 错误文字
- `PlaceholderTextColour` - 占位文字

### 🎯 栏和面板
- `BarBackgroundColour` - 工具栏背景
- `BarActiveTextColour` - 活动文字
- `PanelHeaderColour` - 面板标题背景

### 🔲 按钮和控件
- `ButtonbarDefaultBgColour` - 按钮背景
- `ButtonbarDefaultHoverBgColour` - 按钮悬停
- `DropdownBackgroundColour` - 下拉框背景

## 使用示例

### 在代码中使用主题颜色
```cpp
// 获取主题颜色
wxColour bgColor = CFG_COLOUR("PrimaryContentBgColour");
wxColour textColor = CFG_COLOUR("DefaultTextColour");

// 获取主题化SVG图标
wxBitmap icon = SVG_THEMED_ICON("home", wxSize(16, 16));

// 获取主题字体
wxFont font = CFG_DEFAULTFONT();
```

### 动态切换主题
```cpp
// 切换到暗色主题
ThemeManager::getInstance().setCurrentTheme("dark");

// 所有UI和SVG图标会自动更新颜色
```

## 配置文件结构
```
config/
├── config.ini          # 主配置文件
├── icons/svg/          # SVG图标文件夹
└── README_Themes.md    # 本说明文档
```

## 自定义主题颜色
1. 修改 `config.ini` 中的 `[ThemeColors]` 段落
2. 使用格式：`颜色键=亮色值;暗色值;蓝色值`
3. 重启应用以生效

## 注意事项
- 颜色值格式：R,G,B (0-255)
- 分隔符使用分号 (;)
- 尺寸配置不随主题变化
- SVG图标需要是标准颜色格式才能被正确替换

## 故障排除
1. **颜色不正确**: 检查config.ini中颜色格式是否正确
2. **SVG不变色**: 确认SvgThemeEnabled=true且SVG使用标准颜色
3. **主题不切换**: 检查CurrentTheme值是否为有效选项(default/dark/blue) 