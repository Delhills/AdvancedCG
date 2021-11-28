const int MAX_ITERATIONS = 100000;

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler3D u_texture;
uniform sampler2D u_noise_texture;
uniform sampler2D u_texture_lut;

uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform vec4 u_clipping_plane;

uniform mat4 u_model;
uniform mat4 u_inv_model;

uniform float u_step_length;
uniform float u_threshold;

uniform vec3 u_light_ambient;
uniform vec3 u_light_position;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;
uniform vec3 u_material_specular;
uniform float u_material_shininess;

uniform bool u_jittering;
uniform bool u_transfer_function;
uniform bool u_clipping_plane_check;

uniform float u_h;
uniform bool u_normals;

void main(){

	// 1. Ray setup
	vec3 camera_localpos = (u_inv_model * vec4( u_camera_position, 1.0) ).xyz; //Transform to local coordinates
	vec3 ray_dir = normalize(v_position - camera_localpos);

	vec3 stepVector = ray_dir * u_step_length;
	vec3 samplePos = v_position; 

	// Add jittering
	if (u_jittering)
	{
		vec2 uv_noise = vec2(gl_FragCoord.xy / 128.0);
		vec3 noise_value = texture2D(u_noise_texture, uv_noise).xyz;
		samplePos += stepVector * noise_value;
	}

	// Define final color
	vec4 finalColor = vec4(0.0);

	// Ray loop
	for(int i = 0; i < MAX_ITERATIONS; i++)
	{
		// Check clipping plane	
		bool compute = true;
		if (u_clipping_plane_check)
			compute = (u_clipping_plane.x * samplePos.x + u_clipping_plane.y * samplePos.y + u_clipping_plane.z * samplePos.z + u_clipping_plane.w <= 0);

		if (compute)
		{
			// 2. Volume sampling
			vec3 text_coords = (samplePos + 1.0) / 2.0;
			float d = texture3D(u_texture, text_coords).x;

			// 3. Classification and shading
			if (d > u_threshold)
			{
				// Define normal
				vec3 grad = vec3(0.0);

				//Compute gradient
				vec3 text_coords1_x = (vec3(samplePos.x + u_h, samplePos.y, samplePos.z) + 1.0) / 2.0;
				float dx1 = texture3D(u_texture, text_coords1_x).x;

				vec3 text_coords2_x = (vec3(samplePos.x - u_h, samplePos.y, samplePos.z) + 1.0) / 2.0;
				float dx2 = texture3D(u_texture, text_coords2_x).x;

				vec3 text_coords1_y = (vec3(samplePos.x, samplePos.y + u_h, samplePos.z) + 1.0) / 2.0;
				float dy1 = texture3D(u_texture, text_coords1_y).x;

				vec3 text_coords2_y = (vec3(samplePos.x, samplePos.y - u_h, samplePos.z) + 1.0) / 2.0;
				float dy2 = texture3D(u_texture, text_coords2_y).x;

				vec3 text_coords1_z = (vec3(samplePos.x, samplePos.y, samplePos.z + u_h) + 1.0) / 2.0;
				float dz1 = texture3D(u_texture, text_coords1_z).x;

				vec3 text_coords2_z = (vec3(samplePos.x, samplePos.y, samplePos.z - u_h) + 1.0) / 2.0;
				float dz2 = texture3D(u_texture, text_coords2_z).x;

				//Applying the negative of the gradient to get the normals
				grad.x = 1.0 / (2.0 * u_h) * (dx2 - dx1);
				grad.y = 1.0 / (2.0 * u_h) * (dy2 - dy1);
				grad.z = 1.0 / (2.0 * u_h) * (dz2 - dz1);
				grad = normalize(grad); //Normalize as the normals

				//Visualize normals
				if (u_normals)
				{
					//assign the normals to the output
					finalColor = vec4(grad, 1.0);
					break;
				}

				//PHONG
				vec4 color;
				if (u_transfer_function) //Apply transfer function
					{ color = texture2D(u_texture_lut, vec2(d, 0.5)); }
				else
					color = u_color; //Simple shading

				if (color.w == 1.0)
				{
					//Defining the world position at the sample position
					vec3 world_position = (u_model * vec4( samplePos, 1.0) ).xyz;

					//PHONG computations
					vec3 Ka = u_material_ambient * color.xyz;
					vec3 Kd = u_material_diffuse * color.xyz;
					vec3 Ks = u_material_specular * color.xyz * color.w;

					vec3 KaIa = u_light_ambient * Ka;

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
					break;
				}
			}
		}
		
		// 4. Next sample
		samplePos += stepVector;

		// 5. Early termination
		if (finalColor.w >= 1.0 || (samplePos.x < -1.0 || samplePos.x > 1.0) || (samplePos.y < -1.0 || samplePos.y > 1.0) || (samplePos.z < -1.0 || samplePos.z > 1.0))
			break;
}

	//6. Final color
	gl_FragColor = finalColor;
}