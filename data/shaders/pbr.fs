#define RECIPROCAL_PI 0.3183098861837697
#define PI = 3.14159265359

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

//the texture passed from the application
uniform sampler2D u_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_metallic_roughness_texture;
uniform sampler2D u_ao_texture;
uniform sampler2D u_lut;
uniform sampler2D u_opacity_texture;
uniform sampler2D u_metallic_texture;
uniform sampler2D u_roughness_texture;
uniform samplerCube u_environment_texture;
uniform mat4 u_model;

//here create uniforms for all the data we need here
uniform vec3 u_light_position[100];
uniform float u_light_intensity[100];
uniform vec3 u_light_color[100];

uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform float u_first_pass;
uniform float u_metallic_roughness;
uniform float u_metallic_factor;
uniform float u_roughness_factor;
uniform float u_normal_factor;

uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

uniform float u_output;

uniform float u_num_lights;

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

struct PBRMat
{
	vec4 albedo;
	vec3 N;
	vec3 emission;
	vec3 F_0;
	vec3 diffuse_color;
	float roughness;
	float metallic;
	float ao;
	float opacity;
};

// degamma
vec3 gamma_to_linear(vec3 color)
{
	return pow(color, vec3(GAMMA));
}

// gamma
vec3 linear_to_gamma(vec3 color)
{
	return pow(color, vec3(INV_GAMMA));
}

vec3 getReflectionColor(vec3 r, float roughness)
{
	float lod = roughness * 5.0;

	vec4 color;

	if(lod < 1.0) color = mix( textureCube(u_environment_texture, r), textureCube(u_texture_prem_0, r), lod );
	else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, r), textureCube(u_texture_prem_1, r), lod - 1.0 );
	else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, r), textureCube(u_texture_prem_2, r), lod - 2.0 );
	else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, r), textureCube(u_texture_prem_3, r), lod - 3.0 );
	else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, r), textureCube(u_texture_prem_4, r), lod - 4.0 );
	else color = textureCube(u_texture_prem_4, r);

	return color.rgb;
}

//Javi Agenjo Snipet for Bump Mapping
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv){
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

vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel ){
	#ifdef USE_POINTS
	return N;
	#endif

	// assume N, the interpolated vertex normal and
	// V, the view vector (vertex to eye)
	//vec3 normal_pixel = texture2D(normalmap, texcoord ).xyz;
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


/* 
	Convert 0-Inf range to 0-1 range so we can
	display info on screen
*/
vec3 toneMap(vec3 color)
{
    return color / (color + vec3(1.0));
}

// Uncharted 2 tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 toneMapUncharted2Impl(vec3 color)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 toneMapUncharted(vec3 color)
{
    const float W = 11.2;
    color = toneMapUncharted2Impl(color * 2.0);
    vec3 whiteScale = 1.0 / toneMapUncharted2Impl(vec3(W));
    return color * whiteScale;
}

vec3 compute_F(vec3 F_0, float LdotH)
{
	return F_0 + (1.0 - F_0)*pow((1.0 - LdotH), 5.0);
}

float compute_D(float roughness, float NdotH)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float f = NdotH * NdotH * (a2 - 1.0) + 1.0;
	return a2 * RECIPROCAL_PI * 1.0 / (f * f);
}

float compute_G(float roughness, float NdotL, float NdotV)
{
	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	float G1 = (NdotL / (NdotL * (1.0 - k) + k));
	float G2 = (NdotV / (NdotV * (1.0 - k) + k));
	return G1 * G2;
}

