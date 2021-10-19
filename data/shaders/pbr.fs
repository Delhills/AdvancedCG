#define RECIPROCAL_PI 0.3183098861837697
#define PI 3.1415926535897932384626433832795

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

//the texture passed from the application
uniform sampler2D u_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_metallic_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_ao_texture;
uniform sampler2D u_lut;

uniform bool u_use_emissivetxt;
uniform bool u_use_metaltxt;
uniform bool u_use_aotxt;

uniform float u_metallic_factor;
uniform float u_roughness_factor;
uniform float u_emission_factor;

uniform samplerCube u_environment_texture;

uniform mat4 u_model;

//here create uniforms for all the data we need here
uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;
uniform float u_light_intensity;
uniform float u_light_color;

uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;
uniform vec3 u_material_specular;
uniform float u_material_shininess;

uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform bool u_first_pass;
uniform float u_output;

uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

struct PBRMat
{
	vec4 albedo;
	vec3 N;
	vec3 emission;
	float roughness;
	float metallic;
	float ao;
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

void main()
{
	// 1. Create Material
	PBRMat material;
	
	// 2. Fill Material
	material.albedo = u_color * texture2D(u_texture, v_uv);
	vec3 basecolor = material.albedo.xyz;
	material.albedo.xyz = gamma_to_linear(material.albedo.xyz);

	if(u_use_emissivetxt) {
		material.emission.xyz = texture2D(u_emissive_texture, v_uv).xyz;
		material.emission.xyz = gamma_to_linear(material.emission.xyz) * u_emission_factor;
	}
	else{
		material.emission.xyz = vec3(0.0);
	}

	if(u_use_aotxt) {
		material.ao = texture2D(u_ao_texture, v_uv).x;
	}
	else{
		material.ao = 1.0;
	}
	
	vec3 normal = normalize(v_normal);
	material.N = texture2D(u_normal_texture, v_uv).xyz;
	material.N = perturbNormal(normal, v_world_position, v_uv, material.N);

	vec4 roughness_metallic = texture2D(u_roughness_texture, v_uv);
	material.roughness = roughness_metallic.y * u_roughness_factor;

	if(u_use_metaltxt){
		material.metallic = texture2D(u_metallic_texture, v_uv).z * u_metallic_factor;
	}
	else{
		material.metallic = roughness_metallic.z * u_metallic_factor;
	}

	// 3. Shade (Direct + Indirect)
	//read the pixel RGBA from the texture at the position v_position
	vec3 light = vec3(0.0);
	//light += gamma_to_linear(u_light_ambient) * material.ao;

	//Compute vectors
	vec3 N = material.N;
	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 L = normalize(u_light_position - v_world_position);
	vec3 H = normalize(L+V);

	float NdotL = max(dot(N,L),0.0);
	float NdotH = max(dot(N,H),0.0);
	float NdotV = max(dot(N,V),0.0);
	float LdotH = max(dot(L,H),0.0);

	vec3 F_0 = vec3(0.04); //common material
	F_0 = mix( F_0, material.albedo.xyz, material.metallic );

	vec3 F = compute_F(F_0, LdotH); //F_0 + (1.0 - F_0)*pow((1.0 - LdotH), 5.0); OKAY
	float D = compute_D(material.roughness, NdotH); //a2 * RECIPROCAL_PI * 1.0 / pow(NdotH * NdotH * (a2 - 1.0) + 1.0, 2.0); //OKAY
	float G = compute_G(material.roughness, NdotL, NdotV); //(NdotL / (NdotL * (1.0 - k) + k)) * (NdotV / (NdotV * (1.0 - k) + k)); //OKAY

	//Diffuse component
	//vec3 kD = vec3(1.0) - F;
	vec3 f_diff = (1.0 - material.metallic) * material.albedo.xyz * RECIPROCAL_PI * NdotL;//(1.0 - material.metallic) * kD * material.albedo.xyz * RECIPROCAL_PI * NdotL;
	vec3 diffuseColor = (1.0 - material.metallic) * material.albedo.xyz;

	//Specular component
	vec3 f_specular = (F * G * D) / (4.0 * NdotL * NdotV + 1e-6);	

	vec3 SpecularIBL = vec3(0.0);
	vec3 DiffuseIBL = vec3(0.0);
	//compute the reflection
	vec2 uv_lut = vec2(NdotV, material.roughness);
	//vec2 uv_lut = vec2(material.roughness, NdotV);

	vec4 brdf2D = texture2D(u_lut, uv_lut);
	if (u_first_pass)
	{
		vec3 F_IBL = FresnelSchlickRoughness(NdotV, F_0, material.roughness);
		//vec3 F_IBL = mix(vec3(0.04), material.albedo, material.metallic).rgb;
		vec3 R = reflect(-V,N);

		vec3 specularSample = getReflectionColor(R, material.roughness).xyz;
		vec3 SpecularBRDF = F_IBL * brdf2D.x + brdf2D.y;
		SpecularIBL = specularSample * SpecularBRDF;

		vec3 kD_IBL = vec3(1.0) - F_IBL;
		vec3 diffuseSample = getReflectionColor(N, 1.0).xyz;

		DiffuseIBL = (diffuseSample * diffuseColor) * kD_IBL;
	}

	
	light += (f_diff + f_specular) * u_light_intensity  + (SpecularIBL + DiffuseIBL) * material.ao; // (vec3(1.0) - F)
	material.albedo.xyz *= light;

	if (u_first_pass)
		material.albedo.xyz += material.emission.xyz;

	// 4. Apply Tonemapping
	// ...
	material.albedo.xyz = toneMapUncharted(material.albedo.xyz);

	// 5. Any extra texture to apply after tonemapping
	// ...

	// Last step: to gamma space
	// ...
	material.albedo.xyz = linear_to_gamma(material.albedo.xyz);

	if (u_output == 0.0) gl_FragColor = vec4(material.albedo.xyz, 1.0);
//	if (u_output == 0.0) gl_FragColor = vec4(brdf2D.xyz, 1.0);

	if (u_output == 1.0) gl_FragColor = vec4(basecolor.xyz, 1.0);
	if (u_output == 2.0) gl_FragColor = vec4(material.roughness);
	if (u_output == 3.0) gl_FragColor = vec4(material.metallic);
	if (u_output == 4.0) gl_FragColor = vec4(material.N, 1.0);
}

//	float a = material.roughness * material.roughness;
//	float a2 = a * a;
//	float k = pow(material.roughness + 1.0, 2.0) / 8.0;