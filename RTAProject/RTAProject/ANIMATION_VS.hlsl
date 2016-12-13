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
}


OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);
	float4 test = localCoordinate;
	// THIS HAS TO BE DONE FIRST IN ORDER TO BE PLACED IN THE CORRECT SPACE AFTER //
	/*
	I believe you will have to create a constant buffer with which you need to send all the bones to the shader at register 2.
	I am still not sure if we receive the values but give it a try.  I think that there might be some kind of missmatch stil because we are using float4s for them but idk.
	Dont forget to: Create const buffer in the const buffer function, Set it before rendering the fbx files in the render function and map and unmap it in the mapping function.
	*/ 
	//localCoordinate = float4(0, 0, 0, 0);
	// Local coordinate with smooth skinning 
	if (fromVertexBuffer.indices[0] >= 0)
	{
		localCoordinate = float4(boneOffset[fromVertexBuffer.indices[0]]._41,
			boneOffset[fromVertexBuffer.indices[0]]._42,
			boneOffset[fromVertexBuffer.indices[0]]._43,
			boneOffset[fromVertexBuffer.indices[0]]._44) * fromVertexBuffer.weights[0];
	}
	if (fromVertexBuffer.indices[1] >= 0)
	{
		localCoordinate += float4(boneOffset[fromVertexBuffer.indices[1]]._41,
			boneOffset[fromVertexBuffer.indices[1]]._42,
			boneOffset[fromVertexBuffer.indices[1]]._43,
			boneOffset[fromVertexBuffer.indices[1]]._44) * fromVertexBuffer.weights[1];
	}
	if (fromVertexBuffer.indices[2] >= 0)
	{
		localCoordinate += float4(boneOffset[fromVertexBuffer.indices[2]]._41,
			boneOffset[fromVertexBuffer.indices[2]]._42,
			boneOffset[fromVertexBuffer.indices[2]]._43,
			boneOffset[fromVertexBuffer.indices[2]]._44) * fromVertexBuffer.weights[2];
	}
	if (fromVertexBuffer.indices[3] >= 0)
	{
		localCoordinate += float4(boneOffset[fromVertexBuffer.indices[3]]._41,
			boneOffset[fromVertexBuffer.indices[3]]._42,
			boneOffset[fromVertexBuffer.indices[3]]._43,
			boneOffset[fromVertexBuffer.indices[3]]._44) * fromVertexBuffer.weights[3];
	}
	localCoordinate += test;
	
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
	sendToRasterizer.uv.xyz = fromVertexBuffer.uv.xyz;

	// Tangends and bitangents in worldspace
	sendToRasterizer.tangents = mul(fromVertexBuffer.tangents, worldMatrix).xyz;
	sendToRasterizer.bitangents = mul(fromVertexBuffer.bitangents, worldMatrix).xyz;

	// Sending data to Rasterizer
	return sendToRasterizer;
}
