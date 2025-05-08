#include "MainApplication.h"
#include "logger/Logger.h"
#include <vtkObject.h>

bool MainApplication::OnInit() {
    SetDllDirectory(L"lib");
    return true;
}

wxIMPLEMENT_APP(MainApplication);