void getMaterialProperties(inout PBRMat material)
{
	material.opacity = texture2D(u_opacity_texture, v_uv).x;

	material.albedo = u_color * texture2D(u_texture, v_uv);
	material.albedo.xyz = gamma_to_linear(material.albedo.xyz);
	material.emission.xyz = texture2D(u_emissive_texture, v_uv).xyz;
	material.emission.xyz = gamma_to_linear(material.emission.xyz);
	
	vec3 normal = normalize(v_normal);
	material.N = texture2D(u_normal_texture, v_uv).xyz;
	material.N = mix(normal, perturbNormal(normal, v_world_position, v_uv, material.N), u_normal_factor);

	if (u_metallic_roughness)
	{
		vec2 metallic_roughness = texture2D(u_metallic_roughness_texture, v_uv).yz;
		material.roughness = metallic_roughness.x * u_roughness_factor;
		material.metallic = metallic_roughness.y * u_metallic_factor;
	}
	else{	
		material.roughness = texture2D(u_roughness_texture, v_uv).x * u_roughness_factor;
		material.metallic = texture2D(u_metallic_texture, v_uv).x * u_metallic_factor;
	}

	material.ao = texture2D(u_ao_texture, v_uv).x;

	vec3 F0 = vec3(0.04); //common material
	material.F_0 = mix( F0, material.albedo.xyz, material.metallic );
	material.diffuse_color = (1.0 - material.metallic) * material.albedo.xyz; 
}

vec3 getPixelColor(PBRMat material)
{
	vec3 N, V, L, H;
	N = material.N;
	V = normalize(u_camera_position - v_world_position);

	//Compute dot products
	float NdotV = max(dot(N,V),0.0);
	
	//compute the IBL
	vec3 F_IBL = FresnelSchlickRoughness(NdotV, material.F_0, material.roughness);
	vec3 R = reflect(-V,N);
	vec2 uv_lut = vec2(clamp(NdotV, 0.0, 1.0), clamp(material.roughness, 0.01, 0.99));
	vec2 brdf2D = texture2D(u_lut, uv_lut);

	vec3 specularSample = getReflectionColor(R, material.roughness).xyz;
	vec3 SpecularBRDF = F_IBL * brdf2D.x + brdf2D.y;
	vec3 SpecularIBL = specularSample * SpecularBRDF;

	vec3 kD_IBL = vec3(1.0) - F_IBL;
	vec3 diffuseSample = getReflectionColor(N, 1.0).xyz;
	vec3 DiffuseIBL = (diffuseSample * material.diffuse_color) * kD_IBL;
	vec3 indirect_light = (SpecularIBL + DiffuseIBL) * material.ao;
	vec3 color = indirect_light;

	for (float i = 0.0; i < 100; ++i)
	{
		if (i <= u_num_lights)
		{
			L = normalize(u_light_position[i] - v_world_position);
			H = normalize(L+V);

			vec3 NdotL = max(dot(N,L),0.0);
			vec3 NdotH = max(dot(N,H),0.0);
			vec3 LdotH = max(dot(L,H),0.0);

			//Compute BRDF elements
			vec3 F = compute_F(material.F_0, LdotH); 
			float D = compute_D(material.roughness, NdotH);
			float G = compute_G(material.roughness, NdotL, NdotV); 

			//Diffuse component
			vec3 f_diff = material.diffuse_color * RECIPROCAL_PI * NdotL;

			//Specular component
			vec3 f_specular = (F * G * D) / (4.0 * NdotL * NdotV + 1e-6);	

			vec3 direct_light = (f_diff + f_specular) * u_light_intensity[i] * u_light_color[i]; 
			color += direct_light;
		}
	}

	color += material.emission.xyz;

	return color;
}


void main()
{
	// 1. Create Material
	PBRMat material;
	
	// 2. Fill Material
	getMaterialProperties(material);

	// 3. Shade (Direct + Indirect)
	//Get pixel color
	vec3 color = getPixelColor(material);

	// 4. Apply Tonemapping
	// ...
	color = toneMapUncharted(color);

	// 5. Any extra texture to apply after tonemapping
	// ...
	if (u_output == 1)
		color = vec4(material.albedo.xyz, 1.0);

	// Last step: to gamma space
	// ...
	color = linear_to_gamma(color);
;
	if (u_output == 2)
		color = vec4(vec3(material.roughness), 1.0);
	if (u_output == 3)
		color = vec4(vec3(material.metallic), 1.0);
	if (u_output == 4)
		color = vec4(material.N, 1.0);

	if (gl_FrontFacing) 
		gl_FragColor = vec4(color, material.opacity);
}