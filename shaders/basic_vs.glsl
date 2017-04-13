#version 330
layout( location = 0 ) in vec3 vert_modelSpace;
//layout( location = 0 ) in vec2 vert_modelSpace;
//layout( location = 1 ) in vec3 color;
//layout( location = 2 ) in vec2 offset;


uniform mat4 MVP;
uniform vec3 inputColor;

out vec3 interpolateColor;

void main()
{
		gl_Position = MVP * vec4( vert_modelSpace, 1.0 );
//	gl_Position = vec4( vert_modelSpace + offset, 0.0f, 1.0f );
		interpolateColor = inputColor;
//	interpolateColor = color;
}
