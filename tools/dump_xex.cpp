#include <rex/runtime.h>
#include <rex/kernel/init.h>
#include <rex/system/kernel_state.h>
#include <rex/system/user_module.h>
#include <rex/system/xex_module.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <windows.h>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: dump_xex <game_dir> <out_bin>\n";
        return 1;
    }
    std::string game_dir = argv[1];
    std::string out_bin = argv[2];
    
    rex::Runtime runtime(game_dir);
    auto setup_status = runtime.Setup(rex::RuntimeConfig{
        .kernel_init = rex::kernel::InitializeKernel,
        .tool_mode = true,
    });
    if (setup_status != 0) {
        std::cerr << "Failed to setup runtime: 0x" << std::hex << setup_status << "\n";
        return 1;
    }
    auto status = runtime.LoadXexImage("game:\\default.xex");
    if (status != 0) {
        std::cerr << "Failed to load XEX: 0x" << std::hex << status << "\n";
        return 1;
    }
    auto user_module = runtime.kernel_state()->GetExecutableModule();
    if (!user_module) {
        std::cerr << "No executable module\n";
        return 1;
    }
    auto* mod = user_module->xex_module();
    uint32_t base = mod->base_address();
    uint32_t size = mod->image_size();
    std::cout << "Base: 0x" << std::hex << base << ", Size: 0x" << size << std::dec << "\n";
    
    auto* mem = runtime.memory();
    std::ofstream out(out_bin, std::ios::binary);
    std::vector<uint8_t> page(4096, 0);
    for (uint32_t addr = base; addr < base + size; addr += 4096) {
        uint32_t chunk = std::min(4096u, base + size - addr);
        uint8_t* host_ptr = mem->virtual_membase() + addr;
        __try {
            out.write(reinterpret_cast<const char*>(host_ptr), chunk);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            std::fill(page.begin(), page.end(), 0);
            out.write(reinterpret_cast<const char*>(page.data()), chunk);
        }
    }
    std::cout << "Wrote mapped memory to " << out_bin << "\n";
    return 0;
}
