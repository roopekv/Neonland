#include <metal_stdlib>
#include "../ShaderTypes.h"
using namespace metal;

struct Vertex {
    float3 position  [[attribute(0)]];
    float3 normal    [[attribute(1)]];
    float2 texCoords [[attribute(2)]];
};

struct FragmentData {
    float4 position [[position]];
    float3 normal;
    float2 texCoords;
};

vertex auto vertexFunc(Vertex in [[stage_in]],
                       uint instanceId [[instance_id]],
                       constant Instance* instances [[buffer(1)]],
                       constant GlobalUniforms& sceneData [[buffer(2)]]) -> FragmentData {
    Instance instance = instances[instanceId];
    FragmentData out;
    out.position = sceneData.projMatrix * sceneData.viewMatrix * instance.transform * float4(in.position, 1);
    out.normal = in.normal;
    out.texCoords = in.texCoords;
    
    return out;
}

fragment auto fragmentFunc(FragmentData in [[stage_in]],
                           texture2d<float, access::sample> texMap [[texture(0)]],
                           sampler texSampler [[sampler(0)]]) -> float4 {
    
    float4 out;
    if (is_null_texture(texMap)) {
        float3 light = normalize(float3(1, 1, 1));
        float3 normal = normalize(in.normal);
        
        float color = saturate(dot(light, normal)) * 0.1f;
        out = float4(float3(color), 1);
    }
    else {
        out = texMap.sample(texSampler, in.texCoords);
    }

    if (out.a < 0.1) {
        discard_fragment();
    }
    
    return out;
}
