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
uniform sampler2D u_ao_texture;
uniform sampler2D u_lut;
uniform sampler2D u_opacity_texture;
uniform sampler2D u_metallic_texture;
uniform sampler2D u_roughness_texture;
uniform samplerCube u_environment_texture;
uniform mat4 u_model;

//here create uniforms for all the data we need here
const int MAX_LIGHTS = 100;
uniform vec3 u_light_position[MAX_LIGHTS];
uniform float u_light_intensity[MAX_LIGHTS];
uniform vec3 u_light_color[MAX_LIGHTS];

uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform float u_first_pass;
uniform bool u_metallic_roughness;
uniform float u_metallic_factor;
uniform float u_roughness_factor;
uniform float u_subsurface_factor;
uniform float u_diffuse_type;

uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

uniform float u_output;
uniform float u_num_lights;

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

//Struct for the material
struct PBRMat
{
	vec3 base_color;
	vec3 emission;
	vec3 N;
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
	return F_0 + (1.0 - F_0) * pow(clamp(1.0 - LdotH, 0.0, 1.0), 5.0);
}

float compute_D(float roughness, float NdotH)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float f = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
	return alpha2 * RECIPROCAL_PI * 1.0 / (f * f);
}

float compute_G(float roughness, float NdotL, float NdotV)
{
	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	float G1 = NdotL / (NdotL * (1.0 - k) + k);
	float G2 = NdotV / (NdotV * (1.0 - k) + k);
	return G1 * G2;
}

void getMaterialProperties(inout PBRMat material, vec3 V)
{
	//Define base color
	material.base_color = u_color.xyz * texture2D(u_texture, v_uv).xyz;
	material.base_color = gamma_to_linear(material.base_color);

	//Define emission
	material.emission = texture2D(u_emissive_texture, v_uv).xyz;
	material.emission = gamma_to_linear(material.emission);
	
	//Define normals
	vec3 normal = normalize(v_normal);
	material.N = texture2D(u_normal_texture, v_uv).xyz;
	material.N = perturbNormal(normal, -V, v_uv, material.N); //Mix to enchance normals

	//If metallic and roughness are in the same texture...
	if (u_metallic_roughness)
	{
		vec2 metallic_roughness = texture2D(u_roughness_texture, v_uv).yz; //Roughness G, Metalness B
		material.roughness = u_roughness_factor; //0.4 segun EPIC
		material.metallic = u_metallic_factor;
	}
	else{	//If not...
		material.roughness = texture2D(u_roughness_texture, v_uv).x * u_roughness_factor;
		material.metallic = texture2D(u_metallic_texture, v_uv).x * u_metallic_factor;
	}

	//Ambient occlusion and opacity
	material.ao = texture2D(u_ao_texture, v_uv).x;
	material.opacity = texture2D(u_opacity_texture, v_uv).x;

	//Define the material diffuse color and F0
	material.F_0 = mix( vec3(0.04), material.base_color.xyz, material.metallic );
	material.diffuse_color = mix( material.base_color.xyz, vec3(0.0), material.metallic ); 
}

vec3 computeIndirectLight(PBRMat material, vec3 V, float NdotV)
{
	//compute IBL needed values
	vec3 F_IBL = FresnelSchlickRoughness(NdotV, material.F_0, material.roughness);
	vec3 R = reflect(-V, material.N);
	vec2 uv_lut = vec2(NdotV, material.roughness);
	vec2 brdf2D = texture2D(u_lut, uv_lut).xy;

	//Specular IBL
	vec3 specularSample = getReflectionColor(R, material.roughness).xyz;
	vec3 SpecularBRDF = F_IBL * brdf2D.x + brdf2D.y;
	vec3 SpecularIBL = specularSample * SpecularBRDF;

	//Diffuse IBL
	vec3 diffuseSample = getReflectionColor(material.N, 1.0).xyz;
	vec3 DiffuseIBL = (diffuseSample * material.diffuse_color) * (vec3(1.0) - F_IBL);

	//Result
	return (SpecularIBL + DiffuseIBL) * material.ao;
}

