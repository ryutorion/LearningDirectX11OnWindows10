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

inline float3 HalfVector(float3 V, float3 L)
{
    return normalize(V + L);
}

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

    float3 V = normalize(EyePosition - input.localPosition);
    float3 H = HalfVector(V, LightDir);
    float NH = dot(input.normal, H);
    float NV = dot(input.normal, V);
    float NL = dot(input.normal, LightDir);
    float VH = dot(V, H);
    float G = min(1, min(2 * NH * NV / VH, 2 * NH * NL / VH));
    float m = 0.15f;
    float m2 = m * m;
    float NH2 = NH * NH;
    float D = 1.0f / (m2 * NH2 * NH2) * exp(-(1 - NH2) / (m2 * NH2));
    float n = 20.0f;
    float g = sqrt(n * n * VH * VH - 1);
    float gpc = g + VH;
    float gmc = g - VH;
    float cgpcm1 = VH * gpc - 1;
    float cgmcp1 = VH * gmc + 1;
    float F = 0.5f * gmc * gmc / (gpc * gpc) * (1 + (cgpcm1 * cgpcm1) / (cgmcp1 * cgmcp1));
    float Rs = max(0.0f, F * D * G / (3.14159265f * NL * NV));
    float4 Ks = float4(2.0f * 0.486f, 2.0f * 0.433f, 2.0f * 0.185f, 1.0f);

    return ambient + diffuseColor + Ks * Rs;
}
