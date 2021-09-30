varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

//the texture passed from the application
uniform sampler2D u_texture;
uniform sampler2D u_texture_normals;
uniform mat4 u_model;

//here create uniforms for all the data we need here
uniform vec3 light_ambient;
uniform vec3 light_position;
uniform vec3 light_diffuse;
uniform vec3 light_specular;
uniform float light_intensity;
uniform float u_light_maxdist;
uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;
uniform vec3 u_camera_position;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
    
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

// assume N, the interpolated vertex normal and 
// WP the world position
//vec3 normal_pixel = texture2D( normalmap, uv ).xyz; 
vec3 perturbNormal(vec3 N, vec3 WP, vec2 uv, vec3 normal_pixel)
{
    normal_pixel = normal_pixel * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame(N, WP, uv);
    return normalize(TBN * normal_pixel);
}

void main()
{
	//read the pixel RGBA from the texture at the position v_position
	vec4 color = texture2D( u_texture, v_uv );
	vec3 normal = normalize(v_normal);
	vec3 normal_pixel = texture2D( u_texture_normals, v_uv ).xyz;
	normal =  perturbNormal(normal, v_world_position, v_uv, normal_pixel);

	vec3 Ka = material_ambient * color.xyz;
	vec3 Kd = material_diffuse * color.xyz;
	vec3 Ks = material_specular * color.xyz * color.w;

	//here write the computations for PHONG.
	vec3 KaIa = light_ambient * Ka;
	
	//Defining the light
	vec3 L = light_position - v_world_position;
				
	//compute distance and define the attenuation factor
	float light_distance = length( L );

	//compute a linear attenuation factor
	float att_factor = u_light_maxdist - light_distance;

	//normalize factor
	att_factor /= u_light_maxdist;

	//ignore negative values
	att_factor = max( att_factor, 0.0 );

	//Normalizing L for the point and spot light dot products
	L = normalize(L);

	float LdotN = clamp(dot(normal, L), 0.0, 1.0);
	vec3 diff = LdotN * (Kd * light_diffuse);

	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 R = reflect(-L, normal);
	float RV = pow(max(dot(V, R), 0.0), material_shininess);
	vec3 spec = RV * Ks * light_specular;

	vec3 illumination = (KaIa + diff + spec) * light_intensity * att_factor;

	//assign the color to the output
	gl_FragColor = vec4(illumination, 1.0);
}