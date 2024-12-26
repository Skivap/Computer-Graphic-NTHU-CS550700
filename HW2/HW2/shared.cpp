#include "shared.hpp"

/* Variable */
GLFWwindow* window;
std::pair<double, double> mouseCursor;

const float MAX_FPS = 1.0f / 60.0f;
const int SCREEN_HEIGHT = 800;
const int SCREEN_WIDTH = 800;
bool onFocus = false;

/* Vertex Shader */

const char* vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec3 iv3vertex;
layout (location = 1) in vec2 iv2tex_coord;
layout (location = 2) in vec3 iv3normal;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform mat4 proj;
uniform mat4 view;


void main()
{
    gl_Position = proj * view * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
    vertexData.N = iv3normal;
}
)";

/* Fragment Shader */

const char* fragmentShaderSource = R"(
#version 450 core

layout(location = 0) out vec4 outColor;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D diffuseMap;
uniform sampler2D alphaMap;
uniform sampler2D emissiveMap;

uniform bool hasDiffuseMap;
uniform bool hasAlphaMap;
uniform bool hasEmissiveMap;

uniform bool drawNormal;

void main(){
    if(drawNormal){
        outColor = vec4(vertexData.N, 1.0f);
        return;
    }

     vec4 texColor = vec4(1.0f);

    if(hasDiffuseMap){
        texColor = texture(diffuseMap, vertexData.texcoord);
    }

    if (hasAlphaMap) {
        vec4 alpha = texture(alphaMap, vertexData.texcoord);
        texColor.a = alpha.r;
        if(texColor.a < 0.1f){
            discard;
        }
    }

    if (hasEmissiveMap) {
        vec4 emissive = texture(emissiveMap, vertexData.texcoord);
        texColor.rgb += emissive.rgb;
    }

    outColor = texColor;
}
)";

const char* frameBufferVS = R"(
#version 450 core

layout (location = 0) in vec2 iv2vertex;
layout (location = 1) in vec2 iv2tex_coord;

out vec2 texcoord;

void main()
{
    gl_Position = vec4(iv2vertex.x, iv2vertex.y, 0.0, 1.0); 
    texcoord = iv2tex_coord;
}  
)";

const char* frameBufferFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform sampler2D screenTexture;

void main()
{
    outColor = texture(screenTexture, texcoord);
}
)";

const char* blurQuantizationDogFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform int screenLimit;

uniform sampler2D screenTexture;

float sigma_e = 2.0f;
float sigma_r = 2.8f;
float phi = 3.4f;
float tau = 0.99f;
float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;
int halfWidth = int(ceil( 2.0 * sigma_r ));


void main()
{
    if (gl_FragCoord.x < screenLimit) {
        outColor = texture(screenTexture, texcoord);
        return;
    }

    int half_size = 2;
    vec4 color_sum = vec4(0);
    for (int i = -half_size; i <= half_size ; ++i)
    {
        for (int j = -half_size; j <= half_size ; ++j)
        {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(screenTexture, coord, 0);
        }
    }
    int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
    
    float nbins = 8.0f;

    vec2 sum = vec2(0.0);
    vec2 norm = vec2(0.0);
    for ( int i = -halfWidth; i <= halfWidth; ++i ) {
        for ( int j = -halfWidth; j <= halfWidth; ++j ) {
            float d = length(vec2(i,j));
            vec2 kernel = vec2( exp( -d * d / twoSigmaESquared ), exp( -d * d / twoSigmaRSquared ));
            vec4 c = texture(screenTexture, texcoord + vec2(i,j) / 800);
            vec2 L = vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
            norm += kernel;
            sum += kernel * L;
        }
    }
    sum /= norm;
    float H = 100.0 * (sum.x - tau * sum.y);
    float edge =( H > 0.0 )?1.0:2.0 *smoothstep(-2.0, 2.0, phi * H );

    vec4 quant = floor(nbins * color_sum / sample_count) / nbins;
    outColor =  vec4(quant.x * edge, quant.y * edge, quant.z * edge,1.0 );
    
}
)";

