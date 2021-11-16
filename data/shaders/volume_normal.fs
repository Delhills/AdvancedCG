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

uniform float u_intensity;
uniform float u_step_length;

void main(){
	// 1. Ray setup
	vec2 uv_noise = vec2(gl_FragCoord.xy / 128.0);
	vec3 noise_value = texture2D(u_noise_texture, uv_noise).xyz;
	vec3 ray_dir = normalize(v_world_position - u_camera_position);

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
			finalColor += u_step_length * (1.0 - finalColor.w) * sampleColor;

			// 5. Next sample
			samplePos += stepVector;

			// 6. Early termination
			if (finalColor.w >= 1.0 || (samplePos.x < -1.0 || samplePos.x > 1.0) || (samplePos.y < -1.0 || samplePos.y > 1.0) || (samplePos.z < -1.0 || samplePos.z > 1.0))
				break;
		}
		else
		{
			// 1. Next sample
			samplePos += stepVector;

			// 2. Early termination
			if (finalColor.w >= 1.0 || (samplePos.x < -1.0 || samplePos.x > 1.0) || (samplePos.y < -1.0 || samplePos.y > 1.0) || (samplePos.z < -1.0 || samplePos.z > 1.0))
				break;
		}
	}

	//7. Final color
	gl_FragColor = finalColor * u_intensity * u_color;
}