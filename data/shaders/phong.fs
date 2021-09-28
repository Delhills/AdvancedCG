varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

//the texture passed from the application
uniform sampler2D u_texture;
uniform sampler2D texture_normals;
uniform mat4 u_model;

//here create uniforms for all the data we need here
uniform vec3 light_ambient;
uniform vec3 light_position;
uniform vec3 light_diffuse;
uniform vec3 light_specular;
uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;
uniform vec3 u_camera_position;

void main()
{
	//read the pixel RGBA from the texture at the position v_position
	vec4 color = texture2D( u_texture, v_uv );
	vec3 normal = normalize(v_normal);

	vec3 Ka = material_ambient * color.xyz;
	vec3 Kd = material_diffuse * color.xyz;
	vec3 Ks = material_specular * color.xyz * color.w;

	//here write the computations for PHONG.
	vec3 KaIa = light_ambient * Ka;

	vec3 L = normalize(light_position - v_world_position);
	float LdotN = clamp(dot(normal, L), 0.0, 1.0);
	vec3 diff = LdotN * (Kd * light_diffuse);

	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 R = reflect(-L, normal);
	float RV = pow(max(dot(V, R), 0.0), material_shininess);
	vec3 spec = RV * Ks * light_specular;

	vec3 illumination = KaIa + diff + spec;

	//assign the color to the output
	gl_FragColor = vec4(illumination, 1.0);
}