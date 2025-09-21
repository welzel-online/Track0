/***************************************************************************//**
 * @file    track0.cpp
 *
 * @brief   Main file of the Track0.
 *
 * @copyright   Copyright (c) 2023 by Welzel-Online
 ******************************************************************************/


/****************************************************************** Includes **/
#include <regex>
#include <iostream>
#include <cstdint>
#include <string>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>

 #include "version.h"


using namespace std;


/******************************************************************* Defines **/
typedef struct
{
    string  diskType;
    string  os;
    union
    {
        struct
        {
            int seclen;
            int tracks;
            int sectrk;
            int blocksize;
            int maxdir;
            int skew;
            int boottrk;
        };
        int intValues[7];
    };
} diskdef_t;


/********************************************************** Global Variables **/
string      workingDir = "";
string      diskFormat = "";
string      bootblockFilename = "";
string      imageFilename = "";

diskdef_t   diskdef;
string      diskdefNames[7] = { "seclen", "tracks", "sectrk", "blocksize", "maxdir", "skew", "boottrk" };


/******************************************************* Functions / Methods **/
int  getArgs( int numArgs, char* args[] );
void printUsage();


/***************************************************************************//**
 * @brief   Checks, if a file exists.
 *
 * @param   path    Path including filename to the file to be checked.
 *
 * @return  true    if the file exists, otherwise false.
 ******************************************************************************/
bool FileExists( string path )
{
    struct stat buf;
    int32_t result;

    result = stat( path.c_str(), &buf );

    return ( result == 0 );
}


/***************************************************************************//**
 * @brief  Deletes the spaces from the start of a string (left trim).
 *
 * @param str  The string to trim.
 ******************************************************************************/
static inline void ltrim( string &str )
{
    str.erase( str.begin(), find_if( str.begin(), str.end(), [](unsigned char ch) { return !isspace(ch); } ) );
}


/***************************************************************************//**
 * @brief  Deletes the spaces from the end of a string (right trim).
 *
 * @param str  The string to trim.
 ******************************************************************************/
static inline void rtrim( string &str )
{
    str.erase( find_if( str.rbegin(), str.rend(), [](unsigned char ch) { return !isspace(ch); } ).base(), str.end() );
}


/***************************************************************************//**
 * @brief Deletes the spaces from the start and the end of a string.
 *
 * @param str  The string to trim.
 ******************************************************************************/
static inline void trim( string &str )
{
    ltrim( str );
    rtrim( str );
}


/***************************************************************************//**
 * @brief Converts an integer to a string.
 *
 * @param Val      The integer value.
 * @param width    The desired length of the returned string.
 *
 * @return The converted integer with the specified length, filled with leading
 *         zeros.
 ******************************************************************************/
string IntToStr( int Val, int width )
{
	return (static_cast<std::stringstream const&>( stringstream() << setfill('0') << setw(width) << (int)Val ) ).str();
}


/***************************************************************************//**
 * @brief Checks, if the given string is a number.
 *
 * @param str  The string to check.
 *
 * @return True, if the string contains a number, false otherwise.
 ******************************************************************************/
bool isNumber( string str )
{
    char* p;
    strtol( str.c_str(), &p, 0 );

    return *p == 0;
}


/***************************************************************************//**
 * @brief Loads the given format from file diskdefs.
 *
 * @param format  The format to load.
 *
 * @return True, if the format was load, false otherwise.
 ******************************************************************************/
