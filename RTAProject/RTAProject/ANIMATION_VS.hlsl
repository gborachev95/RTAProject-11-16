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
	float4x4 boneOffset[28];
	float4x4 positionOffset;
}


OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);

	// Local coordinate with smooth skinning  
	// The world position is animated at world origin
	float4 animatedWorld = mul(localCoordinate, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Smooth skinning normals
	//float4 animatedNormal = mul(float4(fromVertexBuffer.normals.xyz, 0), boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x;

	// Local coordinate in world space
	sendToRasterizer.worldPosition = mul(animatedWorld, worldMatrix).xyz;
	//sendToRasterizer.worldPosition = localCoordinate.xyz;

	// Local coodrinate in projection space
	sendToRasterizer.projectedCoordinate = mul(animatedWorld, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);

	// Final coordinate
	//sendToRasterizer.projectedCoordinate = localCoordinate;

	// Normals in world space
	sendToRasterizer.normals = mul(float4(fromVertexBuffer.normals.xyz, 0), worldMatrix).xyz;
	sendToRasterizer.uv.xyz = fromVertexBuffer.uv.xyz;

	// Tangends and bitangents in worldspace
	sendToRasterizer.tangents = mul(fromVertexBuffer.tangents, worldMatrix).xyz;
	sendToRasterizer.bitangents = mul(fromVertexBuffer.bitangents, worldMatrix).xyz;

	// Sending data to Rasterizer
	return sendToRasterizer;
}
