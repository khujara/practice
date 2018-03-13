KH_INTERN f32
normal_distribution_beckmann(f32 n_dot_h, f32 roughness) {
	//NOTE(flo): Simplified model : (tan(angle)/m)^2 = (1 - cos^2)/(cos^2*m^2)
	f32 m = roughness*roughness;
	f32 m2 = m * m;
	f32 c2 = n_dot_h * n_dot_h;
	f32 p = (c2-1.0f)/(c2*m2);
	// f32 c4 = c2 * n_dot_h;
	f32 c4 = c2 * c2;
	f32 r = m2 * c4;

	f32 safe_r = kh_safe_ratio0_f32(1.0f, r * PI32);

	f32 res = kh_exp_f32(p) * safe_r;
	return(res);
}

KH_INTERN f32
normal_distribution_GGX(f32 n_dot_h, f32 roughness) {
	f32 m = roughness*roughness;
	f32 m2 = m * m;
	f32 c2 = n_dot_h * n_dot_h;
	f32 r = ((c2 * (m2 - 1.0f)) + 1.0f);
	f32 r2 = r * r;
	f32 safe_r = kh_safe_ratio1_f32(1.0f, PI32 * r2);
	f32 res = m2 * safe_r;
	return(res);
}

/*NOTE(flo):
	Normalised Blinn-Phong : 1/4*D(h) = (m + 2)/8*PI32*kh_pow_f32(dot(n,h), m), m E [1, 8192]
	glossiness g in [0, 1] where m = kh_pow_f32(2, 13 * g);
	cf s2012 pbs farcry slides v2 page 43
*/
KH_INTERN f32 
normal_distribution_blinn_phong(f32 n_dot_h, f32 roughness) {
	f32 g = 1.0f - roughness;
	f32 m = kh_max_f32(kh_pow_f32(2.0f, 13.0f * g), 1.0f);
	f32 res = ((m+2.0f)/(2.0f*PI32))*kh_pow_f32(n_dot_h, m);
	return(res);
}

KH_INTERN f32
cosine_distribution(f32 n_dot_h, f32 roughness) {
	f32 m2 = roughness * roughness;
	// f32 res = (1.0f / (PI32 * m2 - n_dot_h)) * (n_dot_h);
	// f32 res = (1.0f / (PI32 * m2));

	// f32 chi = (n_dot_h > 0.0f) ? 1.0f : 0.0f;
	// f32 ap = 2.0f/m2 - 2.0f; 
	// f32 theta_ap = kh_pow_f32(kh_acos_f32(n_dot_h), ap);
	// f32 res = chi * ap * kh_cos_f32(theta_ap);
	f32 d0 = (1.0f/PI32)*n_dot_h*n_dot_h*m2;
	f32 res = d0;

	return(res);
}

KH_INTERN f32
geometry_schlick_GGX(f32 c, f32 k) {

	f32 safe_r = kh_safe_ratio0_f32(1.0f, (c * (1.0f - k) + k));
	f32 res = c * safe_r;
	return(res);
}

KH_INTERN f32
geometry_smith_ggx(f32 n_dot_v, f32 n_dot_l, f32 roughness) {

	// initial is k = (roughness * roughness)/2.0f, disney proposes a modification to reduce hotness by remappin
	// roughness to (roughness + 1.0f) / 2.0f before squaring do not use this in IBL
	float r = (roughness + 1.0f);
	float k = (r*r) / 8.0f;
	f32 ggx_0 = geometry_schlick_GGX(n_dot_v, k);
	f32 ggx_1 = geometry_schlick_GGX(n_dot_l, k);

	f32 res = ggx_0 * ggx_1;
	return(res);
}

KH_INTERN v3
fresnel_none_v3(f32 c, v3 f0) {
	return(f0);
}

KH_INTERN v3
fresnel_schlick_v3(f32 c, v3 f0) {
	v3 one_min_f0 = kh_vec3(1.0f) - f0;
	f32 p = kh_pow_f32(1.0f - c, 5.0f);
	v3 res = f0 + one_min_f0 * p;
	return(res);
}

