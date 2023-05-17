// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.


#ifdef TEXTURED_GEOMETRY
	#if defined(VERTEX) ///////////////////////////////////////////////////
	
	layout(location=0) in vec3 aPosition;
	layout(location=1) in vec2 aTexCoord;
	
	out vec2 vTexCoord;
	
	void main()
	{
		vTexCoord = aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
	
	
	#elif defined(FRAGMENT) ///////////////////////////////////////////////
	
	in vec2 vTexCoord;
	
	uniform sampler2D uTexture;
	
	layout(location=0) out vec4 oColor;
	
	void main()
	{
		oColor = texture(uTexture, vTexCoord);
	}
	#endif
#endif

#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;
layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewPorjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;

//uniform mat4 viewMatrix;
//uniform mat4 projection;
void main()
{
    vTexCoord = aTexCoord;

    //gl_Position = uWorldViewPorjectionMatrix * vec4(aPosition, 1);
    //gl_Position.z = -gl_Position.z;

    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));

    vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0));

    gl_Position = uWorldViewPorjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location=0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif