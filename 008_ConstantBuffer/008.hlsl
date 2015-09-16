cbuffer ConstantBuffer
{
    float4 Color;
}

float4 VS(float4 position : POSITION) : SV_Position
{
    return position;
}

float4 PS(float4 position : SV_Position) : SV_Target
{
    return Color;
}
