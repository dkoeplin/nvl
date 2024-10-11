#include "nvl/reflect/Backtrace.h"

#include <dlfcn.h>
#include <execinfo.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace nvl {

namespace {

// TODO: Make this more portable. This only works for macOS at the moment.
#ifdef __APPLE__
void signal_handler(const int sig) {
    static constexpr int kMaxDepth = 40;
    std::cout << "Received signal " << strsignal(sig) << std::endl;

    void *trace[kMaxDepth];
    const int size = backtrace(trace, kMaxDepth);

    // Skip the first few lines in the trace. It'll just point to the trace to the signal handler itself.
    // This is using a macOS specific utility (atos) to get the source/line information from the trace:
    // https://stackoverflow.com/questions/289820/getting-the-current-stack-trace-on-mac-os-x
    for (int i = 3; i < size; ++i) {
        Dl_info info;
        dladdr(trace[i], &info);

        std::stringstream cmd(std::ios_base::out);
        cmd << "atos -o " << info.dli_fname << " -l " << std::hex << reinterpret_cast<uint64_t>(info.dli_fbase) << ' '
            << reinterpret_cast<uint64_t>(trace[i]);

        FILE *atos = popen(cmd.str().c_str(), "r");

        constexpr int kBufferSize = 200;
        char buffer[kBufferSize];

        fgets(buffer, kBufferSize, atos);
        pclose(atos);

        std::cout << buffer;
    }
    std::cout << std::flush;
    exit(sig);
}
#else
void signal_handler(const int sig) {
    // Do nothing for now
}
#endif // #ifdef __APPLE__

} // namespace

void register_signal_handlers() {
    // Install signal handlers
    signal(SIGSEGV, signal_handler);
}

} // namespace nvl