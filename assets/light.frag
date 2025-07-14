#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 lightColor; // Added for light color
uniform vec3 viewPos;
uniform float shininess; // Renamed for clarity, controls specular highlight size
uniform float amb_coeff;

void main()
{

    // Texture color
    vec3 texColor = texture(texture_diffuse1, fs_in.TexCoords).rgb;

    // Ambient
    vec3 ambient = amb_coeff * lightColor * texColor;

    // Diffuse
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * texColor;

    // Specular (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = lightColor * spec; // Specular component reflects the light's color

    // Final color
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
