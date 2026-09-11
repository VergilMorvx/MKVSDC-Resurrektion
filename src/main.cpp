// mkvdc - ReXGlue Recompiled Project

#if defined(_WIN32)
#include <windows.h>
// Force high-performance discrete GPU on laptops with dual GPUs (NVIDIA Optimus / AMD PowerXpress)
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "generated/default/mkvdc_init.h"

#include "mkvdc_app.h"

REX_DEFINE_APP(mkvdc, MkvdcApp::Create)

