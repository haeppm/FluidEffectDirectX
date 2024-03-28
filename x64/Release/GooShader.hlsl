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
    matrix InverseViewport;
}

#define MAX_NEARBY_PARTICLES 32
#define k 3
#define iso 0.5

cbuffer NearbyParticleBuffer : register(b1)
{
    float4 particlePos[MAX_NEARBY_PARTICLES];
};

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
    float3 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(WorldView, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}

struct GS_OUTPUT
{
    float4 position : SV_Position;
    float4 viewPos : VIEWPOSITION;
    float3 color : COLOR;
    float halfLength : HALFLENGTH;
};

[maxvertexcount(4)]
void GSMain(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> OutputStream)
{
    float4 inputPos = input[0].position;
    float halfLength = 1.0f;
    GS_OUTPUT v;
    v.color = input[0].color;
    v.halfLength = halfLength;
    
    v.viewPos = inputPos + float4(-halfLength, -halfLength, 0.0f, 0.0f);
    v.position = mul(Projection, v.viewPos);
    OutputStream.Append(v);
    
    v.viewPos = inputPos + float4(-halfLength, halfLength, 0.0f, 0.0f);
    v.position = mul(Projection, v.viewPos);
    OutputStream.Append(v);
    
    v.viewPos = inputPos + float4(halfLength, -halfLength, 0.0f, 0.0f);
    v.position = mul(Projection, v.viewPos);
    OutputStream.Append(v);
    
    v.viewPos = inputPos + float4(halfLength, halfLength, 0.0f, 0.0f);
    v.position = mul(Projection, v.viewPos);
    OutputStream.Append(v);
}

float GetScalarValue(float3 x)
{
    float s = 0;
    
    for (int i = 0; i < MAX_NEARBY_PARTICLES; i++)
    {
        //The w component of particlePos is only 0 when no nearby Particle was found in Particle::UpdateNearbyParticles
        if (particlePos[i].w == 0.0f)
            continue;
        
        float eucDist = distance(x, particlePos[i].xyz);
        float si = exp(-k * eucDist);
        s += si;
    }
    return s;
}

float3 GetNormal(float3 hitPoint)
{
    float3 normal;
    float e = 0.001f;
    
    normal.x = GetScalarValue(float3(hitPoint.x + e, hitPoint.yz)) - GetScalarValue(float3(hitPoint.x - e, hitPoint.yz));
    normal.y = GetScalarValue(float3(hitPoint.x, hitPoint.y + e, hitPoint.z)) - GetScalarValue(float3(hitPoint.x, hitPoint.y - e, hitPoint.z));
    normal.z = GetScalarValue(float3(hitPoint.xy, hitPoint.z + e)) - GetScalarValue(float3(hitPoint.xy, hitPoint.z - e));
    normal /= 2 * e;
    normal = -normalize(normal);
    
    return normal;
}

float3 BlinnPhong(float3 vertPos, float3 diffuseColor, float3 normal)
{
    float shininess = 500.0f;
    float screenGamma = 2.2f;
    float3 ambientColor = float3(0.0f, 0.02f, 0.05f);
    float3 linearColor;
    
    for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++)
    {
        if (lights[i].power == 0.0f)
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

struct PS_OUTPUT
{
    float4 color : SV_Target;
};

PS_OUTPUT PSMain(GS_OUTPUT input)
{
    PS_OUTPUT output;
    float3 rayDirection = normalize(input.viewPos.xyz);
    float rayLength = length(input.viewPos.xyz) - input.halfLength * 2;
    float stepSize = 0.1f;
    float3 ray = rayDirection * rayLength;
    float scalarValue = GetScalarValue(ray);
    float diff = abs(scalarValue - iso);
    float threshholdDiff = 0.01f;
    int iterations = 0;
    
    while (diff > threshholdDiff && iterations < 40)
    {
        if (scalarValue > iso)
        {
            stepSize /= 2.0f;
            rayLength -= stepSize;
        }
        else
        {
            rayLength += stepSize;
        }
        ray = rayDirection * rayLength;
        scalarValue = GetScalarValue(ray);
        diff = abs(scalarValue - iso);
        iterations++;
    }
    
    if(diff > threshholdDiff)
    {
        discard;
    }
    
    float3 normal = GetNormal(ray);
    
    float3 color = BlinnPhong(ray, input.color, normal);
    
    output.color = float4(color, 1.0f);
    return output;
}

