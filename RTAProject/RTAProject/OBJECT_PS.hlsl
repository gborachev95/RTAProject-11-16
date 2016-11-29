struct INPUT_PIXEL
{
	float4 projectedCoordinate : SV_POSITION;
	float3 normals             : NORMALS;
	float2 uv                  : UV;
	float3 tangents            : TANGENTS;
	float3 bitangents          : BITANGENTS;
	float3 shine               : SHINE;
	// Not part of the structure
	float3 worldPosition       : WORLDPOS;
	float3 cameraPosition      : CAMERA_POS;
};
texture2D baseTexture : register(t0);

texture2D normalTexture : register(t1);

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

float4 main(INPUT_PIXEL _inputPixel) : SV_TARGET
{
	float4 currColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 dirLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 pointLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 spotLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// Getting the texture color
	currColor = baseTexture.Sample(filter, _inputPixel.uv.xy);
	// Normalize stuff
	_inputPixel.normals.xyz = normalize(_inputPixel.normals.xyz);
	_inputPixel.tangents.xyz = normalize(_inputPixel.tangents.xyz);
	_inputPixel.bitangents.xyz = normalize(_inputPixel.bitangents.xyz);

	// Normal mapping
	// Getting the normal map
	float4 normalMap = normalTexture.Sample(filter, _inputPixel.uv.xy);
	normalMap = (normalMap * 2.0f) - 1.0f;
	// Getting the normal from the normal map.
	float3 bumpNormal = (normalMap.x * _inputPixel.tangents.xyz) + (normalMap.y * _inputPixel.bitangents.xyz) + (normalMap.z * _inputPixel.normals.xyz);
	// Normalize the resulting bump normal.
	bumpNormal = normalize(bumpNormal);

	// Directional light
	if (dir_light.status)
	{
		float lightRatio = saturate(dot(-dir_light.direction, bumpNormal.xyz));
		dirLightColor = lightRatio * dir_light.color * currColor;
		// Specular effect
		float3 cameraDir = normalize(_inputPixel.cameraPosition - _inputPixel.worldPosition);
		float3 reflectionVec = normalize(reflect(dir_light.direction, bumpNormal.xyz));
		float specular = pow(saturate(dot(reflectionVec, cameraDir)), 1000) * _inputPixel.shine.x;
		dirLightColor += saturate(currColor * 0.5f + specular);
	}
	// Point light
	if (point_light.status)
	{
		float3 lightDir = normalize(point_light.transform.xyz - _inputPixel.worldPosition.xyz);
		float lightRatio = saturate(dot(lightDir.xyz, bumpNormal.xyz));
		lightRatio *= 1 / dot(point_light.transform.xyz - _inputPixel.worldPosition.xyz, point_light.transform.xyz - _inputPixel.worldPosition.xyz);
		// Specular effect
		float3 cameraDir = normalize(_inputPixel.cameraPosition - _inputPixel.worldPosition);
		float3 reflectionVec = normalize(reflect(-lightDir.xyz, bumpNormal.xyz));
		float specular = pow(saturate(dot(reflectionVec, cameraDir)), 90) * _inputPixel.shine.x;
		pointLightColor = saturate(lightRatio * point_light.color * currColor + (lightRatio*specular));
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
		float aten = 1 - saturate((len / 5.0f));
		if (surRatio > spot_light.radius)
			spotFactor = 2.0f* aten;
		else
			spotFactor = aten * 0.7f;
		float lightRatio = saturate(dot(lightDir.xyz, bumpNormal.xyz));
		// Specular effect
		float3 cameraDir = normalize(_inputPixel.cameraPosition - _inputPixel.worldPosition);
		float3 reflectionVec = normalize(reflect(-lightDir.xyz, bumpNormal.xyz));
		float specular = pow(saturate(dot(cameraDir,reflectionVec)), 10) * _inputPixel.shine.x;
		spotLightColor = saturate(/*spotFactor * lightRatio * */aten*  spot_light.color * currColor + (aten* specular));
	}

	return saturate(dirLightColor + spotLightColor);
}
