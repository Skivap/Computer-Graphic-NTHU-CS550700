#version 450 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 texCoord;
layout(location=2) in vec3 v_normal;
layout(location=3) in vec4 v_worldPosOffset;

out vec3 f_worldVertex;
out vec3 f_viewVertex;
out vec3 textureCoord;
out vec3 normalVertex;

layout(location = 0) uniform mat4 modelMat ;
layout(location = 1) uniform mat4 viewMat ;
layout(location = 2) uniform mat4 projMat ;

uniform vec4 slimePos;

void main(){
	vec4 worldVertex = modelMat * vec4(v_vertex, 1.0) + v_worldPosOffset + slimePos;// vec4(v_worldPosOffset.xyz, 0.0f);
	worldVertex.w = 1.0;

	vec4 viewVertex = viewMat * worldVertex;
	
	f_worldVertex = worldVertex.xyz;
	f_viewVertex = viewVertex.xyz;
	textureCoord = texCoord;

	normalVertex = v_normal;
	
	gl_Position = projMat * viewVertex;
}