#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 clothGain;
uniform vec3 leatherGain;
uniform vec3 skinGain;
out vec4 finalColor;
void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    vec3 c = texel.rgb * 255.0;
    bool skin = c.r > 155.0 && c.r-c.g > 25.0 && c.g-c.b > 12.0;
    bool leather = c.r-c.g > 8.0 && c.g-c.b > 3.0 && c.r > c.b*1.2 && !skin;
    bool cloth = c.b >= c.r-3.0 && c.b >= c.g-3.0 && max(c.r,max(c.g,c.b)) < 170.0 && !skin && !leather;
    vec3 gain = vec3(1.0);
    if (skin) gain = skinGain;
    else if (leather) gain = leatherGain;
    else if (cloth) gain = clothGain;
    // Keep alpha and sword highlights; apply gameplay tint after material correction.
    finalColor = vec4(clamp(texel.rgb*gain,0.0,1.0),texel.a) * colDiffuse * fragColor;
}
