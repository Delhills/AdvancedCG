varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;


uniform vec3 u_camera_position;

uniform sampler2D u_texture;

uniform mat4 u_model;

uniform vec3 u_ambient_light;
uniform vec3 u_light_diff;
uniform vec3 u_light_spec;
uniform vec3 u_light_pos;
uniform float u_light_intensity;

uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;
uniform vec3 u_material_specular;
uniform float u_material_gloss;

void main() {
    
	vec3 N = normalize(v_normal);
    vec2 uv = v_uv;
    vec4 color = texture2D( u_texture, uv ) * u_light_intensity;

	vec3 L = normalize( u_light_pos - v_world_position );
	vec3 V = normalize( u_camera_position - v_world_position );
	vec3 R = normalize( reflect(V,N) );

	float NdotL = max(0.0, dot(N,L));
	float RdotL = pow(max(0.0, dot(-R,L)), u_material_gloss * color.a);

    vec3 ambient = u_ambient_light * u_material_ambient;
	vec3 diffuse = u_material_diffuse * u_light_diff * NdotL; 
	vec3 specular = u_material_specular * u_light_spec * RdotL * color.a;
	vec3 finalColor = color.xyz * (ambient + diffuse + specular);

	gl_FragColor = vec4( finalColor, 1.0);
}