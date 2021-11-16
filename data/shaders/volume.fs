const int MAX_ITERATIONS = 100000;

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler3D u_texture;
uniform sampler2D u_noise_texture;

uniform mat4 u_inv_model;
uniform mat4 u_inv_viewprojection;
uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform vec4 u_clipping_plane;
uniform mat4 u_model;

uniform float u_intensity;
uniform float u_step_length;

uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;
uniform vec3 u_material_specular;
uniform float u_material_shininess;

void main(){
	// 1. Ray setup
	vec2 uv_noise = vec2(gl_FragCoord.xy / 128.0);
	vec3 noise_value = texture2D(u_noise_texture, uv_noise).xyz;
	vec3 ray_dir = normalize(v_position - u_camera_position);

	vec3 stepVector = ray_dir * u_step_length;
	vec3 samplePos = v_position + stepVector * noise_value;
	vec4 finalColor = vec4(0.0);

	// Ray loop
	for(int i = 0; i < MAX_ITERATIONS; i++){
		if (u_clipping_plane.x * samplePos.x + u_clipping_plane.y * samplePos.y + u_clipping_plane.z * samplePos.z + u_clipping_plane.w <= 0)
		{
			// 2. Volume sampling
			vec3 text_coords = (samplePos + 1.0) / 2.0;
			float d = texture3D(u_texture, text_coords).x;

			// 3. Classification
			vec4 sampleColor = vec4(d,d,d,d);

			// 4. Composition
			//finalColor += u_step_length * (1.0 - finalColor.w) * sampleColor;
				
			//TODO DEPENDE DE d
			//Hacer slider de d para ir viendo que representa cada d de cara al LUT (con un  rango de d+-0.1 p.e.)

			if (d > 0.1)
			{
				vec3 grad = vec3(0.0);
				float h = 0.01;

				vec3 text_coords1_x = (vec3(samplePos.x + h, samplePos.y, samplePos.z) + 1.0) / 2.0;
				float dx1 = texture3D(u_texture, text_coords1_x).x;

				vec3 text_coords2_x = (vec3(samplePos.x - h, samplePos.y, samplePos.z) + 1.0) / 2.0;
				float dx2 = texture3D(u_texture, text_coords2_x).x;

				vec3 text_coords1_y = (vec3(samplePos.x, samplePos.y + h, samplePos.z) + 1.0) / 2.0;
				float dy1 = texture3D(u_texture, text_coords1_y).x;

				vec3 text_coords2_y = (vec3(samplePos.x, samplePos.y - h, samplePos.z) + 1.0) / 2.0;
				float dy2 = texture3D(u_texture, text_coords2_y).x;

				vec3 text_coords1_z = (vec3(samplePos.x, samplePos.y, samplePos.z + h) + 1.0) / 2.0;
				float dz1 = texture3D(u_texture, text_coords1_z).x;

				vec3 text_coords2_z = (vec3(samplePos.x, samplePos.y, samplePos.z - h) + 1.0) / 2.0;
				float dz2 = texture3D(u_texture, text_coords2_z).x;

				grad.x = -1.0 / (2.0 * h) * (dx1 - dx2);
				grad.y = -1.0 / (2.0 * h) * (dy1 - dy2);
				grad.z = -1.0 / (2.0 * h) * (dz1 - dz2);
				grad = normalize(grad);

				//PHONG
				vec3 Ka = u_material_ambient * u_color.xyz;
				vec3 Kd = u_material_diffuse * u_color.xyz;
				vec3 Ks = u_material_specular * u_color.xyz * u_color.w;

				//here write the computations for PHONG.
				vec3 KaIa = u_light_ambient * Ka;
	
				//Defining the light
				vec3 world_position = (u_model * vec4( samplePos, 1.0) ).xyz;

				vec3 L = normalize(u_light_position - world_position);
				float LdotN = clamp(dot(grad, L), 0.0, 1.0);
				vec3 diff = LdotN * (Kd * u_light_diffuse);

				vec3 V = normalize(u_camera_position - world_position);
				vec3 R = reflect(-L, grad);
				float RV = pow(max(dot(V, R), 0.0), u_material_shininess);
				vec3 spec = RV * Ks * u_light_specular;

				vec3 illumination = (KaIa + diff + spec);

				//assign the color to the output
				finalColor = vec4(illumination, 1.0);

				//finalColor = vec4(clamp(normalize(grad), vec3(0.0), vec3(1.0)), 1.0);
				//finalColor = vec4((normalize(grad) + 1.0) / 2.0, 1.0);
				//finalColor = vec4(normalize(grad), 1.0);
				break;
			}
		}
		
		// 1. Next sample
		samplePos += stepVector;

		// 2. Early termination
		if (finalColor.w >= 1.0 || (samplePos.x < -1.0 || samplePos.x > 1.0) || (samplePos.y < -1.0 || samplePos.y > 1.0) || (samplePos.z < -1.0 || samplePos.z > 1.0))
			break;
}

	//7. Final color
	gl_FragColor = finalColor * u_intensity * u_color;
}