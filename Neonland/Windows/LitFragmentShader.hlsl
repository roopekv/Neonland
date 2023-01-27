Texture2D t1 : register(t0);
SamplerState s1: register(s0);

struct FragmentShaderInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 viewPosition : VIEWPOS;
	float3 normal : NORMAL;
	float4 tint : TINT;
};

float4 main(FragmentShaderInput input) : SV_TARGET
{
    float4 baseColor = t1.Sample(s1, input.texCoord) * input.tint;

    float3 normal = normalize(input.normal);
    float3 view = normalize(-input.viewPosition);

    float3 light = { 0, 0.5, -0.5 };
    float3 halfway = normalize(light + view);

    float diffuse = saturate(dot(normal, light));
    float specular = pow(saturate(dot(normal, halfway)), 50.0f);

    float3 litColor = (1.0f + diffuse + specular) * 0.5f * baseColor.rgb;

    return float4((litColor * baseColor.a), baseColor.a);
}