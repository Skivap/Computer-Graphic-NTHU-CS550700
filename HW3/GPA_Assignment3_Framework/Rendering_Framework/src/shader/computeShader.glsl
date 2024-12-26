#version 450 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

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

struct InstanceProperties2 {
    vec4 position;
};

layout (std430, binding = 3) buffer DrawCommandsBlock {
    DrawCommand commands[];
};

layout (std430, binding = 1) buffer InstanceData {
    InstanceProperties rawInstanceProps[];
};

layout (std430, binding = 2) buffer CurrValidInstanceData {
    InstanceProperties2 currValidInstanceProps[];
};

uniform int numMaxInstance;
uniform mat4 viewProjMat;
uniform vec4 slimePos;

void main() {
    const uint idx = gl_GlobalInvocationID.x;

    if (idx >= numMaxInstance) {
        return;
    }

   float distanceToSlime = distance(rawInstanceProps[idx].position.xyz, slimePos.xyz);
    if (distanceToSlime < 2.0) {
        rawInstanceProps[idx].info.x = 1.0;
    }

    vec4 clipSpaceV = viewProjMat * vec4(rawInstanceProps[idx].position.xyz, 1.0) ;
    clipSpaceV = clipSpaceV / clipSpaceV.w ;
    // determine if it is culled
    bool frustumCulled = (clipSpaceV.x < -1.0) || (clipSpaceV.x > 1.0) || (clipSpaceV.y < -1.0) ||
    (clipSpaceV.y > 1.0) || (clipSpaceV.z < -1.0) || (clipSpaceV.z > 1.0) ;

    if (frustumCulled == false && rawInstanceProps[idx].info.x == 0.0f) {
        uint mmidx = uint(rawInstanceProps[idx].position.w);
        uint UNIQUE_IDX = atomicAdd(commands[mmidx].instanceCount, 1);
        uint offset = commands[mmidx].baseInstance;
        currValidInstanceProps[offset + UNIQUE_IDX].position = rawInstanceProps[idx].position;
    }
}
