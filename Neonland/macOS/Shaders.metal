#include <metal_stdlib>
#include "../ShaderUniforms.h"
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 normal;
};

vertex auto vertexFunc(VertexIn in [[stage_in]],
                       constant VertexUniforms& uniforms [[buffer(1)]]) -> VertexOut {
    VertexOut out;
    out.position = uniforms.projection * float4(in.position + uniforms.offset, 1);
    out.normal = in.normal;
    return out;
}

fragment auto fragmentFunc(VertexOut in [[stage_in]]) -> float4 {
    float3 normal = normalize(in.normal);
    float3 color = normal * float3(0.5) + float3(0.5);
    return float4(color, 1);
}