KH_INTERN v3
fresnel_spherical_gaussian_v3(f32 c, v3 f0) {
	v3 one_min_f0 = kh_vec3(1.0f) - f0;
	f32 p = (-5.55473f*c-6.98316f)*c;
	f32 mul = kh_pow_f32(2.0f, p);
	v3 res = f0 + one_min_f0*mul;
	return(res);
}

KH_INTERN v3
importance_sampling_ggx(v2 sequence, v3 N, f32 roughness) {
	f32 a = roughness * roughness;

	f32 phi = 2.0f * PI32 * sequence.x;
	f32 cos_theta = kh_sqrt_f32((1.0f - sequence.y) / (1.0f + (a * a - 1.0f) * sequence.y));
	f32 sin_theta = kh_sqrt_f32(1.0f - cos_theta*cos_theta);

	f32 cos_phi = kh_cos_f32(phi);
	// f32 sin_phi = kh_sin_f32(phi);
	f32 sin_phi = kh_sqrt_f32(1.0f - cos_phi*cos_phi);

	v3 H;
	H.x = cos_phi * sin_theta;
	H.y = sin_phi * sin_theta;
	H.z = cos_theta;

	v3 up = kh_abs_f32(N.z) < 0.999f ? kh_vec3(0,0,1) : kh_vec3(1,0,0);
	v3 tangent = kh_normalize_v3(kh_cross_v3(up, N));
	v3 bitangent = kh_cross_v3(N, tangent);
	v3 res = kh_normalize_v3(tangent * H.x + bitangent * H.y + N * H.z);
	return(res);
}

typedef f32 BRDF_D(f32 n_dot_h, f32 glossiness);
typedef v3 BRDF_F(f32 c, v3 f0);
typedef f32 BRDF_G(f32 n_dot_v, f32 n_dot_l, f32 roughness);

static BRDF_D *g_brdf_d = normal_distribution_GGX;
static BRDF_F *g_brdf_f = fresnel_schlick_v3;
static BRDF_G *g_brdf_g = geometry_smith_ggx;

struct LightProperty {
	f32 attenuation;
	f32 intensity;
	v3 L;
};

KH_INTERN LightProperty
get_light_property(Light *light, v3 pos) {
	LightProperty res = {};
	if(light->type == LightType_directional) {
		res.attenuation = 1.0f;
		res.intensity = 1.0f;
		// res.L = -kh_vec3(light->tr.c0.z, light->tr.c1.z, light->tr.c2.z);
		res.L = light->dir;
	} else if(light->type == LightType_point) {
		v3 to_light = light->pos - pos;
		f32 dist = kh_length_v3(to_light);
		f32 inv_dist = kh_safe_ratio0_f32(1.0f, dist);
		res.attenuation = inv_dist * inv_dist;
		res.intensity = 1.0f;
		res.L = to_light * inv_dist;
	} else if(light->type == LightType_spot) {
		v3 to_light = light->pos - pos;
		f32 dist = kh_length_v3(to_light);
		f32 inv_dist = kh_safe_ratio0_f32(1.0f, dist);
		v3 L = to_light * inv_dist;
		f32 theta = kh_max_f32(kh_dot_v3(light->dir, L), 0.0f);
		f32 eps = light->cutoff - light->outer_cutoff;
		res.intensity = kh_clamp01_f32((theta - light->outer_cutoff) / eps);
		res.attenuation = inv_dist * inv_dist;
		res.L = L;
	}
	return(res);
}

