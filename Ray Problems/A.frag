const float INF = 1e10;
const float EPS = 1e-4;
const vec3 CAMERA_POS = vec3(2, 0, -8);

const int EMISSION = 0;
const int DIFFUSE = 1;
const int REFRACTION = 2;
const int REFLECTION = 3;
const int VOLUME = 4;

struct Material {
    int base_type;
    int alt_type;
    float proba_alt;
};

// deterministic
const Material DetMats[] = Material[] (
    Material(EMISSION, EMISSION, 0.0),
    Material(DIFFUSE, DIFFUSE, 0.0),
    Material(REFRACTION, REFRACTION, 0.0),
    Material(REFLECTION, REFLECTION, 0.0),
    Material(VOLUME, VOLUME, 0.0)
);

const float ETA_AIR = 1.0;
const float ETA_CRYSTAL = 1.4;
const float ETA_RATIOS[2] = float[2](ETA_AIR/ETA_CRYSTAL, ETA_CRYSTAL/ETA_AIR);
const float CRYSTAL_R = (ETA_CRYSTAL - ETA_AIR) * (ETA_CRYSTAL - ETA_AIR) / (ETA_CRYSTAL + ETA_AIR) / (ETA_CRYSTAL + ETA_AIR);

struct Light {
    vec3 pos;
    float intensity;
    vec4 color;
};

struct Hit {
    float t;
    vec3 worldPos;
    vec3 normal;
    Material material;
    vec4 color;
};

struct Sphere {
    vec3 center;
    float r;
    Material material;
    vec4 color;
};

const Light lights[] = Light[](
    Light(vec3(3, 1, 1), 10.0, vec4(0.78125, 0.8863, 1.0, 1)),
    Light(vec3(-1, -0.3, -4), 5.0, vec4(1, 0.95, 0.78, 1)),
    //Light(vec3(-3, -0.5, 0), 10.0, vec4(1, 0.95, 0.78, 1)),
    Light(vec3(0, 0.0, 0), 10.0, vec4(1.0,0.9,0.5,1))
);

const int n_lights = lights.length();

// no it doesn't help here
const vec4 AMBIENT = vec4(0);//vec4(0.1, 0.01, 0.1, 1.0);

const Sphere spheres[] = Sphere[](
    Sphere(lights[0].pos, 0.4, DetMats[EMISSION], lights[0].color),
    Sphere(lights[1].pos, 0.4, DetMats[EMISSION], lights[1].color)
    //Sphere(vec3(1.5,0,4), 1.0, DetMats[DIFFUSE], vec4(1,0,0,1)),
    //Sphere(vec3(1.2,0,0), 0.4, DetMats[REFRACTION], vec4(0,0,1,1)),
    //Sphere(vec3(-1.2,0,0), 0.4, DetMats[REFLECTION], vec4(0,0,1,1)),
    //Sphere(vec3(2, -0.2, 0.1), 0.1, DetMats[EMISSION], vec4(1,1,0,1)),
    //Sphere(vec3(-2, -0.9, 0.1), 0.1, DetMats[EMISSION], vec4(1,0,1,1))
);

const int n_spheres = spheres.length();

// this mesh contains a lot of duplicates but it's still small
const vec3 vee_pyramid[] = vec3[](
    vec3(0.0, 1.3, 0.0), vec3(-1.0, -2.3, -1.0), vec3(-1.0, -2.3, 1.0), 
    vec3(0.0, 1.3, 0.0), vec3(-1.0, -2.3, 1.0), vec3(1.0, -2.3, 1.0), 
    vec3(0.0, 1.3, 0.0), vec3(1.0, -2.3, 1.0), vec3(1.0, -2.3, -1.0), 
    vec3(0.0, 1.3, 0.0), vec3(1.0, -2.3, -1.0), vec3(-1.0, -2.3, -1.0), 
    vec3(-1.0, -1.0, -1.0), vec3(2.0, 0.0, 0.0), vec3(0.0, 0.0, 2.0), 
    vec3(1.0, -1.0, 1.0), vec3(-2.0, 0.0, 0.0), vec3(0.0, 0.0, -2.0)
);


