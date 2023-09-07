#version 330

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;//像素位置
in vec3 Normal;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 viewPos;
//点光源
struct PointLight{
	vec3 position;//位置
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;//常项
	float linear;//一次项
	float quadratic;//二次项
};

uniform PointLight light;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);//灯光向量
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // 合并结果
    vec3 ambient  = light.ambient*vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient+diffuse+specular);
}

float near =0.1;
float far =100.0;
//线性深度
float LinearizeDepth(float depth){
    float z=depth *2.0-1.0;
    return (2.0*near*far)/(far+near-z*(far-near));

}
void main()
{   
    vec3 norm=normalize(Normal);
    vec3 viewDir=normalize(viewPos-FragPos);
    vec3 result=CalcPointLight(light,norm,FragPos,viewDir);
    float depth=LinearizeDepth(gl_FragCoord.z);
    FragColor = vec4(vec3(depth),1.0f);
   
}


