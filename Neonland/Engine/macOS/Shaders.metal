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
    float3 viewPosition;
    float3 normal;
    float2 texCoords;
    float4 tint;
};

vertex auto vertexFunc(Vertex in [[stage_in]],
                       uint instanceId [[instance_id]],
                       constant Instance* instances [[buffer(1)]],
                       constant GlobalUniforms& sceneData [[buffer(2)]]) -> FragmentData {
    Instance instance = instances[instanceId];
    
    float4 viewPos = sceneData.viewMatrix * instance.transform * float4(in.position, 1);
    float4 viewNormal = sceneData.viewMatrix * instance.transform * float4(in.normal, 0);
    
    FragmentData out;
    out.position = sceneData.projMatrix * viewPos;
    out.viewPosition = viewPos.xyz;
    out.normal = viewNormal.xyz;
    
    out.texCoords = in.texCoords;
    out.tint = instance.color;
    
    return out;
}

fragment auto fragmentFunc(FragmentData in [[stage_in]],
                           texture2d<float, access::sample> texMap [[texture(0)]],
                           sampler texSampler [[sampler(0)]],
                           constant Instance* instances [[buffer(1)]]) -> float4 {
    
    float4 baseColor = texMap.sample(texSampler, in.texCoords) * in.tint;
    if (baseColor.a < 0.1f) {
        discard_fragment();
    }
    
    float3 normal = normalize(in.normal);
    float3 view = normalize(-in.viewPosition);
    
    float3 light(0, 0, -1);
    float3 halfway = normalize(light + view);
    
    float diffuse = saturate(dot(normal, light));
    float specular = powr(saturate(dot(normal, halfway)), 50.0f);
    
    float3 litColor = (0.1f + diffuse + specular) * 1.0f * baseColor.rgb;
    
    return float4((litColor * baseColor.a), baseColor.a);
}
