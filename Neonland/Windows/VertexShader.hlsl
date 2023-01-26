cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	matrix instanceTf : INSTANCETF;
	float4 instanceColor : INSTANCECOLOR;
};

struct FragmentShaderInput
{
	float4 position : SV_POSITION;
	float3 color : COLOR0;
};

FragmentShaderInput main(VertexShaderInput input)
{
	float4 pos = float4(input.position, 1.0f);

	pos = mul(pos, input.instanceTf);
	pos = mul(pos, viewMatrix);
	pos = mul(pos, projectionMatrix);

	FragmentShaderInput output;
	output.position = pos;
	output.color = input.instanceColor;

	output.color = float4(1, 1, 1, 1);

	return output;
}
