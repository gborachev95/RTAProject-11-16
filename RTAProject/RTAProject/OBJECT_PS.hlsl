struct INPUT_PIXEL
{
	float4 projectedCoordinate : SV_POSITION;
	float3 worldPosition       : POSITION;
	float3 normals             : NORMALS;
	float3 uv                  : UV;
	float3 tangents            : TANGENTS;
	float3 bitangents          : BITANGENTS;
};


texture2D baseTexture : register(t0);
texture2D normalTexture : register(t1);
texture2D specularTexture : register(t2);
SamplerState filter : register(s0);

struct LIGHT_DATA
{
	float3 transform;
	float3 direction;
	float  radius;
	float4 color;
	bool   status;
	bool3  padding;
};
cbuffer DIRECTION_LIGHT : register(b2)
{
	LIGHT_DATA dir_light;
};
cbuffer POINT_LIGHT : register(b3)
{
	LIGHT_DATA point_light;
};
cbuffer SPOT_LIGHT : register(b4)
{
	LIGHT_DATA spot_light;
};

// Used for specular lighting 
cbuffer SCENE : register(b5)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 cameraPosition;
}

float4 main(INPUT_PIXEL _inputPixel) : SV_TARGET
{
	float4 currColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 dirLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 pointLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 spotLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// Getting the texture color
	currColor = baseTexture.Sample(filter, _inputPixel.uv.xy);

	// Normalize stuff
	_inputPixel.normals.xyz = normalize(_inputPixel.normals.xyz);
	_inputPixel.tangents.xyz = normalize(_inputPixel.tangents.xyz);
	_inputPixel.bitangents.xyz = normalize(_inputPixel.bitangents.xyz);

	// Normal mapping
	float4 normalMap = normalTexture.Sample(filter, _inputPixel.uv.xy);
	normalMap = (normalMap * 2.0f) - 1.0f;

	// Mirror mapping
	float3 newTangent = _inputPixel.tangents.xyz;
	float3 newBitangent = _inputPixel.bitangents.xyz;
	if (_inputPixel.uv.x >= 0.985f)
	{
		newTangent *= -0.001f;
		//newBitangent *= -0.1f;
	}
	float3 bumpNormal = (normalMap.x * newTangent.xyz) + (normalMap.y * newBitangent.xyz) + (normalMap.z * _inputPixel.normals.xyz);
	bumpNormal = normalize(bumpNormal);
	
	// Specular mapping
	float4 specularMap = specularTexture.Sample(filter, _inputPixel.uv.xy);

	// Directional light
	if (dir_light.status)
	{
		float lightRatio = saturate(dot(-dir_light.direction, bumpNormal.xyz));
		dirLightColor = lightRatio * dir_light.color * currColor;
		// Specular effect
		float3 pixelToCamera = normalize(cameraPosition.xyz - _inputPixel.worldPosition.xyz);
		float3 reflectionVec = normalize(reflect(dir_light.direction.xyz, bumpNormal.xyz));
		float specularFactor = dot(pixelToCamera, reflectionVec);
		if (specularFactor > 0.0f)
			specular += pow(specularFactor, 1000) *  specularMap * _inputPixel.uv.z;
		
		dirLightColor += saturate(currColor * 0.7f);
	}

	// Spot light
	if (spot_light.status)
	{
		float spotFactor = 0;
		float3 lightDir = spot_light.transform.xyz - _inputPixel.worldPosition.xyz;
		float len = length(lightDir);
		lightDir = normalize(lightDir);
		float surRatio = saturate(dot(-lightDir.xyz, spot_light.direction));
		// Calculating attenuation
		float aten = 1 - saturate((len / 20.0f));
		if (surRatio > spot_light.radius)
			spotFactor = 2.0f* aten;
		else
			spotFactor = aten * 0.7f;
		float lightRatio = saturate(dot(lightDir.xyz, bumpNormal.xyz));
		// Specular effect
		float3 pixelToCamera = normalize(cameraPosition.xyz - _inputPixel.worldPosition.xyz);
		float3 reflectionVec = normalize(reflect(lightDir.xyz, bumpNormal.xyz));
		float specularFactor = dot(pixelToCamera, reflectionVec);
	
		if (specularFactor > 0.0f)
			specular += pow(specularFactor, 32) * specularMap * aten * _inputPixel.uv.z;
		spotLightColor = saturate(spotFactor * lightRatio * aten * spot_light.color * currColor);
	}

	return saturate(dirLightColor + spotLightColor + specular);
}
