#include "cpu/pipeline.hpp"
#include <cstring>

pipeline::pipeline() {
    reset();
}

void pipeline::reset() {
    std::memset(&regs[0], 0, sizeof(PipelineRegs));
    std::memset(&regs[1], 0, sizeof(PipelineRegs));
    idx = 0;
}