bool loadFormat( string format )
{
    bool   retVal = false;
    string line;
    size_t strPos;
    string strName;
    string strValue;


    if( !FileExists( workingDir + "\\diskdefs" ) )
    {
        cout << "ERROR: file diskdefs does not exist." << endl;
    }
    else
    {
        ifstream file( workingDir + "\\diskdefs" );
        if( file.is_open() )
        {
            while( getline( file, line ) )
            {
                trim( line );

                // Search for "#" comment
                strPos = line.find( "#", 0 );
                if( strPos != string::npos )
                {
                    // Delete comment till the end of the line
                    line = line.substr( 0, strPos );
                    trim( line );
                }

                // cout << "line: "  << line << endl;

                // Search for "diskdef"
                strPos = line.find( "diskdef", 0 );
                if( strPos == 0 )
                {
                    // Get Name and Value
                    line = line.substr( strPos + 7 );
                    trim( line );

                    // Format found?
                    if( line == diskFormat )
                    {
                        retVal = true;

                        diskdef.diskType = line;
                        // cout << "diskdef " << diskdef.diskType << " found" << endl;

                        while( getline( file, line ) )
                        {
                            trim( line );
                            if( line == "end" ) { break; }

                            strPos = line.find( " ", 0 );
                            if( strPos != string::npos )
                            {
                                strName = line.substr( 0, strPos );
                                trim( strName );

                                strValue = line.substr( strPos );
                                trim( strValue );

                                if( strName == "os" )
                                {
                                    diskdef.os = strValue;
                                }
                                else
                                {
                                    for( int i = 0; i < 7; i++ )
                                    {
                                        if( strName == diskdefNames[i] )
                                        {
                                            diskdef.intValues[i] = stoi( strValue, 0, ( strValue.find("0x") != string::npos ) ? 16 : 10 );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if( retVal == true ) { break; }
            }
            file.close();
        }
    }


    return retVal;
}


/***************************************************************************//**
 * @brief   The main function of Track0.
 *          This function takes the command line arguments, loads an disk image
 *          file and writes the given binary file to track 0/1.
 *
 * @param   argc    The number of arguments contained in argv[].
 * @param   argv    The passing parameters. Description in printUsage()
 *
 * @return  0 if everything is okay. -1 in case of general error.
 ******************************************************************************/
int main( int argc, char* argv[] )
{
    int retVal = 0;
    int bootsize = 0;
    int bootFileSize = 0;

    fstream imageFile;
    fstream bootFile;
    char    data;


    // Print status message
    cout << endl;
    cout << "track0 v" << TRACK0_REVISION << " - Copyright (c) 2023 by Welzel-Online" << endl << endl;

    retVal = getArgs( argc, argv );

    if( retVal == 0 )
    {
        // Load the desired format from file diskdefs
        if( loadFormat( diskFormat ) == true )
        {
            // Check for boot tracks
            if( diskdef.boottrk == 0 )
            {
                cout << "ERROR: No boot tracks defined in " << diskdef.diskType << endl;
                retVal = -1;
            }
            else
            {
                bootsize = diskdef.boottrk * diskdef.sectrk * diskdef.seclen;

                // Open boot track file
                bootFile.open( bootblockFilename, ios::binary | ios::in );
                bootFile.seekg( 0, ios::end );  // Seek to the file end
                bootFileSize = bootFile.tellg();
                if( bootsize < bootFileSize )
                {
                    cout << "ERROR: Size of the boot track(s) is to small for " << bootblockFilename << endl;
                    retVal = -1;
                }
                else
                {
                    bootFile.seekg( 0, ios::beg );  // Seek back to the beginning of the file
                }

                // Open disk image file
                imageFile.open( imageFilename, ios::binary | ios::out | ios::in );

                // Copy content of boot track file to disk image file
                for( int i = 0; i < bootsize; i++ )
                {
                    // Seek to offset in disk image file
                    imageFile.seekp( i );

                    // Get data from boot track file
                    bootFile.get( data );
                    // If EOF than use fill byte
                    if( !bootFile ) { data = 0xE5; }
                    imageFile.write( &data, 1 );
                }

                cout << "Copied " << bootFileSize << " Bytes from " << bootblockFilename << " to " << imageFilename << endl << endl;
            }

            imageFile.close();
            bootFile.close();
        }
    }
    else
    {
        printUsage();
    }

    return retVal;
}


/***************************************************************************//**
 * @brief   Parses the command line arguments and fills a global array.
 *
 * @param numArg   The number of arguments contained in args[].
 * @param args     The passing parameters. Description in printUsage()
 *
 * @return 0 if everything is okay. -1 in case of general error.
 ******************************************************************************/
int getArgs( int numArgs, char* args[] )
{
    int retVal = 0;
    size_t strPos;
    string arg = "";


    workingDir = args[0];
    strPos = workingDir.find_last_of( "/\\" );
    workingDir = workingDir.substr( 0, strPos );

    for( int i = 1; i < numArgs; i++ )
    {
        arg = args[i];

        if( arg == "-f" )               // CP/M disk format from file diskdefs
        {
            diskFormat = args[++i];

            // Trim spaces
            trim( diskFormat );
        }
        else if ( arg == "-b" )
        {
            bootblockFilename = args[++i];

            // Trim spaces
            trim( bootblockFilename );
        }
        else
        {
            imageFilename = args[i];

            // Trim spaces
            trim( imageFilename );

            break;
        }
    }

    // Check for needed arguments
    if( !FileExists( bootblockFilename ) )   { cout << "ERROR: Bootblock file (" << bootblockFilename << ") does not exist." << endl; retVal = -1; }
    if( !FileExists( imageFilename ) ) { cout << "ERROR: Image file (" << imageFilename << ") does not exist." << endl; retVal = -1; }

    return retVal;
}


/***************************************************************************//**
 * @brief   Shows a help text.
 ******************************************************************************/
void printUsage( void )
{
    cout << endl;
    cout << "Usage: track0.exe <param> <value> ... <param> <value> image" << endl << endl;

    cout << "    -f format    - Use the given CP/M disk format (diskdefs)." << endl;
    cout << "    -b bootblock - Write the contents of the file bootblock to the system tracks." << endl;

    cout << endl;
    cout << "Example:" << endl;
    cout << "track0.exe -f z80mbc2-d0 -b cpm22.bin DS0N00.DSK" << endl << endl;
}
