cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    matrix WorldView;
    matrix WorldViewProj;
    matrix InverseWorld;
    matrix InverseView;
    matrix InverseProjection;
}

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(WorldView, float4(input.position, 1.0f));
    return output;
}

struct GS_OUTPUT
{
    float4 position : SV_Position;
};

[maxvertexcount(4)]
void GSMain(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> OutputStream)
{
    float4 inputPos = input[0].position;
    float halfLength = 0.05f;
    GS_OUTPUT v;
    
    v.position = mul(Projection, inputPos + float4(-halfLength, -halfLength, 0.0f, 0.0f));
    OutputStream.Append(v);

    v.position = mul(Projection, inputPos + float4(-halfLength, halfLength, 0.0f, 0.0f));
    OutputStream.Append(v);
    
    v.position = mul(Projection, inputPos + float4(halfLength, -halfLength, 0.0f, 0.0f));
    OutputStream.Append(v);
    
    v.position = mul(Projection, inputPos + float4(halfLength, halfLength, 0.0f, 0.0f));
    OutputStream.Append(v);
}


struct PS_OUTPUT
{
    float4 color : SV_Target;
};

PS_OUTPUT PSMain(VS_OUTPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    output.color = float4(1.0f, 0.0f, 0.4f, 1.0f);
    return output;
}