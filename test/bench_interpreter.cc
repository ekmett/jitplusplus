import jitpp;
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>

// Isolate dispatch and state access from pthread creation and native entry/exit.
int main(int argc, char ** argv) {
    jitpp::application app(argc, argv);
    const unsigned char loop[] = {
        0x48, 0x83, 0xc0, 0x01, // add $1, %rax
        0x48, 0xff, 0xc9,       // dec %rcx
        0x75, 0xf7,             // jnz back to add
        0xc6, 0x07, 0x01        // movb $1, (%rdi): stop
    };
    const int64_t iterations = 1000000;
    double samples[9];
    jitpp::interpreter interpreter;
    for (double & elapsed : samples) {
        interpreter.rax() = 0;
        interpreter.rcx() = iterations;
        interpreter.rdi() = reinterpret_cast<int64_t>(&interpreter.m_stopped);
        interpreter.rip() = reinterpret_cast<int64_t>(loop);
        interpreter.rflags(2);
        auto start = std::chrono::steady_clock::now();
        interpreter.run();
        elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        if (interpreter.rax() != iterations || interpreter.rcx() != 0 ||
            interpreter.rip() != reinterpret_cast<int64_t>(loop + sizeof(loop))) {
            std::fputs("FAIL: interpreter loop result\n", stderr);
            return 1;
        }
    }
    std::sort(samples, samples + 9);
    std::printf("interpreter bytes: %zu; median: %.6f s; %.2f million instructions/s\n",
                sizeof(interpreter), samples[4], (3 * iterations + 1) / samples[4] / 1e6);
}
