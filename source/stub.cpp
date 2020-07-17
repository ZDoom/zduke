#include "duke3d.h"
#include "control.h"
#include "keyboard.h"
#include "i_system.h"
#include "s_sound.h"

char inputloc;

int MUSIC_StopSong (void)
{
	S_StopMusic (false);
	return 0;
}

int MUSIC_ErrorCode;

void MUSIC_Continue (void)
{
	S_ResumeSound ();
}

void MUSIC_Pause (void)
{
	S_PauseSound ();
}

void MUSIC_SetVolume (int)
{
}

int MUSIC_Shutdown (void)
{
	return 0;
}

void MUSIC_RegisterTimbreBank (unsigned char *)
{
}

char *MUSIC_ErrorString (int)
{
	return "MUSBlah";
}

int MUSIC_Init (int, int)
{
	return 0;
}


extern "C"
{
uint32 CONTROL_ButtonState1;
uint32 CONTROL_ButtonState2;

void CONTROL_GetInput (ControlInfo *info)
{
	memset (info, 0, sizeof(*info));
}

int32 CONTROL_GetMouseSensitivity ()
{
	return 0;
}

bool CONTROL_JoystickEnabled;
byte CONTROL_JoystickPort;
bool CONTROL_MousePresent = true;
bool CONTROL_RudderEnabled;

void CONTROL_SetMouseSensitivity( int32 newsensitivity )
{
}

void CONTROL_Shutdown (void)
{
}

void CONTROL_CenterJoystick
   (
   void ( *CenterCenter )( void ),
   void ( *UpperLeft )( void ),
   void ( *LowerRight )( void ),
   void ( *CenterThrottle )( void ),
   void ( *CenterRudder )( void )
   )
{
}

void CONTROL_ClearAssignments (void)
{
}

void CONTROL_DefineFlag (int32 which, bool toggle)
{
}

void CONTROL_MapAnalogAxis (int32, int32)
{
}

void CONTROL_MapDigitalAxis
   (
   int32 whichaxis,
   int32 whichfunction,
   int32 direction
   )
{
}

void CONTROL_MapButton
        (
        int32 whichfunction,
        int32 whichbutton,
        bool doubleclicked
        )
{
}

void CONTROL_MapKey( int32 which, kb_scancode key1, kb_scancode key2 )
{
}

void CONTROL_SetAnalogAxisScale
   (
   int32 whichaxis,
   int32 axisscale
   )
{
}

void CONTROL_Startup
   (
   controltype which,
   int32 ( *TimeFunction )( void ),
   int32 ticspersecond
   )
{
}

int32 MOUSE_GetButtons (void)
{
	return 0;
}

char * SCRIPT_Entry( int32 scripthandle, char * sectionname, int32 which )
{
	return "Booga";
}

void SCRIPT_Free( int32 scripthandle )
{
}

void SCRIPT_GetDoubleString
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   char * dest1,
   char * dest2
   )
{
}

bool SCRIPT_GetNumber
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   int32 * number
   )
{
	return 0;
}

void SCRIPT_GetString
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   char * dest
   )
{
}

void SCRIPT_GetBoolean
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   bool * 
   )
{
}

int32 SCRIPT_Load ( char * filename )
{
	return 0;
}

int32 SCRIPT_NumberEntries( int32 scripthandle, char * sectionname )
{
	return 0;
}

void SCRIPT_PutNumber
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   int32 number,
   bool hexadecimal,
   bool defaultvalue
   )
{
}

void SCRIPT_PutString
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   char * string
   )
{
}

void SCRIPT_Save (int32 scripthandle, char * filename)
{
}


}