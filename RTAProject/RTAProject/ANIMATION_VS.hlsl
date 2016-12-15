#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 normals    : NORMALS;
	float4 uv         : UV;
	float4 tangents   : TANGENTS;
	float4 bitangents : BITANGENTS;
	float4 indices    : SKIN_INDICES;
	float4 weights    : SKIN_WEIGHT;
	
};		   
struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float3 worldPosition       : POSITION;
	float3 normals             : NORMALS;
	float3 uv                  : UV;
	float3 tangents            : TANGENTS;
	float3 bitangents          : BITANGENTS;
};

cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}

cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 cameraPosition;
}

cbuffer BONES : register(b2)
{
	float4x4 boneOffset[38];
	float4x4 positionOffset;
}


OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;

	// Local coordinate with smooth skinning  
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);

	float4 animatedWorld = mul(localCoordinate, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x; // The world position is animated at world origin
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Moving the object
	animatedWorld = mul(animatedWorld, positionOffset);

	// Smooth skinning normals
	float4 normal = float4(fromVertexBuffer.normals.xyz, 0);
	float4 animatedNormal = mul(normal, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x;
	animatedNormal += mul(normal, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedNormal += mul(normal, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedNormal += mul(normal, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Smooth skinning tangents
	float4 tangent = float4(fromVertexBuffer.tangents.xyz,0);
	float4 animatedTangent = mul(tangent, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x;
	animatedTangent += mul(tangent, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedTangent += mul(tangent, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedTangent += mul(tangent, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Smooth skinning tangents
	float3 animatedBitangent = cross(animatedNormal.xyz, animatedTangent.xyz).xyz;

	// Coordinate in world space
	sendToRasterizer.worldPosition = mul(sendToRasterizer.worldPosition.xyz, (float3x3)worldMatrix).xyz;

	// Coodrinate in projection space
	sendToRasterizer.projectedCoordinate = mul(animatedWorld, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);

	animatedNormal = mul(animatedNormal, positionOffset);
	// Normals in world space
	sendToRasterizer.normals = mul(animatedNormal, worldMatrix).xyz;
	sendToRasterizer.uv.xyz = fromVertexBuffer.uv.xyz;

	// Tangends and bitangents in worldspace
	sendToRasterizer.tangents = mul(animatedTangent,worldMatrix).xyz;
	sendToRasterizer.bitangents = mul(animatedBitangent,(float3x3)worldMatrix).xyz;

	// Sending data to Rasterizer
	return sendToRasterizer;
}
