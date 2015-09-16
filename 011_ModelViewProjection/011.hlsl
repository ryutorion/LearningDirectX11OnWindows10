cbuffer ConstantBuffer
{
    float4x4 Model;
    float4x4 View;
    float4x4 Projection;
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

    result.position = mul(Projection, mul(View, mul(Model, position)));
    result.texcoord = texcoord;

    return result;
}

float4 PS(PSInput input) : SV_Target
{
    return myTex.Sample(mySampler, input.texcoord);
}