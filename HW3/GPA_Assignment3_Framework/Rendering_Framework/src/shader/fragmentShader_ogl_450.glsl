#version 450 core

in vec3 f_worldVertex;
in vec3 f_viewVertex;
in vec3 textureCoord;
in vec3 normalVertex;

layout (location = 0) out vec4 fragColor ;
layout (location = 5) uniform int shadingModelId;

uniform sampler2DArray texture2D;
uniform sampler2D textureSlime;

vec4 WithFog(vec4 color){
	const vec4 FOG_COLOR = vec4(0.0, 0.0, 0.0, 1) ;
	const float MAX_DIST = 150.0 ;
	const float MIN_DIST = 120.0 ;
	
	float dis = length(f_viewVertex) ;
	float fogFactor = (MAX_DIST - dis) / (MAX_DIST - MIN_DIST) ;
	fogFactor = clamp(fogFactor, 0.0f, 1.0f) ;
	fogFactor = fogFactor * fogFactor ;
	
	return mix(FOG_COLOR, color, fogFactor) ;
}

float GridFactor(vec3 worldPos){
	const vec2 HALF_LINE_WIDTH = vec2(0.05f, 0.05f);
	// make it repeat
	vec2 fr = fract(worldPos.xz);
	// [0.0, 1.0) -> [-1.0, 1.0)
	fr = 2.0f * fr - 1.0f;
	// fwidth = abs(dFdx) + abs(dFdy)
	vec2 uvDeriv = fwidth(worldPos.xz);
	const vec2 lineAA = uvDeriv * 1.5f;
	// (approximate) constant pixel width
	vec2 finalHalfLineWidth = HALF_LINE_WIDTH * lineAA;
	// prevent from too thickness
	finalHalfLineWidth = max(HALF_LINE_WIDTH, finalHalfLineWidth);
	
	const vec2 SX = vec2(0.0f, 0.0f);	
	const vec2 s = smoothstep(SX - finalHalfLineWidth - lineAA, SX - finalHalfLineWidth,          fr);	
	const vec2 m = smoothstep(SX + finalHalfLineWidth,          SX + finalHalfLineWidth + lineAA, fr);	
	const vec2 res = s - m;
	// fade out the line based on the thickness
	const vec2 fadeOutRes = res * clamp(HALF_LINE_WIDTH / finalHalfLineWidth, vec2(0.0f), vec2(1.0f));	
	// for the area that is usually has Moire patterns, fade out it
	const vec2 deriv2 = clamp(uvDeriv, 0.0f, 1.0f);
	const vec2 finalRes = mix(fadeOutRes, HALF_LINE_WIDTH, deriv2);
	
	return mix(finalRes.x, 1.0, finalRes.y);
}
	
void RenderGrid(){
	const vec4 groundColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const vec4 gridColor = vec4(0.7f, 0.7f, 0.7f, 1.0f);
	
	vec4 color = mix(groundColor, gridColor, GridFactor(f_worldVertex));
	fragColor = WithFog(color);
}

void RenderViewFrustum(){
	fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void RenderMesh(){

    vec4 texColor = texture(texture2D, textureCoord);
    if(texColor.a < 0.5){
        discard;
    }

    vec3 N = normalize(normalVertex);
    vec3 L = normalize(vec3(0.3, 0.7, 0.5)); // light dir
    vec3 V = normalize(-f_viewVertex); // view dir
    vec3 H = normalize(L + V); // half

    // Material coefficients
    vec3 Ka = vec3(0.1, 0.1, 0.1); // ambient
    vec3 Kd = vec3(0.8, 0.8, 0.8); // diffuse
    vec3 Ks = vec3(0.1, 0.1, 0.1); // specular
    float specular_power = 32.0;

    vec3 ambient = Ka * texColor.rgb;
    vec3 diffuse = Kd * texColor.rgb * max(dot(N, L), 0.0);
    vec3 specular = Ks * pow(max(dot(N, H), 0.0), specular_power);

    vec4 final = vec4(ambient + diffuse + specular, 1.0);
    fragColor = vec4(1.0) - exp(-final * 3.0) ;
}

void RenderSlime(){
    vec4 texColor = texture(textureSlime, textureCoord.xy);

	vec3 N = normalize(normalVertex);
    vec3 L = normalize(vec3(0.3, 0.7, 0.5)); // light dir
    vec3 V = normalize(-f_viewVertex); // view dir
    vec3 H = normalize(L + V); // half

    // Material coefficients
    vec3 Ka = vec3(0.1, 0.1, 0.1); // ambient
    vec3 Kd = vec3(0.8, 0.8, 0.8); // diffuse
    vec3 Ks = vec3(0.1, 0.1, 0.1); // specular
    float specular_power = 32.0;

    vec3 ambient = Ka * texColor.rgb;
    vec3 diffuse = Kd * texColor.rgb * max(dot(N, L), 0.0);
    vec3 specular = Ks * pow(max(dot(N, H), 0.0), specular_power);

    vec4 final = vec4(ambient + diffuse + specular, 1.0);
    fragColor = vec4(1.0) - exp(-final * 3.0) ;
}

void main(){	
	if(shadingModelId == 5){
		RenderViewFrustum();
	}
	else if(shadingModelId == 1){
		RenderMesh();
	}
	else if(shadingModelId == 2){
		RenderSlime();
	}
	else{
		RenderGrid();
	}	
}