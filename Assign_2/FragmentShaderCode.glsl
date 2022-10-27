#version 460

out vec4 daColor;

in vec3 theColor;
in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform sampler2D myTextureSampler;
uniform vec4 ambientLight;
uniform vec3 lightPositionWorld;
uniform vec3 eyePositionWorld;
uniform float bright;

uniform vec3 spotSourcePosition;
uniform vec3 spotLightDir;
uniform int flash;
void main()
{
	float cutOff = 0.992f;
	vec4 spotLightColor =	  vec4( 1.0, 0.0, 0.0, 0);
	vec4 diffuseLightColor  = vec4( 1.0, 1.0, 1.0, 0);
	vec4 specularLightColor = vec4( 1.0, 1.0, 1.0, 0); 

	vec3 lightVectorWorld = normalize(-lightPositionWorld ); //direction light
	float diff  = clamp(dot(lightVectorWorld, normalize(normalWorld)),0,1);

	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld, normalize(normalWorld));
	float specBright = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld),0,1);
	specBright = pow(specBright,50);

	vec3 lightDir = normalize(vertexPositionWorld - spotSourcePosition);
	float theta = clamp(dot(lightDir, normalize(spotLightDir)),0,1);

	vec3 spotDifVec = normalize(spotSourcePosition);
	float spotDiff = clamp(dot(spotDifVec, normalize(normalWorld)),0,1);

	vec3 spotReflect = reflect(lightDir , normalize(normalWorld));
	float spotSpec = clamp(dot(spotReflect, eyeVectorWorld),0,1);
	spotSpec = pow(spotSpec,50);

	vec4 MaterialColor =  vec4(texture( myTextureSampler, UV ).rgb, 1.0f);

	if(theta > cutOff)
	{
		if(bright<0)
		{
			if(flash == 0)
			{
				daColor =  MaterialColor * spotLightColor * spotDiff  +
					       specularLightColor * spotSpec ;
			}
			else
			{
				daColor =  MaterialColor * spotLightColor * bright;
			}
		}
		else
		{
			daColor =  MaterialColor * spotLightColor * spotDiff  +
					   specularLightColor * spotSpec +

					  MaterialColor * ambientLight +
					  MaterialColor * diffuseLightColor * diff * bright +
			          specularLightColor * specBright * bright;
		}
	}
	else
	{
		if(bright<0)
		{
			daColor = MaterialColor * ambientLight +
					  MaterialColor * diffuseLightColor * bright +
			          specularLightColor * specBright * bright;
		}
		else
		{
			daColor = MaterialColor * ambientLight +
					  MaterialColor * diffuseLightColor * diff * bright +
			          specularLightColor * specBright * bright;
		}
	}
}