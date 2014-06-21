//[Vertex_Shader]

// this statement is read by kgliv ONLY, see it's code.
// can be a comment, but please no blanks around the "="
// NUMPOLY=1

uniform const float time;

void main(void)
{
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_Position = ftransform();
}