vec3 computeDirectLight(PBRMat material, vec3 V, float NdotV)
{
	//Declare vectors and dot produts
	vec3 L, H;
	float NdotL, NdotH, LdotH;

	//Initial direct light
	vec3 direct_light = vec3(0.0);
	
	//Use all lights
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		if (i < u_num_lights)
		{
			//Compute vectors
			L = normalize(u_light_position[i] - v_world_position);
			H = normalize(L+V);
	
			//Compute dot products
			NdotL = clamp(dot(material.N,L),0.0, 1.0);
			NdotH = clamp(dot(material.N,H),0.0, 1.0);
			LdotH = clamp(dot(L,H),0.0, 1.0);
	
			//Compute BRDF elements
			vec3 F = compute_F(material.F_0, LdotH); 
			float D = compute_D(material.roughness, NdotH);
			float G = compute_G(material.roughness, NdotL, NdotV); 
	
			vec3 f_diff;
			//Diffuse component (BURLEY)
			if (u_diffuse_type == 0)
			{
				float fSS90 = material.roughness * LdotH * LdotH;
				float lightScatter = 1.0 + (fSS90 - 1.0) * pow(1.0 - NdotL, 5.0);
				float viewScatter  = 1.0 + (fSS90 - 1.0) * pow(1.0 - NdotV, 5.0);
				float scatter =  lightScatter * viewScatter;
				float f = NdotL * NdotV;
				
				float fss = abs((1.0 / (f + 1e-6) - 0.5) * scatter + 0.5);
				
				float fd90 = 0.5 + 2 *  material.roughness * LdotH * LdotH;
				float lightScatter_d = 1.0 + (fd90 - 1.0) * pow(1.0 - NdotL, 5.0);
				float viewScatter_d  = 1.0 + (fd90 - 1.0) * pow(1.0 - NdotV, 5.0);
				float fd =  lightScatter_d * viewScatter_d;
				
				f_diff = material.diffuse_color * RECIPROCAL_PI * (fss * 1.25 * u_subsurface_factor + (1 - u_subsurface_factor) * fd) * NdotV * NdotL;
			}

			//Diffuse component (HAMMON)
			if (u_diffuse_type == 1)
			{
				float f_multi = 0.3641 * material.roughness * material.roughness;
				float k_facing = 0.5 + clamp(dot(L,V),0.0, 1.0);
				float f_rough = k_facing * (0.9 - 0.4 * k_facing) * ((0.5 + NdotH) / (NdotH + 1e-6));
				float lightScatter = 1.0 - pow(1.0 - NdotL, 5.0);
				float viewScatter  = 1.0 - pow(1.0 - NdotV, 5.0);
				float scatter =  lightScatter * viewScatter;
				float f_smooth = 21/20 * (1 - material.F_0) * scatter;
				float fss = (1.0 - material.roughness * material.roughness) * f_smooth + material.roughness * material.roughness * f_rough + material.diffuse_color * f_multi;
				f_diff = material.diffuse_color * RECIPROCAL_PI * fss * NdotL * NdotV;
			}

			if (u_diffuse_type == 2)
			{
				f_diff = material.diffuse_color * RECIPROCAL_PI;
			}

			//Specular component
			vec3 f_specular = (F * G * D) / (4.0 * NdotL * NdotV + 1e-6);	
			
			//Result
			direct_light += (f_diff + f_specular) * gamma_to_linear(u_light_color[i]) * u_light_intensity[i] * NdotL; 
		}
		else
			break;
	}

	return direct_light;
}

vec3 getPixelColor(PBRMat material, vec3 V, float NdotV)
{
	//compute IBL
	vec3 color = computeIndirectLight(material, V, NdotV);

	//Compute direct light
	color = computeDirectLight(material, V, NdotV);

	//Add emissions
	color += material.emission;

	return color;
}


void main()
{
	//Create Material
	PBRMat material;
	
	//View vector and dot product with the normal
	vec3 V = normalize(u_camera_position - v_world_position);

	//Fill Material
	getMaterialProperties(material, V);

	//Dot product with the normal
	float NdotV = clamp(dot(material.N,V),0.0, 1.0);

	//Get pixel color
	vec3 color = getPixelColor(material, V, NdotV);

	//Apply Tonemapping
	color = toneMapUncharted(color);

	//Debugger albedo
	if (u_output == 1)
		color = vec4(material.base_color, 1.0);

	// Last step: to gamma space
	color = linear_to_gamma(color);
;
	//Not color debugger outputs
	if (u_output == 2)
		color = vec4(vec3(material.roughness), 1.0);
	if (u_output == 3)
		color = vec4(vec3(material.metallic), 1.0);
	if (u_output == 4)
		color = vec4(material.N, 1.0);

	//Output color
	gl_FragColor = vec4(color, u_color.w * material.opacity);
}