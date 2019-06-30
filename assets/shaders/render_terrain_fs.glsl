#version 330 core

in vec3 v_vertPos;
in vec3 v_vertNormal;
in vec3 v_vertColor;

out vec4 o_color;

uniform sampler2D s_textureGrass;
uniform sampler2D s_textureRock;
uniform vec4 u_color = vec4(1, 1, 1, 1);
uniform float u_textureScaleInv = 1.0;

void main()
{
	// Determine the blend weights for the 3 planar projections.
	// N_orig is the vertex-interpolated normal vector.
	vec3 blend_weights = v_vertNormal;   // Tighten up the blending zone:
	blend_weights.x *= blend_weights.x;
	blend_weights.y *= blend_weights.y;
	blend_weights = abs(normalize(blend_weights));
	blend_weights = (blend_weights - 0.2) * 7;
	blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)
	blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z); 
	
	// Now determine a color value and bump vector for each of the 3
	// projections, blend them, and store blended results in these two
	// vectors:

	// Compute the UV coords for each of the 3 planar projections.
	// tex_scale (default ~ 1.0) determines how big the textures appear.
	vec3 scaledVertPos = v_vertPos * u_textureScaleInv;
	vec2 coord1 = scaledVertPos.yz;
	vec2 coord2 = scaledVertPos.zx;
	vec2 coord3 = scaledVertPos.xy;
	
	// Sample color maps for each projection, at those UV coords.
	vec4 col1 = texture2D(s_textureRock, coord1);
	vec4 col2 = texture2D(s_textureRock, coord2);
	vec4 col3 = texture2D(s_textureGrass, coord3) * vec4(v_vertColor, 1.0);

	// Finally, blend the results of the 3 planar projections.
	vec4 diffuse =
		col1 * blend_weights.x +
		col2 * blend_weights.y +
		col3 * blend_weights.z;

	vec3 lightDir = normalize(vec3(0.2, 0.6, -1));
	float light = dot(v_vertNormal, -lightDir);
	light = (light + 1.0) * 0.5;
	o_color = diffuse * u_color * vec4(vec3(light), 1.0);
}

