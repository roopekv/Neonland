Texture2D t1 : register(t0);
SamplerState s1: register(s0);

struct UIFragmentShaderInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 tint : TINT;
};

float4 main(UIFragmentShaderInput input) : SV_TARGET
{
    float4 baseColor = t1.Sample(s1, input.texCoord) * input.tint;

    //if (baseColor.a < 0.25f) {
    //    discard;
    //}

    return baseColor;
}