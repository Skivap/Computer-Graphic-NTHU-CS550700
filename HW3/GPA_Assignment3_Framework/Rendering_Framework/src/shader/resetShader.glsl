#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

struct InstanceProperties {
    vec4 position;
    vec4 info;
};


layout (std430, binding = 1) buffer InstanceData {
    InstanceProperties rawInstanceProps[];
};

layout (std430, binding = 3) buffer DrawCommandsBlock {
    DrawCommand commands[];
};

void main() {
    // Using gl_GlobalInvocationID.x to directly access the correct command based on the dispatch call
    uint idx = gl_GlobalInvocationID.x;
    commands[0].instanceCount = 0;
    commands[1].instanceCount = 0;
    commands[2].instanceCount = 0;
}
