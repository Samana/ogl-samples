#version 420 core

#define POSITION		0
#define COLOR			3
#define FRAG_COLOR		0

uniform mat4 MVP;

struct vertex
{
	vec2 Position[2]
};

layout(location = POSITION) in vec2 vertex Vertex;
layout(location = COLOR) in vec4 Color;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout(location = 0) out vertex st_Out;

out block
{
	vec4 Color;
} bl_Out; 

void main()
{	
	gl_Position = MVP * vec4(Vertex.Position[0] + Vertex.Position[1], 0.0, 1.0);
	st_Out.Color = Color * 0.75;
	bl_Out.Color = Color * 0.25;
}