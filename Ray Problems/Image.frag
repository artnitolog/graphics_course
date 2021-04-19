void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Accumulation: divide by number of frames
    fragColor = texture(iChannel0, uv) / float(iFrame + 1);
}
