#include <metal_stdlib>
#include "../ShaderTypes.h"
using namespace metal;

struct Vertex {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
};

struct FragmentData {
    float4 position [[position]];
    float3 normal;
};

vertex auto vertexFunc(Vertex in [[stage_in]],
                       uint instanceId [[instance_id]],
                       constant Instance* instances [[buffer(1)]],
                       constant GlobalUniforms& sceneData [[buffer(2)]]) -> FragmentData {
    Instance instance = instances[instanceId];
    FragmentData out;
    out.position = sceneData.projMatrix * sceneData.viewMatrix * instance.transform * float4(in.position, 1);
    out.normal = in.normal;
    
    return out;
}

fragment auto fragmentFunc(FragmentData in [[stage_in]]) -> float4 {
    float3 normal = normalize(in.normal);
    float3 color = normal * float3(0.5) + float3(0.5);
    return float4(color, 1);
}
