#include "Model3D.hpp"
#include "OpenGLRenderer.hpp"
#include "StringFunctions.hpp"


//-----------------------------------------------------------------------------------------------
Model3D::Model3D()
{
	
}


//-----------------------------------------------------------------------------------------------
void Model3D::LoadModel( const std::string& modelFileName )
{
	m_material = new Material( MODEL_VERTEX_SHADER_STRING, MODEL_FRAGMENT_SHADER_STRING );

	FILE* modelFile = LoadFile( modelFileName );

	std::vector<Vector3> points;
	std::vector<Vector3> normals;
	std::vector<Vector2> texCoords;

	while( !feof( modelFile ) )
	{
		std::string fileLine = GetLineFromFile( modelFile );
		if( fileLine.size() >= 2 )
		{
			if( fileLine.substr( 0, 2 ) == "v " )
			{
				points.push_back( GetVector3FromLine( fileLine ) );
			}
			else if( fileLine.substr( 0, 2 ) == "vn" )
			{
				normals.push_back( GetVector3FromLine( fileLine ) );
			}
			else if( fileLine.substr( 0, 2 ) == "vt" )
			{
				texCoords.push_back( GetVector2FromLine( fileLine ) );
			}
		}

		if( fileLine.size() >= 1 )
		{
			if( fileLine[0] == 'g' )
			{
				points.clear();
				normals.clear();
				texCoords.clear();
			}

			if( fileLine[0] == 'f' )
			{
				std::vector<std::string> faceVertices = GetVectorOfStringsFromSingleString( fileLine.substr( 2 ), ' ' );
				for( unsigned int vertIndex = 0; vertIndex < faceVertices.size(); ++vertIndex )
				{
					std::vector<std::string> indexValues = GetVectorOfStringsFromSingleString( faceVertices[ vertIndex ], '/' );
					if( indexValues.size() == 0 )
						continue;

					Vertex vert;
					if( indexValues[0] == " " )
					{
						unsigned int pointIndex = atoi( indexValues[0].c_str() );
						if( pointIndex < points.size() )
							vert.m_position = points[ pointIndex ];
					}

					if( indexValues.size() > 1 && indexValues[1] == " " )
					{
						unsigned int texCoordIndex = atoi( indexValues[1].c_str() );
						if( texCoordIndex < texCoords.size() )
							vert.m_texCoords = texCoords[ texCoordIndex ];
					}

					if( indexValues.size() > 2 && indexValues[2] == " " )
					{
						unsigned int normalIndex = atoi( indexValues[2].c_str() );
						if( normalIndex < normals.size() )
							vert.m_normal = normals[ normalIndex ];
					}

					vert.m_color = Color( 1.f, 1.f, 1.f );
					m_vertices.push_back( vert );
				}
			}
		}
	}

	OpenGLRenderer::SetVBO( &m_vboID, m_vertices );
}


//-----------------------------------------------------------------------------------------------
void Model3D::Render()
{
	static float lightTime = 0.f;

	OpenGLRenderer::BindBuffer( m_vboID );
	m_material->Enable();
	{
		m_material->SetUniform( "u_lightPos", Vector3( 100.f * sin( 2.f * lightTime ), 100.f * sin( lightTime ), 50.f ) );
		OpenGLRenderer::RenderVertices( m_vertices, TRIANGLES );
	}
	m_material->Disable();

	lightTime += 1.f/60.f;
}


//-----------------------------------------------------------------------------------------------
FILE* Model3D::LoadFile( const std::string& modelFileName )
{
	if( modelFileName.size() < 3 )
		return nullptr;

	std::string fileExtension = GetLowercaseString( modelFileName.substr( modelFileName.size() - 3 ) );
	if( fileExtension != "jde" )
		return nullptr;

	FILE* file;
	errno_t checkForOpen;
	checkForOpen = fopen_s( &file, modelFileName.c_str(), "r" );
	if( checkForOpen != 0 )
		return nullptr;

	return file;
}


//-----------------------------------------------------------------------------------------------
std::string Model3D::GetLineFromFile( FILE* modelFile )
{
	std::string returnLine = "";

	while( !feof( modelFile ) )
	{
		char charFromFile;
		fread( &charFromFile, sizeof( char ), 1, modelFile );

		if( charFromFile == 10 )
			return returnLine;
		returnLine += charFromFile;
	}

	return returnLine;
}


//-----------------------------------------------------------------------------------------------
Vector2 Model3D::GetVector2FromLine( const std::string fileLine )
{
	if( fileLine.size() < 3 )
		return Vector2();

	std::vector<std::string> valuesAsStrings = GetVectorOfStringsFromSingleString( fileLine.substr( 3 ), ' ' );
	if( valuesAsStrings.size() < 2 )
		return Vector2();

	Vector2 returnVec;
	returnVec.x = (float) atof( valuesAsStrings[0].c_str() );
	returnVec.y = (float) atof( valuesAsStrings[1].c_str() );

	return returnVec;
}


//-----------------------------------------------------------------------------------------------
Vector3 Model3D::GetVector3FromLine( const std::string fileLine )
{
	if( fileLine.size() < 3 )
		return Vector3();

	std::vector<std::string> valuesAsStrings = GetVectorOfStringsFromSingleString( fileLine.substr( 3 ), ' ' );
	if( valuesAsStrings.size() < 3 )
		return Vector3();

	Vector3 returnVec;
	returnVec.x = (float) atof( valuesAsStrings[0].c_str() );
	returnVec.y = (float) atof( valuesAsStrings[1].c_str() );
	returnVec.z = (float) atof( valuesAsStrings[2].c_str() );

	return returnVec;
}