//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

//***************************************************************************
//
// Public header for CONTROL.C.
//
//***************************************************************************

#ifndef _control_public
#define _control_public
#ifdef __cplusplus
extern "C" {
#endif


//***************************************************************************
//
// DEFINES
//
//***************************************************************************

#define MaxJoys             2
#define MAXGAMEBUTTONS      64

//***************************************************************************
//
// TYPEDEFS
//
//***************************************************************************
typedef enum
   {
   axis_up,
   axis_down,
   axis_left,
   axis_right
   } axisdirection;

typedef enum
   {
   analog_turning=0,
   analog_strafing=1,
   analog_lookingupanddown=2,
   analog_elevation=3,
   analog_rolling=4,
   analog_moving=5,
   analog_maxtype
   } analogcontrol;

typedef enum
   {
   dir_North,
   dir_NorthEast,
   dir_East,
   dir_SouthEast,
   dir_South,
   dir_SouthWest,
   dir_West,
   dir_NorthWest,
   dir_None
   } direction;

typedef struct
   {
   bool   button0;
   bool   button1;
   direction dir;
   } UserInput;

typedef struct
   {
   fixed     dx;
   fixed     dy;
   fixed     dz;
   fixed     dyaw;
   fixed     dpitch;
   fixed     droll;
   } ControlInfo;

typedef enum
   {
   controltype_keyboard,
   controltype_keyboardandmouse,
   controltype_keyboardandjoystick,
   controltype_keyboardandexternal,
   controltype_keyboardandgamepad,
   controltype_keyboardandflightstick,
   controltype_keyboardandthrustmaster
   } controltype;


//***************************************************************************
//
// GLOBALS
//
//***************************************************************************

extern bool  CONTROL_RudderEnabled;
extern bool  CONTROL_MousePresent;
extern bool  CONTROL_JoysPresent[ MaxJoys ];
extern bool  CONTROL_MouseEnabled;
extern bool  CONTROL_JoystickEnabled;
extern byte     CONTROL_JoystickPort;
extern uint32   CONTROL_ButtonState1;
extern uint32   CONTROL_ButtonHeldState1;
extern uint32   CONTROL_ButtonState2;
extern uint32   CONTROL_ButtonHeldState2;


//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************

void CONTROL_MapKey( int32 which, kb_scancode key1, kb_scancode key2 );
void CONTROL_MapButton
        (
        int32 whichfunction,
        int32 whichbutton,
        bool doubleclicked
        );
void CONTROL_DefineFlag( int32 which, bool toggle );
bool CONTROL_FlagActive( int32 which );
void CONTROL_ClearAssignments( void );
void CONTROL_GetUserInput( UserInput *info );
void CONTROL_GetInput( ControlInfo *info );
void CONTROL_ClearUserInput( UserInput *info );
void CONTROL_WaitRelease( void );
void CONTROL_Ack( void );
void CONTROL_CenterJoystick
   (
   void ( *CenterCenter )( void ),
   void ( *UpperLeft )( void ),
   void ( *LowerRight )( void ),
   void ( *CenterThrottle )( void ),
   void ( *CenterRudder )( void )
   );
int32 CONTROL_GetMouseSensitivity( void );
void CONTROL_SetMouseSensitivity( int32 newsensitivity );
void CONTROL_Startup
   (
   controltype which,
   int32 ( *TimeFunction )( void ),
   int32 ticspersecond
   );
void CONTROL_Shutdown( void );

void CONTROL_MapAnalogAxis
   (
   int32 whichaxis,
   int32 whichanalog
   );

void CONTROL_MapDigitalAxis
   (
   int32 whichaxis,
   int32 whichfunction,
   int32 direction
   );
void CONTROL_SetAnalogAxisScale
   (
   int32 whichaxis,
   int32 axisscale
   );
void CONTROL_PrintAxes( void );

#ifdef __cplusplus
};
#endif
#endif