const Material PyrMaterial = Material(REFRACTION, REFLECTION, CRYSTAL_R * 10.0);
const vec4 PyrColor = vec4(1, 1, 0.9, 1);

const Material FloorMat = Material(DIFFUSE, REFLECTION, 0.5);
const float FloorPos = -1.1;

const vec3 FirePos = vec3(0,0.0,0);
const float ShellRadius = 0.5;

const int N_VOLUME_STEPS = 200;
const float VOLUME_DENSITY = 0.25;
const float STEP_SIZE = 0.014;


float rand(int frame) {
    return fract(sin(dot(vec3(frame), vec3(12.9898,78.233,45.5432))) * 43758.5453);
}

int whichMaterial(Material mat, float rv) {
    if (mat.base_type == mat.alt_type) {
        return mat.base_type;
    }
    if (rv < mat.proba_alt) {
        return mat.alt_type;
    } else {
        return mat.base_type;
    }
}

void traceFloor(vec3 pos, vec3 dir, inout Hit hit) {
    float t = (FloorPos - pos.y) / dir.y;
    if (t <= 0.0 || t > hit.t) {
        return;
    }
    vec3 worldPos = pos + t * dir;
    if (dot(worldPos.xz, worldPos.xz) > 50.0) {
        return;
    }
    hit = Hit(t, worldPos, vec3(0, 1, 0), FloorMat,
              texture(iChannel0, 0.1 * worldPos.xz));
}


void tracePedestal(vec3 pos, vec3 dir, inout Hit hit) {
    const float PED_SQR = 6.0;
    float t = (-1.0 - pos.y) / dir.y;
    if (t <= 0.0) {
        return;
    }
    vec3 worldPos = pos + dir * t;
    if (dot(worldPos.xz, worldPos.xz) < PED_SQR && t < hit.t) {
        hit = Hit(t, worldPos, vec3(0, 1, 0), DetMats[DIFFUSE], texture(iChannel2, worldPos.xz * worldPos.y));
        //return;
    }
    float k = dot(pos.xz, dir.xz);
    float a = dot(dir.xz, dir.xz);
    float D1 = k * k - (dot(pos.xz, pos.xz) - PED_SQR) * a;
    if (D1 < 0.0) {
        return;
    }
    t = (-k - sqrt(D1)) / a;
    worldPos = pos + t * dir;
    if (t < 0.0 || worldPos.y > -1.0 || t > hit.t) {
        return;
    }
    vec3 normal = normalize(vec3(worldPos.x, 0, worldPos.z));
    hit = Hit(t, worldPos, normal, DetMats[DIFFUSE], texture(iChannel2, worldPos.xz * worldPos.y));
}

void traceSphere(vec3 pos, vec3 dir, Sphere S, inout Hit hit) {
    vec3 cpos = pos - S.center;
    float k = dot(cpos, dir);
    float D1 = k * k - dot(cpos, cpos) + S.r * S.r;
    if (D1 < 0.0) {
        return;
    }
    float t = -k - sqrt(D1);
    if (t < 0.0) {
        t = -k + sqrt(D1);
        if (t < 0.0) {
            return;
        }
    };
    if (t > hit.t) {
        return;
    }
    vec3 normal = normalize(cpos + t * dir);
    hit = Hit(t, pos + t * dir, normal, S.material, S.color);
}


vec3 traceTriangle(vec3 pos, vec3 dir, vec3 v0, vec3 e1, vec3 e2) {
    vec3 T = pos - v0;
    vec3 P = cross(dir, e2);
    float det = dot(P, e1);
    vec3 Q = cross(T, e1);
    float t = dot(Q, e2) / det;
    if (t < 0.0) {
        return vec3(INF);
    }
    float u = dot(P, T) / det;
    float v = dot(Q, dir) / det;
    if (u < 0.0 || v < 0.0 || u + v > 1.0) {
        return vec3(INF);
    }
    return vec3(t, u, v);
}

