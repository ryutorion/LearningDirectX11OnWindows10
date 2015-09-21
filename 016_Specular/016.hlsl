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
    float3   AmbientColor;
    float3   EyePosition;
}

Texture2D myTex;
SamplerState mySampler;

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 localPosition : POSITION;
};

PSInput VS(float4 position : POSITION, float3 normal : NORMAL)
{
    PSInput result;

    result.position = mul(Projection, mul(View, mul(Model, position)));
    result.normal = normal;
    result.localPosition = position.xyz;

    return result;
}

float4 PS(PSInput input) : SV_Target
{
    float4 ambient = float4(AmbientColor, 0.0);

    float diffuse = max(0, dot(input.normal, LightDir));
    float4 diffuseColor = float4(LightColor, 1.0) * float4(diffuse, diffuse, diffuse, 1.0);

    // float3 R = 2.0 * dot(LightDir, input.normal) * input.normal - LightDir;
    float3 R = reflect(-LightDir, input.normal);
    float specular = pow(max(0, dot(R, normalize(EyePosition - input.localPosition))),5);

    return float4(AmbientColor, 0.0) + diffuseColor + specular;
}
