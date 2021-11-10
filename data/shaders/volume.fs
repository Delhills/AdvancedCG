const int MAX_ITERATIONS = 100000;

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler3D u_texture;

uniform mat4 u_inv_model;
uniform mat4 u_inv_viewprojection;
uniform vec3 u_camera_position;
uniform vec4 u_color;

uniform float u_intensity;
uniform float u_step_length;

void main(){
	// 1. Ray setup
	vec3 samplePos = v_position; //(u_inv_model * vec4( v_world_position, 1.0) ).xyz;
	vec3 ray_dir = normalize(v_world_position - u_camera_position);
	vec3 stepVector = ray_dir * u_step_length;
	vec4 finalColor = vec4(0.0);

	// Ray loop
	for(int i = 0; i < MAX_ITERATIONS; i++){
		// 2. Volume sampling
		vec3 text_coords = (samplePos + 1.0 / 2.0);
		float d = texture3D(u_texture, text_coords).x;

		// 3. Classification
		vec4 sampleColor = vec4(d,d,d,d);
		//vec4 sampleColor = vec4(d,1-d,0,d*d);

		// 4. Composition
		finalColor += u_step_length * (1.0 - finalColor.w) * sampleColor;

		// 5. Next sample
		samplePos += stepVector;

		// 6. Early termination
		if (finalColor.w >= 1.0 || (samplePos.x < -1.0 || samplePos.x > 1.0) || (samplePos.y < -1.0 || samplePos.y > 1.0) || (samplePos.z < -1.0 || samplePos.z > 1.0))
			break;
	}

	//7. Final color
	gl_FragColor = finalColor * u_intensity * u_color;
}