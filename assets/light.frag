#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;


uniform bool has_texture;      // Flag to control which color source to use
uniform vec3 default_color;
uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;


void main()
{

    vec3 color;
    if (has_texture) {
        color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    } else {
        color = default_color;
    }

    float ambient_strength = 0.5f;
    vec3 ambient_color = lightColor * ambient_strength;
    vec3 normal = normalize(fs_in.Normal);
    vec3 light_dir = normalize(lightPos - fs_in.FragPos);
    float diffuse_strength = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = lightColor * diffuse_strength;

    vec3 view_dir = normalize(viewPos -  fs_in.FragPos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular_color = specular_strength * lightColor;
    vec3 result = (ambient_color + diffuse_color + specular_color) * color.xyz;

    FragColor = vec4(result, 1.0);
}
