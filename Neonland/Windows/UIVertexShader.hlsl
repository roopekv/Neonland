cbuffer SceneData : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
};

struct UIVertexShaderInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4x4 instanceTf : INSTANCETF;
	float4 instanceColor : INSTANCECOLOR;
};

struct UIFragmentShaderInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 tint : TINT;
};

UIFragmentShaderInput main(UIVertexShaderInput input)
{
	UIFragmentShaderInput output;
	float4 pos = float4(input.position, 1);
	pos = mul(pos, input.instanceTf);
	pos = mul(pos, viewMatrix);
	pos = mul(pos, projMatrix);
	pos.z = 0.1f;
	output.position = pos;
	output.texCoord = input.texCoord;
	output.tint = input.instanceColor;

	return output;
}