const char* blurFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform sampler2D screenTexture;

void main(){
    int half_size = 2;
    vec4 color_sum = vec4(0);
    for (int i = -half_size; i <= half_size ; ++i)
    {
        for (int j = -half_size; j <= half_size ; ++j)
        {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(screenTexture, coord, 0);
        }
    }
    int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
    outColor = color_sum / sample_count;
}

)";


const char* quantizationFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform sampler2D screenTexture;

void main(){
    float nbins = 8.0;
    vec4 tex_color = texelFetch(screenTexture, ivec2(gl_FragCoord.xy), 0);
    tex_color = floor(tex_color * nbins) / nbins;
    outColor = vec4(tex_color.rgb, 1.0);
}
)";

const char* diffGaussianFS = R"(
#version 450 core

uniform sampler2D tex;

out vec4 color;

in vec2 texcoord;

float sigma_e = 2.0f;
float sigma_r = 2.8f;
float phi = 3.4f;
float tau = 0.99f;
float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;
int halfWidth = int(ceil( 2.0 * sigma_r ));

void main(void)
{
    vec2 sum = vec2(0.0);
    vec2 norm = vec2(0.0);
    for ( int i = -halfWidth; i <= halfWidth; ++i ) {
        for ( int j = -halfWidth; j <= halfWidth; ++j ) {
            float d = length(vec2(i,j));
            vec2 kernel= vec2( exp( -d * d / twoSigmaESquared ),
            exp( -d * d / twoSigmaRSquared ));
            vec4 c = texture(tex,texcoord + vec2(i,j) / 800);
            vec2 L = vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
            norm += kernel;
            sum += kernel * L;
        }
    }
    sum /= norm;
    float H = 100.0 * (sum.x - tau * sum.y);
    float edge =( H > 0.0 )?1.0:2.0 *smoothstep(-2.0, 2.0, phi * H );
    color = vec4(edge,edge,edge,1.0 );
}

)";


const char* waterFS = R"(
#version 420 core

out vec4 outColor;

in vec2 texcoord;

uniform int screenLimit;

float scale = 1.0 / 800.0;

layout (binding = 0) uniform sampler2D screenTexture;
layout (binding = 1) uniform sampler2D noiseTexture;

void main(void) {
    if (gl_FragCoord.x < screenLimit) {
        outColor = texture(screenTexture, texcoord);
        return;
    }

    vec4 noiseColor = texture(noiseTexture, texcoord / 4.0);
    vec2 distortedUV = texcoord + noiseColor.xy * 0.006;
    int half_size = 5;
    vec4 color_sum = vec4(0.0);

    for (int i = -half_size; i <= half_size ; ++i) {
        for (int j = -half_size; j <= half_size ; ++j) {
            vec2 offset = vec2(i, j) * scale;
            vec2 sampleUV = distortedUV + offset;
            color_sum += texture(screenTexture, sampleUV);
        }
    }

    int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);

    float nbins = 8.0;
    vec4 tex_color = color_sum / sample_count;
    tex_color = floor(tex_color * nbins) / nbins;
    outColor = vec4(tex_color.rgb, 1.0);
}
)";

const char* pixelFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform sampler2D screenTexture;
uniform int screenLimit;

void main() {
    if (gl_FragCoord.x < screenLimit) {
        outColor = texture(screenTexture, texcoord);
        return;
    }


    int blockSize = 10;
    
    int limitX = int(floor(gl_FragCoord.x / blockSize) * blockSize);
    int upperX = int(ceil(gl_FragCoord.x / blockSize) * blockSize);
    int limitY = int(floor(gl_FragCoord.y / blockSize) * blockSize);
    int upperY = int(ceil(gl_FragCoord.y / blockSize) * blockSize);

    vec4 color_sum = vec4(0.0);

    for (int i = limitX; i < upperX; ++i) {
        for (int j = limitY; j < upperY; ++j) {
            ivec2 coord = ivec2(i, j);
            color_sum += texelFetch(screenTexture, coord, 0);
        }
    }

    outColor = color_sum / float(blockSize * blockSize);
}
)";


