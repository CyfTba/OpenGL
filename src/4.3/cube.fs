#version 330

out vec4 FragColor;
uniform sampler2D texture0;
in vec2 TexCoord;
void main()
{    
    vec4 texColor = texture(texture0,TexCoord);
    if(texColor.a<0.01)
    discard;
    FragColor=texColor;
   
}