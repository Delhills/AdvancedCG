varying vec3 v_world_position;

uniform vec4 u_color;
uniform samplerCube u_texture;
uniform vec3 u_camera_position;

void main()
{
	vec3 V = v_world_position - u_camera_position;
	vec4 color = textureCube(u_texture, V);
	gl_FragColor = color;
}