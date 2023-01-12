#version 330 core

in vec3 outRGB;
out vec4 outColor;
in vec2 outTexcoord;

uniform vec4 col_mul;
uniform vec4 grad_mul;
uniform float visibility_both;
uniform sampler2D textureSampler;
uniform int disableTexture;
uniform int enableGradient;
uniform int enableBoth;

void main()
{
	vec4 tex_col = texture(textureSampler, outTexcoord) * col_mul;
	if(enableBoth != 0)
		outColor = texture(textureSampler, outTexcoord) * vec4(outRGB, 0.0) * visibility_both;
	else if(enableGradient != 0)
		outColor = vec4(outRGB, 0.0) * grad_mul;
	else if(disableTexture == 0)
		outColor = tex_col;
	else if(disableTexture == 0 && enableBoth == 0 && enableGradient == 0)
		outColor = vec4(0.0, 0.0, 0.0, 0.0);
}