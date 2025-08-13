struct SpriteData
{
	float2x2 Transform;
	float2 Position;
    float2 Pivot;
    float4 Color;
    float TexU, TexV, TexW, TexH;
};

struct Output
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Position : SV_Position;
};

StructuredBuffer<SpriteData> DataBuffer : register(t0, space0);

cbuffer UniformBlock : register(b0, space1)
{
    float4x4 ViewProjectionMatrix : packoffset(c0);
};

static const uint triangleIndices[6] = {0, 1, 2, 3, 2, 1};
static const float2 vertexPos[4] = {
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f}
};

float4 UintToColor(dword color) {
	float s = 1.0f / 255.0f;
    return float4(
    	((color >>  0) & 0xff) * s,
    	((color >>  8) & 0xff) * s,
    	((color >> 16) & 0xff) * s,
    	((color >> 24) & 0xff) * s);
}

Output main(uint id : SV_VertexID)
{
    uint spriteIndex = id / 6;
    uint vert = triangleIndices[id % 6];
    SpriteData sprite = DataBuffer[spriteIndex];

    float2 texcoord[4] = {
        { sprite.TexU,               sprite.TexV               },
        { sprite.TexU + sprite.TexW, sprite.TexV               },
        { sprite.TexU,               sprite.TexV + sprite.TexH },
        { sprite.TexU + sprite.TexW, sprite.TexV + sprite.TexH }
    };

    float2 coord = vertexPos[vert] - sprite.Pivot;
    
    coord = mul(coord, sprite.Transform) + sprite.Position;
    
    Output output;

    output.Position = mul(ViewProjectionMatrix, float4(coord, 0.0f, 1.0f));
    output.TexCoord = texcoord[vert];
    output.Color = sprite.Color;
 
    return output;
}
