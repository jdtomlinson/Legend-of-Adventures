#ifndef include_Model3D
#define include_Model3D
#pragma once

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "Vertex.hpp"
#include "Material.hpp"


//-----------------------------------------------------------------------------------------------
const std::string MODEL_VERTEX_SHADER_STRING = "Data/Shaders/Simple_No_Texture_330.vertex.glsl";
const std::string MODEL_FRAGMENT_SHADER_STRING = "Data/Shaders/Simple_No_Texture_330.fragment.glsl";


//-----------------------------------------------------------------------------------------------
class Model3D
{
public:
	Model3D();
	void LoadModel( const std::string& modelFileName );
	void Render();

private:
	FILE* LoadFile( const std::string& modelFileName );
	std::string GetLineFromFile( FILE* modelFile );
	Vector2 GetVector2FromLine( const std::string fileLine );
	Vector3 GetVector3FromLine( const std::string fileLine );

	unsigned int			m_vboID;
	std::vector<Vertex>		m_vertices;
	Material*				m_material;
};


#endif // include_Model3D