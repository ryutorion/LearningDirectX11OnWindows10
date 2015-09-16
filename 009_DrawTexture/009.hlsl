cbuffer ConstantBuffer
{
    float4 Color;
}

Texture2D myTex;

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
    uint width, height;
    myTex.GetDimensions(width, height);
    return myTex.Load(float3(input.texcoord.x * width, input.texcoord.y * height, 0.0f));
}