void tracePyramid(vec3 pos, vec3 dir, inout Hit hit) {
    vec3 tuv = vec3(INF);
    int idx = -1;
    for (int i = 0; i < vee_pyramid.length(); i += 3) {
        vec3 cur_tuv = traceTriangle(pos, dir, vee_pyramid[i], vee_pyramid[i + 1], vee_pyramid[i + 2]);
        if (cur_tuv.x < tuv.x) {
            tuv = cur_tuv;
            idx = i;
        }
    }
    if (tuv.x > hit.t) {
        return;
    }
    vec3 norm = normalize(cross(vee_pyramid[idx + 1], vee_pyramid[idx + 2]));
    hit = Hit(tuv.x, pos + tuv.x * dir, norm, PyrMaterial, PyrColor);
}

bool isOccluded(vec3 pos, vec3 target) {
    vec3 dir = normalize(target - pos);
    Hit hit;
    hit.t = INF;
    /*tracePyramid(pos, dir, hit);
    if (hit.t != INF) {
        return true;
    }*/
    tracePedestal(pos, dir, hit);
    if (hit.t != INF) {
        return true;
    }
    return false;
}

vec4 computeLight(vec3 pos, vec3 normal, vec4 color) {
    vec4 diffuse = vec4(0);
    for (int i = 0; i < n_lights; ++i) {
        vec3 toLight = lights[i].pos - pos;
        float attSq = isOccluded(pos + EPS * normal, lights[i].pos + 0.05 * lights[i].intensity * vec3(rand(-3 * iFrame), rand(-3 * iFrame + 1), rand(-3 * iFrame + 2))) ? 0.0 : lights[i].intensity / dot(toLight, toLight);
        diffuse += lights[i].color * max(0.0, dot(normal, normalize(toLight))) * attSq;
    }
    return color * diffuse + AMBIENT;
}

vec3 hmRefract(vec3 dir, vec3 normal, inout int inside) {
    if (dot(dir, normal) < 0.0) {
        normal = -normal;
    }
    //inside = 1 - inside; LOOK OUT
    float cosA = dot(dir, normal);
    float sinA = sqrt(1.0 - cosA * cosA);
    vec3 tang = normalize(dir - cosA * normal);
    float sinB = sinA * ETA_RATIOS[inside];
    if (sinB > 1.0) {
        // Total internal reflection
        
        return reflect(dir, normal);
        
    }
    inside = 1 - inside;
    float cosB = sqrt(1.0 - sinB * sinB);
    return sinB * tang + cosB * normal;
}

// Original idea:
// https://www.shadertoy.com/view/4ssGzn
float noise(in vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f * f * f * (3.0-2.0*f);
	vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
	vec2 rg = texture(iChannel3, (uv+0.5)/256.0, -100.0).xz;
	return mix(rg.x, rg.y, f.z) * 2.0 - 1.0;
}

float fbm(vec3 p) {
    float f = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        f += noise(p) * amp;
        p *= 2.03;
        amp *= 0.5;
	}
    return f;
}
const float NOISE_FREQ = 1.5;
const float NOISE_AMP = 2.0;

float distanceFunc(vec3 p) {
    p *= 6.0;
    float d = length(p) - 1.0;
	d += fbm(p * NOISE_FREQ) * NOISE_AMP;
	return d;
}

vec4 shade(float d) {	
    if (d >= 0.0 && d < 0.2) return (mix(vec4(3, 3, 3, 1), vec4(1, 1, 0, 1), d / 0.2));
	if (d >= 0.2 && d < 0.4) return (mix(vec4(1, 1, 0, 1), vec4(1, 0, 0, 1), (d - 0.2) / 0.2));
	if (d >= 0.4 && d < 0.5) return (mix(vec4(1, 0, 0, 1), vec4(0, 0, 0, 0), (d - 0.4) / 0.2));    
    if (d >= 0.5 && d < 0.8) return (mix(vec4(0, 0, 0, 0), vec4(0, .5, 1, 0.2), (d - 0.6) / 0.2));
    if (d >= 0.8 && d < 1.2) return (mix(vec4(0, .5, 1, .2), vec4(0, 0, 0, 0), (d - 0.8) / 0.2));            
    return vec4(0.0, 0.0, 0.0, 0.0);
}

