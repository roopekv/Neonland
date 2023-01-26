struct FragmentShaderInput
{
	float4 position : SV_POSITION;
	float3 color : COLOR0;
};

float4 main(FragmentShaderInput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}
