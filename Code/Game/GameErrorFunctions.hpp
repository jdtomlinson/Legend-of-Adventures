#ifndef include_GameErrorFunctions
#define include_GameErrorFunctions
#pragma once

//-----------------------------------------------------------------------------------------------
#include "../Engine/XMLDocument.hpp"


//-----------------------------------------------------------------------------------------------
inline void ErrorMultiplePlayerDefinitionInMapLegend( const XMLDocument& xmlDoc )
{
	std::string messageWindowText = "PLAYER DEFINED MORE THAN ONCE\n\n";
	messageWindowText += "More than one PlayerDefinition found in file " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n";
	messageWindowText += "Only one player definition is allowed per map file\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Player Definition Error", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorMultiplePlayersInMap( const XMLDocument& xmlDoc, const std::string& mapName )
{
	std::string messageWindowText = "MORE THAN ONE PLAYER IN MAP\n\n";
	messageWindowText += "More than one player found in \'" + mapName + "\' map\n";
	messageWindowText += "Only one player is allowed per map\n\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Multiple Player Error", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorNoPlayersInMap( const XMLDocument& xmlDoc, const std::string& mapName )
{
	std::string messageWindowText = "NO PLAYER FOUND IN MAP\n\n";
	messageWindowText += "No player found in \'" + mapName + "\' map\n";
	messageWindowText += "One player is required per map\n\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "No Player Error", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorAsciiSymbolUsedInMultipleDefinitions( const XMLDocument& xmlDoc, char asciiSymbol )
{
	std::string messageWindowText = "ASCII SYMBOL USED IN MULTIPLE DEFINITIONS\n\n";
	messageWindowText += "ASCII symbol \'";
	messageWindowText += asciiSymbol;
	messageWindowText += "\' is used in multiple definitions\n";
	messageWindowText += "ASCII symbol definitions must be unique\n\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Multiple ASCII Symbol Definition", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorBlueprintNotFound( const XMLDocument& xmlDoc, const std::string& blueprintName )
{
	std::string messageWindowText = "BLUEPRINT DEFINITION NOT FOUND\n\n";
	messageWindowText += "Blueprint \'" + blueprintName + "\' was not defined\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Blueprint Not Defined", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorTextureFileNotFound( const XMLDocument& xmlDoc, const std::string& givenFileName )
{
	std::string messageWindowText = "GIVEN TEXTURE FILE COULD NOT BE LOADED\n\n";
	messageWindowText += "Given File Path/Name: " + givenFileName + "\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n";
	messageWindowText += "Please check if file exists and if its name matches the given file name\n";
	messageWindowText += "For best results, please enter full file path with file name\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Texture File Not Found", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorNonPositiveNumberOfRowsOrColumnsForSpriteSheet( const XMLDocument& xmlDoc, const IntVector2& givenValues )
{
	std::string givenStringX = ConvertNumberToString( givenValues.x );
	std::string givenStringY = ConvertNumberToString( givenValues.y );

	std::string messageWindowText = "NON-POSITIVE NUMBER OF ROWS AND/OR COLUMNS IN SPRITE SHEET DEFINITION\n\n";
	messageWindowText += "Cannot have values for number of rows and columns that are less than 1\n";
	messageWindowText += "Given Values: " + givenStringX + ", " + givenStringY + "\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Non-Positive Values For Sprite Sheet", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorTexCoordsNotValid( const XMLDocument& xmlDoc, const IntVector2& givenCoords, const IntVector2& maxAllowedCoords )
{
	std::string givenStringX = ConvertNumberToString( givenCoords.x );
	std::string givenStringY = ConvertNumberToString( givenCoords.y );
	std::string allowedStringX = ConvertNumberToString( maxAllowedCoords.x - 1 );
	std::string allowedStringY = ConvertNumberToString( maxAllowedCoords.y - 1 );

	std::string messageWindowText = "GIVEN TEXTURE COORDINATES NOT ALLOWED\n\n";
	messageWindowText += "The texture coordinates " + givenStringX + ", " + givenStringY + " are not allowed\n";
	if( givenCoords.x < 0 || givenCoords.y < 0 )
		messageWindowText += "Negative coordinates are not allowed\n";

	messageWindowText += "Allowed Texture Coordinates: (0 >= x <= " + allowedStringX +"), (0 >= y <= " + allowedStringY + ")\n";
	messageWindowText += "File Error Occurs: " + GetShortenedFileName( xmlDoc.GetFileName() ) + "\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "Texture Coordinates Not Valid", MB_OK );
	std::exit( EXIT_FAILURE );
}


//-----------------------------------------------------------------------------------------------
inline void ErrorNoMapsLoaded()
{
	std::string messageWindowText = "NO MAPS WERE FOUND\n\n";
	messageWindowText += "There were no map files found in folder Data\\XML Files\\Maps\n";
	messageWindowText += "At least one XML map file is needed\n\n";
	messageWindowText += "The program will now close";

	ShowCursor( TRUE );
	MessageBoxA( 0, messageWindowText.c_str(), "No Maps Loaded", MB_OK );
	std::exit( EXIT_FAILURE );
}


#endif // include_GameErrorFunctions