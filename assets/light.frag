#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool gamma;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // diffuse
    vec3 lightDir = normalize(fragPos - lightPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 1.0);
    vec3 specular = spec * lightColor;
    // simple attenuation
    float max_distance = 10.0;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (gamma ? distance * distance : distance);

    diffuse *= attenuation;
    specular *= attenuation;

    return diffuse + specular + 0.01f;
}

void main()
{
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 lighting = BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPos, lightColor);
    color *= lighting;
    if(gamma)
        color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}
