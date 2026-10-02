#include <jit++/common.h>
#include <jit++/interpreting/impl.h>
#include <cstdio>
#include <cstdlib>

DECLARE_uint64(jitpp_steps);

extern "C" uint64_t roundtrip(jitpp::tracer *, bool *);
extern "C" char roundtrip_stopped[], roundtrip_unsupported[];
extern "C" char roundtrip_step_limit[];

class observed_interpreter : public jitpp::interpreter_impl {
public:
    unsigned entries = 0;
    int64_t exit_rip = 0, exit_rax = 0;
    void run() override {
        ++entries;
        interpreter_impl::run();
        exit_rip = rip();
        exit_rax = rax();
    }
};

static void require(bool condition, const char * message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main(int argc, char ** argv) {
    jitpp::application app(argc, argv);
    observed_interpreter interpreter;
    for (unsigned iteration = 0; iteration != 2; ++iteration) {
        FLAGS_jitpp_steps = -1;
        uint64_t flags = roundtrip(&interpreter, &interpreter.m_stopped);
        require(interpreter.entries == iteration + 1, "interpreter entry count");
        require(interpreter.exit_rip == reinterpret_cast<int64_t>(roundtrip_stopped),
                "arithmetic and stop must execute under interpretation");
        require(interpreter.exit_rax == 0, "interpreted addition result");
        require((flags & 0x8d5) == 0x55, "arithmetic flags survive native resumption");
    }
    // Stop after the addition, before the explicit stop store.
    FLAGS_jitpp_steps = 2;
    uint64_t flags = roundtrip(&interpreter, &interpreter.m_stopped);
    require(interpreter.exit_rip == reinterpret_cast<int64_t>(roundtrip_step_limit),
            "step limit resumes immediately after the second instruction");
    require(interpreter.exit_rax == 0, "step-limited interpreted addition result");
    require((flags & 0x8d5) == 0x55, "step-limit exit preserves flags");

    // With a separate stop flag, CPUID is the first unsupported instruction.
    bool ignored_stop = false;
    FLAGS_jitpp_steps = -1;
    roundtrip(&interpreter, &ignored_stop);
    require(interpreter.exit_rip == reinterpret_cast<int64_t>(roundtrip_unsupported),
            "unsupported instruction resumes at its original address");
    require(ignored_stop, "interpreted store reaches native memory");
    std::puts("PASS: interpreted arithmetic, flags, repeated entry, step limit, native fallback");
}
