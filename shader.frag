// [Pixel_Shader]

uniform sampler2D tex;
// these are autoexported by KGLImageView
uniform const float brightness;
uniform const float inverted;
uniform const vec3 color; // DO NOT rely on gl_Color - it carries strange values for the non shader version
uniform const float alpha;

void main(void)
{
   vec4 texel = texture2D(tex, gl_TexCoord[0].st);
   if (inverted > 0.0)
   {
      vec3 rgb = (texel.rgb * color); rgb = vec3(1.0,1.0,1.0) - rgb;
      gl_FragColor = vec4(rgb + vec3(brightness,brightness,brightness), alpha*texel.a);
   }
   else
   {
      gl_FragColor = vec4((texel.rgb * color) + vec3(brightness,brightness,brightness), alpha*texel.a);
   }
}


