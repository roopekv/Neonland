Texture2D t1 : register(t0);
SamplerState s1: register(s0);

struct FragmentShaderInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float4 main(FragmentShaderInput input) : SV_TARGET
{
	return t1.Sample(s1, input.texCoord);
}