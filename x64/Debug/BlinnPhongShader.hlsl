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

struct PointLight
{
    float3 position;
    float3 color;
    float power;
};

StructuredBuffer<PointLight> lights : register(t0);

static const int MAX_NUM_OF_LIGHTS = 5;

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
    float4 viewPos : VIEWPOSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(WorldViewProj, float4(input.position, 1.0f));
    output.viewPos = mul(WorldView, float4(input.position, 1.0f));
    output.normal = normalize(mul((float3x3) WorldView, input.normal));
    output.color = input.color;
    return output;
}



struct PS_OUTPUT
{
    float4 color : SV_Target;
};


float3 BlinnPhong(float3 vertPos, float3 diffuseColor, float3 normal)
{
    float shininess = 20.0f;
    float screenGamma = 2.2f;
    float3 ambientColor = float3(0.0f, 0.02f, 0.05f);
    float3 linearColor;
    
    for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++)
    {
        if(lights[i].power == 0.0f)
            continue;
        
        float4 lightPosView = mul(View, float4(lights[i].position, 1.0f));
        lightPosView /= lightPosView.w;
        float3 lightDir = lightPosView.xyz - vertPos;
        float distance = length(lightDir);
        distance *= distance;
        lightDir = normalize(lightDir);
    
        float lambertian = max(dot(lightDir, normal), 0.0f);
        float specular = 0.0f;
        float3 viewDir;
        if (lambertian > 0.0f)
        {
            viewDir = normalize(-vertPos);
        
            float3 halfDir = normalize(lightDir + viewDir);
            float specAngle = max(dot(halfDir, normal), 0.0f);
            specular = pow(specAngle, shininess);
        }
        
        linearColor += diffuseColor * lambertian * lights[i].color * lights[i].power / distance +
                         specular * lights[i].color * lights[i].power / distance;
        
    }
    float3 gammaCorrectedColor = pow(linearColor, 1.0f / screenGamma);
    gammaCorrectedColor += ambientColor;
    return gammaCorrectedColor;
}

PS_OUTPUT PSMain(VS_OUTPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    float3 color = BlinnPhong(input.viewPos.xyz, input.color, input.normal);
    output.color = float4(color, 1.0f);
    return output;
}