KH_INTERN v3
cook_torrance_brdf(Light *light, v3 pos, v3 normal, v3 albedo, v3 view_pos, f32 roughness, f32 metallic) {
	LightProperty light_prop = get_light_property(light, pos);
	v3 F0 = kh_mix_v3(kh_vec3(0.04f), albedo, metallic);

	v3 N = kh_normalize_v3(normal);
	v3 V = kh_normalize_v3(view_pos - pos);
	v3 L = light_prop.L;
	v3 H = kh_normalize_v3(L + V);
	v3 radiance = light->color * light_prop.attenuation;

	f32 h_dot_v = kh_max_f32(kh_dot_v3(H, V), 0.0f);
	f32 n_dot_v = kh_max_f32(kh_dot_v3(N, V), 0.0f);
	f32 n_dot_l = kh_max_f32(kh_dot_v3(N, L), 0.0f);
	f32 n_dot_h = kh_max_f32(kh_dot_v3(N, H), 0.0f);

    radiance *= light_prop.intensity;

	f32 D = g_brdf_d(n_dot_h, roughness);
	v3 F = g_brdf_f(h_dot_v, F0);
	f32 G = g_brdf_g(n_dot_v, n_dot_l, roughness);

	v3 ks = F;
	v3 kd = kh_vec3(1.0f) - ks;
	kd *= (1.0f - metallic);

	v3 brdf_nom = D * F * G;
	// NOTE(flo): to avoid division by zero
	f32 brdf_denom = 4.0f * n_dot_v * n_dot_l + 0.001f;

	f32 safe_denom = kh_safe_ratio0_f32(1.0f, brdf_denom);
	v3 specular = brdf_nom * safe_denom;

	v3 res = kh_hadamard_v3(kd, (albedo / PI32)) + specular;
	res = kh_hadamard_v3(res, radiance) * n_dot_l;

#if 0

	f32 t1 = 1.0f;
	f32 t2 = 0.2f;

	f32 red = (D >= t1) ? 1.0f : 0.0f;
	f32 green = (D >= t2 && D < t1) ? 1.0f : 0.0f;
	f32 blue = (D < t2) ? 1.0f : 0.0f;
	v3 c = kh_vec3(red, green, blue) * n_dot_l;
	res = c;
#endif
	return(res);
}

KH_INLINE v3
blinn_phong_brdf(Light *light, v3 pos, v3 normal, v3 albedo, v3 view_pos, f32 roughness) {

	LightProperty light_prop = get_light_property(light, pos);

	v3 L = light_prop.L;
	v3 N = kh_normalize_v3(normal);
	v3 V = kh_normalize_v3(view_pos - pos);
	v3 H = kh_normalize_v3(L + V);

	float n_dot_l = kh_max_f32(kh_dot_v3(N, L), 0.0f);
	float n_dot_h = kh_max_f32(kh_dot_v3(N, H), 0.0f);

	v3 radiance = light->color * light_prop.intensity * light_prop.attenuation;
	f32 glossiness = 1.0f - roughness;
	f32 m = kh_max_f32(kh_pow_f32(2.0f, 13.0f * glossiness), 1.0f);
	f32 p = kh_pow_f32(n_dot_h, m);

	v3 specular = light->color * p;

	v3 res = kh_hadamard_v3((albedo / PI32) * n_dot_l, specular + radiance);
	return(res);
}

KH_INLINE v3
phong_brdf(Light *light, v3 pos, v3 normal, v3 albedo, v3 view_pos, f32 roughness) {
	LightProperty light_prop = get_light_property(light, pos);
	v3 L = light_prop.L;
	v3 N = kh_normalize_v3(normal);
	v3 V = kh_normalize_v3(view_pos - pos);
	v3 R = 2.0f*kh_dot_v3(L, N)*N - L;

	float n_dot_l = kh_max_f32(kh_dot_v3(N, L), 0.0f);
	float v_dot_r = kh_max_f32(kh_dot_v3(V, R), 0.0f);

	v3 radiance = light->color * light_prop.intensity * light_prop.attenuation;
	f32 glossiness = 1.0f - roughness;
	f32 m = kh_max_f32(kh_pow_f32(2.0f, 13.0f * glossiness), 1.0f);
	f32 p = kh_pow_f32(v_dot_r, kh_max_f32(m * 0.25f, 1.0f));

	v3 specular = light->color * p;

	v3 res = kh_hadamard_v3((albedo / PI32) * n_dot_l, specular + radiance);
	return(res);
}