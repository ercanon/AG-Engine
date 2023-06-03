// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.

#ifdef TEXTURED_GEOMETRY
	#if defined(VERTEX) ///////////////////////////////////////////////////
	
	layout(location = 0) in vec3 aPosition;
	layout(location = 1) in vec2 aTexCoord;
	
	out vec2 vTexCoord;
	
	void main()
	{
		vTexCoord = aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
		

	#elif defined(FRAGMENT) ///////////////////////////////////////////////

	in vec2 vTexCoord;
	
	uniform sampler2D uTexture;
	uniform bool isDepth;
	
	layout(location = 0) out vec4 oColor;
	
	void main()
	{
		if (isDepth)
		{
			float z = texture(uTexture, vTexCoord).r * 2.0 - 1.0;
			oColor = vec4(vec3((2.0 * 0.1 * 100.0) / (100.0 + 0.1 - z * (100.0 - 0.1)) / 100.0), 1);
		}
		else
			oColor = texture(uTexture, vTexCoord);
	}
	#endif
#endif



#ifdef TEXTURED_MESH
	struct Light
	{
		unsigned int type;
		vec3		 color;
		vec3		 direction;
		vec3		 position;
		vec3		 intensity;
	};

	#if defined(VERTEX) ///////////////////////////////////////////////////
	
	layout(location = 0) in vec3 aPosition;
	layout(location = 1) in vec3 aNormal;
	layout(location = 2) in vec2 aTexCoord;
	layout(location = 3) in vec3 aTangent;
	layout(location = 4) in vec3 aBitangent;

	layout (binding = 0, std140) uniform GlobalParams
	{
		vec3		 uCameraPosition;
		unsigned int uLightCount;
		Light		 uLight[16];
	};

	layout (binding = 1, std140) uniform LocalParams
	{
		mat4 uWorldMatrix;
		mat4 uWorldViewProjectionMatrix;
	};
	
	out vec2 vTexCoord;
	out vec3 vPosition;
	out vec3 vNormal;
	out vec3 vViewDir;

	void main()
	{
		vTexCoord = aTexCoord;

		vPosition = vec3( uWorldMatrix * vec4(aPosition, 1.0) );
		vNormal   = vec3( uWorldMatrix * vec4(aNormal, 0.0) );
		vViewDir = normalize(uCameraPosition - vPosition);
		gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
	}
	
	
	#elif defined(FRAGMENT) ///////////////////////////////////////////////
	
	in vec2 vTexCoord;
	in vec3 vPosition;
	in vec3 vNormal;
	in vec3 vViewDir;
	
	uniform sampler2D uTexture;

	layout (binding = 0, std140) uniform GlobalParams
	{
		vec3		 uCameraPosition;
		unsigned int uLightCount;
		Light		 uLight[16];
	};
	
	layout(location = 0) out vec4 oColor;
	layout(location = 1) out vec4 oNormals;
	layout(location = 2) out vec4 oPosition;
	
	void main()
	{
		vec3 lightStrenght = vec3(0.0);
		for(int i = 0; i< uLightCount; ++i)
		{
			if(uLight[i].type == 0)
			{
				vec3 ambient = uLight[i].intensity.x * uLight[i].color;
				
				float diff = max(dot(normalize(vNormal), normalize(uLight[i].direction)), 0.0);
				vec3 diffuse = diff * uLight[i].color * uLight[i].intensity.y;

				vec3 reflectDir = reflect(normalize(-uLight[i].direction), normalize(vNormal));
				float spec = pow(max(dot(normalize(vViewDir), reflectDir), 0.0), 32);
				vec3 specular = uLight[i].intensity.z * spec * uLight[i].color;

				lightStrenght += (ambient + diffuse + specular) * texture(uTexture, vTexCoord).rgb;
			}
			else if(uLight[i].type == 1)
			{
				vec3 lightDir = normalize(uLight[i].position - vPosition);

				vec3 ambient = uLight[i].intensity.x * uLight[i].color;

				float diff = max(dot(normalize(vNormal), lightDir), 0.0);
				vec3 diffuse = diff * uLight[i].color * uLight[i].intensity.y;

				float dist = length(uLight[i].position - vPosition);
				float attenuation = 1.0 /(pow(dist, 2));

				vec3 reflectDir = reflect(normalize(-lightDir), normalize(vNormal));
				float spec = pow(max(dot(normalize(vViewDir), reflectDir), 0.0), 32);
				vec3 specular = uLight[i].intensity.z * spec * uLight[i].color;

				diffuse *= attenuation * 2;
				lightStrenght += (ambient + diffuse + specular) * texture(uTexture, vTexCoord).rgb;
			}
		}
		
		oColor = vec4(lightStrenght, 1.0);
		oNormals = vec4(normalize(vNormal),1.0);
		oPosition = vec4(vPosition,1.0);
	}
	#endif
#endif



#ifdef BLIT
#version 330 core
	#if defined(VERTEX) ///////////////////////////////////////////////////
	
	layout(location = 0) in vec3 aPosition;
	layout(location = 1) in vec2 aTexCoord;
	
	out vec2 vTexCoord;
	
	void main()
	{
		vTexCoord = aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
		

	#elif defined(FRAGMENT) ///////////////////////////////////////////////
	
	uniform sampler2D uColorTexture;
	uniform float uThreshold;

	in vec2 vtexCoord;

	out vec4 oColor;
	
	void main()
	{
		vec4 texel = texture2D(uColorTexture, texCoord);
		float luminance = dot(vec3(0.2126, 0.7152, 0.0722), texel.rgb);

		luminance = max(0.0, luminance - uThreshold);
		texel.rgb *= sign(luminance);
		texel.a = 1.0;

		oColor = texel;
	}

	#endif
#endif

#ifdef BLUR
#version 330 core
	#if defined(VERTEX) ///////////////////////////////////////////////////
	
	layout(location = 0) in vec3 aPosition;
	layout(location = 1) in vec2 aTexCoord;
	
	out vec2 vTexCoord;
	
	void main()
	{
		vTexCoord = aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
		
	#elif defined(FRAGMENT) ///////////////////////////////////////////////

	uniform sampler2D uColorMap;
	uniform vec2 uDirection;
	uniform int uInputLod;

	in vec2 vtexCoord;

	out vec4 oColor;
	
	void main()
	{
		vec2 texSize = textureSize(uColorMap, uInputLod);
		vec2 texelSize = 1.0/texSize;
		vec2 marginl = texelSize * 0.5;
		vec2 margin2 = vec2(1.0) - margin1;

		oColor = vec4(0.0);

		veca directionFragCoord = gl_FragCoord.xy * uDirection;
		int coord = int(directionFragCoord.x + directionFragCoord.y);
		vec2 directionTexSize = texSize * uDirection;
		
		int size = int(directionTexSize.x + directionTexSize.y);
		int kernelRadius = 24;
		int kernelBegin = -min(kernelRadius, coord);
		int kernelEnd = min(kernelRadius, size - coord);
		float weight = 0.0;
		
		for (int i = kernelBegin; i <= kernelEnd; ++i)
		{
			float currentWeight = smoothstep(float(kernelRadius), 0.0, float(abs(i)));
			vec2 finalTexCoords = vtexCoord + i * uDirection * texelSize;
			finalTexCoords = clamp(finalTexCoords, marginl, margin2);
			oColor += texturelod(uColorMap, finalTexCoords, uInputLod) * currentWeight;
			weight += currentWeight;	
		}
		
		oColor /= weight;
	}
	#endif
#endif

#ifdef BLOOM
#version 330 core
	#if defined(VERTEX) ///////////////////////////////////////////////////
	
	layout(location = 0) in vec3 aPosition;
	layout(location = 1) in vec2 aTexCoord;
	
	out vec2 vTexCoord;
	
	void main()
	{
		vTexCoord = aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
		
	#elif defined(FRAGMENT) ///////////////////////////////////////////////

	uniform sampler2D uColorMap;
	uniform int uMaxLod;

	in vec2 vtexCoord;

	out vec4 oColor;
	
	void main()
	{
		oColor = vec4(0.0);
		for (int lod = 0; uMaxLod; ++lod)
			oColor += textureLod(uColorMap, vtexCoord, float(lod));

		oColor.a = 1.0;
	}
	#endif
#endif