#include "Game.hpp"
#include "../Engine/DeveloperConsole.hpp"


//-----------------------------------------------------------------------------------------------
Game::Game( float gameWidth, float gameHeight )
	: m_size( gameWidth, gameHeight )
	, m_world( gameWidth, gameHeight )
{

}


//-----------------------------------------------------------------------------------------------
void Game::Initialize()
{
	m_world.Initialize();
	m_isPaused = false;
	m_camera.m_position = Vector3( 0.f, 0.f, 0.f );
	m_mouse = Mouse( CURSOR_TEXTURE_FILE_NAME );
}


//-----------------------------------------------------------------------------------------------
bool Game::ProcessKeyDownEvent( unsigned char virtualKeyCode )
{
	m_keyboard.SetKeyDown( virtualKeyCode );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Game::ProcessKeyUpEvent( unsigned char virtualKeyCode )
{
	m_keyboard.SetKeyUp( virtualKeyCode );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Game::ProcessCharDownEvent( unsigned char charCode )
{
	m_keyboard.SetCharDown( charCode );
	return false;
}


//-----------------------------------------------------------------------------------------------
void Game::SetCameraPositionAndOrientation( const Camera& camera )
{
	OpenGLRenderer::Rotatef( -90.f, 1.f, 0.f, 0.f );
	OpenGLRenderer::Rotatef( 90.f, 0.f, 0.f, 1.f );

	OpenGLRenderer::Rotatef( -camera.m_orientation.roll, 1.f, 0.f, 0.f );
	OpenGLRenderer::Rotatef( -camera.m_orientation.pitch, 0.f, 1.f, 0.f );
	OpenGLRenderer::Rotatef( -camera.m_orientation.yaw, 0.f, 0.f, 1.f );

	OpenGLRenderer::Translatef( -camera.m_position.x, -camera.m_position.y, -camera.m_position.z );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameraPositionFromInput( float deltaSeconds )
{
	float yawRadians = ConvertDegreesToRadians( m_camera.m_orientation.yaw );
	Vector3 cameraMoveVector( 0.f, 0.f, 0.f );
	Vector3 cameraForwardXY( cos( yawRadians ), sin( yawRadians ), 0.f );
	Vector3 cameraLeftXY( -cameraForwardXY.y, cameraForwardXY.x, 0.f );

	if( m_keyboard.IsKeyPressedDown( KEY_W ) )
	{
		cameraMoveVector += cameraForwardXY;
	}
	else if( m_keyboard.IsKeyPressedDown( KEY_S ) )
	{
		cameraMoveVector -= cameraForwardXY;
	}

	if( m_keyboard.IsKeyPressedDown( KEY_A ) )
	{
		cameraMoveVector += cameraLeftXY;
	}
	else if( m_keyboard.IsKeyPressedDown( KEY_D ) )
	{
		cameraMoveVector -= cameraLeftXY;
	}

	if( m_keyboard.IsKeyPressedDown( KEY_E ) )
	{
		cameraMoveVector.z += 1.f;
	}
	else if( m_keyboard.IsKeyPressedDown( KEY_Q ) )
	{
		cameraMoveVector.z -= 1.f;
	}

	cameraMoveVector *= ( MOVE_SPEED_POINTS_PER_SECOND * deltaSeconds );
	m_camera.m_position += cameraMoveVector;
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameraOrientationFromInput( float deltaSeconds )
{
	float rotationRadiansPerSecond = ConvertDegreesToRadians( ROTATION_DEGREES_PER_SECOND );
	EulerAngles cameraRotation( 0.f, 0.f, 0.f );

	Vector2 currentMousePosition = m_mouse.GetCursorPositionOnWindow();

	cameraRotation.yaw += ( m_size.x * 0.5f ) - currentMousePosition.x;
	cameraRotation.pitch += currentMousePosition.y - ( m_size.y * 0.5f );
	cameraRotation *= ( rotationRadiansPerSecond * deltaSeconds );
	m_camera.m_orientation += cameraRotation;

	if( m_camera.m_orientation.pitch > 90.f )
	{
		m_camera.m_orientation.pitch = 90.f;
	}
	else if( m_camera.m_orientation.pitch < -90.f )
	{
		m_camera.m_orientation.pitch = -90.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromInput( float deltaSeconds, HWND hWnd )
{
	if( m_keyboard.IsKeyPressedDownAndWasNotBefore( KEY_TILDE ) )
	{
		g_developerConsole.m_drawConsole = !g_developerConsole.m_drawConsole;
	}

	if( m_keyboard.IsKeyPressedDownAndWasNotBefore( KEY_SPACE ) )
	{
		m_isPaused = !m_isPaused;
		if( m_world.CheckIfGameOver() )
			m_isPaused = false;
	}

	m_mouse.Update( deltaSeconds, hWnd );

	UpdateCameraOrientationFromInput( deltaSeconds );

	if( g_developerConsole.m_drawConsole )
		return;

	UpdateCameraPositionFromInput( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::Update( HWND hWnd )
{
	float deltaSeconds = static_cast< float >( FRAME_TIME_SECONDS );

	if( !m_isPaused )
	{
		m_world.Update( deltaSeconds, m_keyboard, m_mouse );
	}
	
	UpdateFromInput( deltaSeconds, hWnd );

	if( g_developerConsole.m_drawConsole )
		g_developerConsole.Update( m_keyboard, deltaSeconds );

	m_keyboard.Update();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderWorld3D()
{
	OpenGLRenderer::PushMatrix();

	OpenGLRenderer::EnableDepthTest();
	OpenGLRenderer::ClearColorBufferBit();
	OpenGLRenderer::ClearDepthBufferBit();

	OpenGLRenderer::SetPerspective( FIELD_OF_VIEW_Y, SIXTEEN_BY_NINE, 0.1f, 1000.f );

	SetCameraPositionAndOrientation( m_camera );

	m_world.RenderObjects3D();

	OpenGLRenderer::PopMatrix();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderWorld2D()
{
	OpenGLRenderer::PushMatrix();

	OpenGLRenderer::DisableDepthTest();
	OpenGLRenderer::SetOrtho( 0.f, m_size.x, 0.f, m_size.y, 0.f, 1.f );

	m_world.RenderObjects2D();
	if( m_isPaused )
		m_world.RenderPause();

	m_mouse.RenderCursor();

	if( g_developerConsole.m_drawConsole )
		g_developerConsole.Render();

	OpenGLRenderer::PopMatrix();
}


//-----------------------------------------------------------------------------------------------
void Game::Render()
{
	RenderWorld3D();
	RenderWorld2D();
}