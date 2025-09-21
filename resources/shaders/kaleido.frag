#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;
uniform float u_energy;

vec2 rotate(vec2 v, float a)
{
    float c = cos(a);
    float s = sin(a);
    return vec2(c * v.x - s * v.y, s * v.x + c * v.y);
}

float petal(vec2 uv, float petals, float power)
{
    float angle = atan(uv.y, uv.x);
    float radius = length(uv);
    float wave = cos(angle * petals) * 0.5 + 0.5;
    return pow(wave, power) * exp(-radius * 2.0);
}

float tunnel(vec2 uv)
{
    float r = length(uv);
    return sin(r * 6.0 - u_time * 6.0) * 0.5 + 0.5;
}

void main()
{
    vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec2 mouseNorm = (u_mouse * 2.0 - u_resolution.xy) / u_resolution.y;

    float mouseInfluence = length(mouseNorm);
    float energy = mix(-1.0, 1.0, u_energy * 0.5 + 0.5);

    float baseRotation = u_time * 0.25 + mouseInfluence * 2.0;
    uv = rotate(uv, baseRotation);

    float bloom = 0.0;
    for (int i = 0; i < 6; ++i)
    {
        float petals = 4.0 + float(i) * 1.5 + mouseInfluence * 5.0;
        float power = 1.5 + sin(u_time + float(i)) * 0.5 + mouseInfluence;
        bloom += petal(uv, petals, power);
        uv = rotate(uv, 3.14159 / 6.0);
    }

    vec2 tunnelUV = rotate(uv, u_time * 0.5);
    float vortex = tunnel(tunnelUV * (1.5 + mouseInfluence * 0.5));

    float glow = bloom * 0.6 + vortex * 0.4;
    float pulse = sin(u_time * 3.0 + mouseInfluence * 5.0) * 0.5 + 0.5;
    float halo = exp(-dot(uv, uv) * 3.0) * (0.5 + pulse * 0.5);

    vec3 baseColor = vec3(0.05, 0.1, 0.2);
    vec3 neonA = vec3(0.3, 0.7, 1.0);
    vec3 neonB = vec3(1.0, 0.2, 0.8);
    vec3 neonC = vec3(0.2, 1.0, 0.5);

    float colorMix = fract(glow * 0.4 + u_time * 0.1 + mouseInfluence * 0.2);
    vec3 palette = mix(neonA, neonB, smoothstep(0.0, 0.5, colorMix));
    palette = mix(palette, neonC, smoothstep(0.5, 1.0, colorMix));

    vec3 color = baseColor;
    color += palette * glow * (0.7 + halo * 0.5);
    color += vec3(0.1, 0.05, 0.15) * halo * (1.0 + energy * 0.3);

    float vignette = smoothstep(1.2, 0.3, length(uv));
    color *= vignette;

    gl_FragColor = vec4(color, 1.0);
}