vec4 volumeFunc(vec3 p) {
	float d = distanceFunc(p);
	return shade(d);
}

vec4 rayMarch(vec3 pos, vec3 ray_step) {
    vec4 sum = vec4(0, 0, 0, 0);
	for (int i = 0; i < N_VOLUME_STEPS; i++) {
		vec4 col = volumeFunc(pos);
		col.a *= VOLUME_DENSITY;
		col.rgb *= col.a;
		sum += col*(1.0 - sum.a);
        pos += ray_step;
	}
	return sum * 0.9;
}

void traceFire(vec3 pos, vec3 dir, inout Hit hit) {
    vec3 cpos = pos - FirePos;
    float k = dot(cpos, dir);
    float D1 = k * k - dot(cpos, cpos) + ShellRadius * ShellRadius;
    if (D1 < 0.0) {
        return;
    }
    float t1 = -k - sqrt(D1);
    float t2 = -k + sqrt(D1);
    if (t1 < 0.0 || t1 > hit.t) {
        return;
    }
    vec3 start = pos + dir * t1;
    vec4 color = rayMarch(start, dir * STEP_SIZE);
    hit = Hit(t1, pos + dir * t2, vec3(0), DetMats[VOLUME], color);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 rvs = vec3(rand(3 * iFrame), rand(3 * iFrame + 1), rand(3 * iFrame + 2));
    vec2 AA_shift = 2.0 * rvs.xy - 1.0;
    vec2 uv = (fragCoord - iResolution.xy * 0.5 + AA_shift) / iResolution.x;
    vec3 front = normalize(-CAMERA_POS);
    vec3 up = vec3(0, 1, 0);
    vec3 right = normalize(cross(front, up));
    up = normalize(cross(right, front));
    vec3 viewVec = normalize(front + right * uv.x + up * uv.y);
    
    
    vec3 pos = CAMERA_POS;
    vec3 dir = viewVec;
    
    int inside = 0;
    
    fragColor = vec4(0, 0, 0, 0);
    
    for (int i = 0; i < 30; ++i) {
        Hit hit = Hit(INF, vec3(0), vec3(0), DetMats[0], vec4(0));
        traceFloor(pos, dir, hit);
        tracePedestal(pos, dir, hit);
        for (int j = 0; j < n_spheres; ++j) {
            Sphere S = spheres[j];
            if (S.material.base_type == EMISSION) {
                vec3 toCenter = -normalize(S.center) * rvs.x * 0.4;
                S.center += rvs * 0.2;
            }
            traceSphere(pos, dir, S, hit);
        }
        tracePyramid(pos, dir, hit);
        traceFire(pos, dir, hit);
        if (hit.t != INF) {
            int material_type = whichMaterial(hit.material, rvs.x);
            if (material_type == EMISSION) {
                fragColor = hit.color;
                break;
            } else if (material_type == DIFFUSE) {
                vec4 color = computeLight(hit.worldPos, hit.normal, hit.color);
                fragColor.rgb += color.rgb * (1.0 - fragColor.a);
                break;
            } else if (material_type == REFLECTION) {
                pos = hit.worldPos + hit.normal * EPS;
                dir = reflect(dir, hit.normal);
            } else if (material_type == REFRACTION) {
                dir = hmRefract(dir, hit.normal, inside);
                pos = hit.worldPos + dir * EPS;
            } else if (material_type == VOLUME) {
                pos = hit.worldPos + dir * EPS;
                fragColor += hit.color * (1.0 - fragColor.a);
            }
        } else {
            vec4 color = texture(iChannel1, dir);
            fragColor += color * (1.0 - fragColor.a);
            break;
        }
    }
    
} 
