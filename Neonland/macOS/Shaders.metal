#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float2 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
};

struct Uniforms {
    float2 offset;
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
};

vertex auto vertexFunc(VertexIn in [[stage_in]],
                       constant Uniforms& uniforms [[buffer(1)]]) -> VertexOut {
    VertexOut out;
    out.position = float4(in.position + uniforms.offset, 0, 1);
    out.color = in.color;
    return out;
}

fragment auto fragmentFunc(VertexOut in [[stage_in]]) -> float4 {
    return in.color;
}
