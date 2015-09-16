float4 VS(float4 position : POSITION) : SV_Position
{
    return position;
}

float4 PS(float4 position : SV_Position) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
