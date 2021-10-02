varying vec3 v_world_position;
varying vec3 v_normal;

uniform samplerCube u_texture;
uniform vec3 u_camera_position;

void main()
{
	vec3 N = normalize(v_normal);

	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 R = reflect(-V,N);

	//compute the reflection
	vec3 reflection = textureCube( u_texture, R ).xyz;
	gl_FragColor = vec4(reflection, 1.0);
}
