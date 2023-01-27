cbuffer SceneData : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4x4 instanceTf : INSTANCETF;
	float4 instanceColor : INSTANCECOLOR;
};

struct FragmentShaderInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 viewPosition : VIEWPOS;
	float3 normal : NORMAL;
	float4 tint : TINT;
};

FragmentShaderInput main(VertexShaderInput input)
{
	float4 viewPos = float4(input.position, 1);
	viewPos = mul(viewPos, input.instanceTf);
	viewPos = mul(viewPos, viewMatrix);

	float4 viewNormal = float4(input.normal, 0);
	viewNormal = mul(viewNormal, input.instanceTf);
	viewNormal = mul(viewNormal, viewMatrix);

	FragmentShaderInput output;
	output.position = mul(viewPos, projMatrix);
	output.viewPosition = viewPos.xyz;
	output.normal = viewNormal.xyz;

	output.texCoord = input.texCoord;
	output.tint = input.instanceColor;

	return output;
}
