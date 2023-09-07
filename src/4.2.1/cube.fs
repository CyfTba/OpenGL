#version 330

out vec4 FragColor;
uniform sampler2D texture_diffuse1;
in vec2 TexCoord;
uniform sampler2D texture0;
void main()
{    
    FragColor = texture(texture0,TexCoord);
   
}