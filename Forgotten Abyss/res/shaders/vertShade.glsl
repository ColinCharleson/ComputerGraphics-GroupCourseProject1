#version 410

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;

// Complete MVP
uniform mat4 u_ModelViewProjection;
// Just the model transform, we'll do worldspace lighting
uniform mat4 u_Model;
// Normal Matrix for transforming normals
uniform mat3 u_NormalMatrix;

uniform float t; // 3rd parameter of mix is t

void main() {

	gl_Position = u_ModelViewProjection * vec4(inPosition, 1.0); // instead of inpos, put in 2 positions and mix.

	// Lecture 5
	// Pass vertex pos in world space to frag shader
	outWorldPos = (u_Model * vec4(inPosition, 1.0)).xyz; // instead of inpos, put in 2 positions and mix.

	// Normals
	outNormal = u_NormalMatrix * inNormal; //replace inNormal, mix 2 normals:  mix(norm0,norm1,t)

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

}

