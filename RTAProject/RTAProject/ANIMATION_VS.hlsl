#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 normals    : NORMALS;
	float3 uv         : UV;
	float3 tangents   : TANGENTS;
	float3 bitangents : BITANGENTS;
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

float4x4 BoneOffset[256] : register(b2);

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);

	// THIS HAS TO BE DONE FIRST IN ORDER TO BE PLACED IN THE CORRECT SPACE AFTER //
	/*
	I believe you will have to create a constant buffer with which you need to send all the bones to the shader at register 2.
	I am still not sure if we receive the values but give it a try.  I think that there might be some kind of missmatch stil because we are using float4s for them but idk.
	Dont forget to: Create const buffer in the const buffer function, Set it before rendering the fbx files in the render function and map and unmap it in the mapping function.
	*/ 
	// Local coordinate with smooth skinning 
	//localCoordinate =  /*BoneOffset[fromVertexBuffer.indices[0]] * */ localCoordinate * fromVertexBuffer.weights[0];
	//localCoordinate += /*BoneOffset[fromVertexBuffer.indices[1]] * */ localCoordinate * fromVertexBuffer.weights[1];
	//localCoordinate += /*BoneOffset[fromVertexBuffer.indices[2]] * */ localCoordinate * fromVertexBuffer.weights[2];
	//localCoordinate += /*BoneOffset[fromVertexBuffer.indices[3]] * */ localCoordinate * fromVertexBuffer.weights[3];

	// Local coordinate in world space
	localCoordinate = mul(localCoordinate, worldMatrix);
	sendToRasterizer.worldPosition = localCoordinate.xyz;

	// Local coodrinate in projection space
	localCoordinate = mul(localCoordinate, viewMatrix);
	localCoordinate = mul(localCoordinate, projectionMatrix);

	// Final coordinate
	sendToRasterizer.projectedCoordinate = localCoordinate;

	// Normals in world space
	sendToRasterizer.normals = mul(float4(fromVertexBuffer.normals.xyz, 0), worldMatrix).xyz;
	sendToRasterizer.uv = fromVertexBuffer.uv.xyz;

	// Tangends and bitangents in worldspace
	sendToRasterizer.tangents = mul(fromVertexBuffer.tangents, (float3x3)worldMatrix);
	sendToRasterizer.bitangents = mul(fromVertexBuffer.bitangents, (float3x3)worldMatrix);

	// Sending data to Rasterizer
	return sendToRasterizer;
}
