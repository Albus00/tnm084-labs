#version 150

out vec4 outColor;

in vec2 texCoord;
in vec3 exNormal;
in float height;
uniform sampler2D tex;

void main(void)
{
	// Texture from disc
	vec4 t = texture(tex, texCoord);
	vec3 sand = vec3(0.76, 0.70, 0.50);
	vec3 rock = vec3(0.5, 0.5, 0.5);
	vec3 grass = vec3(0.0, 0.9, 0.0);
	
	// Procedural texture
	// t.r = sin(texCoord.s * 3.1416);
	// t.g = sin(texCoord.t * 3.1416);
	// t.b = sin((texCoord.s + texCoord.t) * 10.0);

	if (height == 0.0) {
		t.r = 0.;
		t.g = 0.;
		t.b = 0.9;
	}
	else if (height < 0.05) {
		t.r = mix(sand, grass, height).x;
		t.g = mix(sand, grass, height).y;
		t.b = mix(sand, grass, height).z;
	}
	else {
		t.r = mix(grass, rock, height).x;
		t.g = mix(grass, rock, height).y;
		t.b = mix(grass, rock, height).z;
	}


	// t.r = ;
	// t.g = 0.1;
	// t.b = 0.1;
	
	vec3 n = normalize(exNormal);
	float shade = n.y + n.z;
//	if (t.a < 0.01) discard;
//	else
		outColor = t * shade * shade; // Over-emphasized fake light
	
//	outColor = vec4(texCoord.s, texCoord.t, 0, 1);
//	outColor = vec4(n.x, n.y, n.z, 1);
//	outColor = vec4(1) * shade;
}
