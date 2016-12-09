#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 normals    : NORMALS;
	float3 uv         : UV;
	float3 tangents   : TANGENTS;
	float3 bitangents : BITANGENTS;
	float3 shine      : SHINE;
	//float4 indices : SKIN_INDICES;
	//float4 weights : SKIN_WEIGHTS;
};
struct OUTPUT_VERTEX
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

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);

	// Shading
	localCoordinate = mul(localCoordinate, worldMatrix);
	sendToRasterizer.worldPosition = localCoordinate.xyz;
	float3 worldNormals = mul(float4(fromVertexBuffer.normals.xyz, 0), worldMatrix).xyz;
	localCoordinate = mul(localCoordinate, viewMatrix);
	localCoordinate = mul(localCoordinate, projectionMatrix);

	// Tangents and Bitangents
	sendToRasterizer.tangents = mul(fromVertexBuffer.tangents, (float3x3)worldMatrix);
	sendToRasterizer.bitangents = mul(fromVertexBuffer.bitangents, (float3x3)worldMatrix);

	// Sending data
	sendToRasterizer.projectedCoordinate = localCoordinate;
	sendToRasterizer.normals = worldNormals;
	sendToRasterizer.uv = fromVertexBuffer.uv.xy;
	sendToRasterizer.cameraPosition.xyz = cameraPosition.xyz;

	sendToRasterizer.shine = fromVertexBuffer.shine;

	return sendToRasterizer;
}
