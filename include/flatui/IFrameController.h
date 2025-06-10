#pragma once
#include <wx/wx.h>

class IFrameController {
public:
    virtual ~IFrameController() = default;
    virtual void SetMinimumSize(const wxSize& size) = 0;
    virtual void HandleResize(const wxSize& newSize) = 0;
    virtual void SetAdaptiveUIEnabled(bool enabled) = 0;
};

class IUIBarController {
public:
    virtual ~IUIBarController() = default;
    virtual void SetFunctionSpaceVisible(bool visible) = 0;
    virtual void SetProfileSpaceVisible(bool visible) = 0;
    virtual int CalculateMinimumWidth() const = 0;
    virtual int CalculateMinimumHeight() const = 0;
};