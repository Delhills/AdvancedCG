varying vec3 v_world_position;
varying vec3 v_position;

uniform samplerCube u_texture;
uniform vec3 u_camera_position;

void main()
{
	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 color = textureCube(u_texture, -V).xyz;
	gl_FragColor = vec4(color, 1.0);
}