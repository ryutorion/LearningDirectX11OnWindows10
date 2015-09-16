cbuffer ConstantBuffer
{
    float4 Color;
}

Texture2D myTex;
SamplerState mySampler;

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

PSInput VS(float4 position : POSITION, float2 texcoord : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.texcoord = texcoord;

    return result;
}

float4 PS(PSInput input) : SV_Target
{
    return myTex.Sample(mySampler, input.texcoord);
}