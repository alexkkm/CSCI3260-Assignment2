#version 460

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;

uniform mat4 MVP;  //projection
uniform mat4 ModelMatrix;		//transform

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

void main()
{
	UV = vertexUV;

	vec4 v = vec4(position, 1.0);
	vec4 newPosition = ModelMatrix * v;
	gl_Position = MVP * newPosition ;

	vec4 normal_temp = mat4(transpose(inverse(ModelMatrix))) * vec4(normal,1.0);
	normalWorld = normal_temp.xyz;

	vertexPositionWorld = newPosition.xyz;

}
