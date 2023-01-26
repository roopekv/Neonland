struct FragmentShaderInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

float4 main(FragmentShaderInput input) : SV_TARGET
{
	return input.color;
}
