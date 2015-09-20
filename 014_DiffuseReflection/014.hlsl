cbuffer ConstantBuffer : register(b0)
{
    float4x4 Model;
    float4x4 View;
    float4x4 Projection;
}

cbuffer LightBuffer : register(b1)
{
    float3   LightDir;
    float3   LightColor;
}

Texture2D myTex;
SamplerState mySampler;

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

PSInput VS(float4 position : POSITION, float3 normal : NORMAL)
{
    PSInput result;

    result.position = mul(Projection, mul(View, mul(Model, position)));
    result.normal = normal;

    return result;
}

float4 PS(PSInput input) : SV_Target
{
    float diffuse = max(0, dot(input.normal, LightDir));

    return float4(LightColor, 1.0) * float4(diffuse, diffuse, diffuse, 1.0);
}