const char* sinWaveFS = R"(
#version 450 core

out vec4 outColor;

uniform float offset;

in vec2 texcoord;

uniform sampler2D screenTexture;
uniform int screenLimit;

const float PI = 3.14159265359;

void main() {
    if (gl_FragCoord.x < screenLimit) {
        outColor = texture(screenTexture, texcoord);
        return;
    }

    vec2 coord = texcoord + 0.05f * sin(texcoord.y * 1.5f * PI + offset);

    outColor = texture(screenTexture, coord);
}
)";

const char* zoomFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform sampler2D screenTexture;
uniform int screenLimit;
uniform vec2 center = vec2(400, 400);

const float radius = 100.0f;

void main() {
    if (gl_FragCoord.x < screenLimit) {
        outColor = texture(screenTexture, texcoord);
        return;
    }

    vec2 coord = texcoord;

    float check = length(center - gl_FragCoord.xy);

    if(check < radius){
        vec2 scaledCenter = center /  800.0;
        coord = (texcoord - scaledCenter) / 2.0 + scaledCenter;
    }
    else if(check < radius + 2.0){
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    outColor = texture(screenTexture, coord);
}
)";


// second pass of blur kernel will be
/*
    1 2 3 2 1
    2 4 6 4 2
    3 6 9 6 3
    2 4 6 4 2
    1 2 3 2 1

    divided by 81

    if the kernel is 3x3

Python proof:

arr = [
    ['a', 'b', 'c', 'd', 'e'],
    ['f', 'g', 'h', 'i', 'j'],
    ['k', 'l', 'm', 'n', 'o'],
    ['p', 'q', 'r', 's', 't'],
    ['u', 'v', 'w', 'x', 'y']
]

dicc = {}

for i in range(1, 4):
    for j in range(1, 4):
        for x in range(i-1, i+2):
            for y in range(j-1, j+2):
                if arr[x][y] not in dicc:
                    dicc[arr[x][y]] = 0
                dicc[arr[x][y]] += 1

sorted_counts = {key: dicc[key] for key in sorted(dicc.keys())}

i = 0
for key in sorted_counts:
    print(sorted_counts[key], end="\n" if i%5 == 4 else " ")
    i+=1
*/

const char* bloomFS = R"(
#version 450 core

out vec4 outColor;

in vec2 texcoord;

uniform sampler2D screenTexture;
uniform int screenLimit;

const int kernel[25] = int[](
    1, 2, 3, 2, 1,
    2, 4, 6, 4, 2,
    3, 6, 9, 6, 3,
    2, 4, 6, 4, 2,
    1, 2, 3, 2, 1
);

void main() {
    if (gl_FragCoord.x < screenLimit) {
        outColor = texture(screenTexture, texcoord);
        return;
    }

    int half_size = 1;
    vec4 color_sum = vec4(0.0);
    for (int i = -half_size; i <= half_size; ++i) {
        for (int j = -half_size; j <= half_size; ++j) {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(screenTexture, coord, 0);
        }
    }
    vec4 firstPass = color_sum / 9.0;

    half_size = 2;
    color_sum = vec4(0.0);

    for (int i = -half_size; i <= half_size; ++i) {
        for (int j = -half_size; j <= half_size; ++j) {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            vec4 tex = texelFetch(screenTexture, coord, 0);
            color_sum += tex * float(kernel[(i + half_size) * 5 + j + half_size]);
        }
    }
    vec4 secondPass = color_sum / 81.0;
    outColor = texture(screenTexture, texcoord) + ( secondPass + firstPass ) * 0.5;
    //texture(screenTexture, texcoord) + secondPass + 
}
)";
