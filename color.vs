////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
	float4 uv	: TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
	float4 uv	: TEXCOORD;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
    
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = float4( input.position, 1.0f );// mul(float4( input.position, 1.0f ), worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    //output.position = mul(output.position, projectionMatrix);
    
	// Store the input color for the pixel shader to use.
    output.color = input.color;
    
	output.uv = input.uv;

    return output;
}