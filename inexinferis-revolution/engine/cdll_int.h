/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_int.h
//
// 4-23-98
// JOHN:  client dll interface declarations
//

#ifndef CDLL_INT_H
#define CDLL_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"

// this file is included by both the engine and the client-dll,
// so make sure engine declarations aren't done twice

//typedef int int;	// handle to a graphic

#define SCRINFO_SCREENFLASH 1
#define SCRINFO_STRETCHED	2

typedef struct SCREENINFO_S
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
} SCREENINFO;


typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
	float	fov;	// field of view
} client_data_t;

typedef struct client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;

typedef struct client_textmessage_s
{
	int		effect;
	byte	r1, g1, b1, a1;		// 2 colors for effects
	byte	r2, g2, b2, a2;
	float	x;
	float	y;
	float	fadein;
	float	fadeout;
	float	holdtime;
	float	fxtime;
	const char *pName;
	const char *pMessage;
} client_textmessage_t;

typedef struct hud_player_info_s
{
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

  // stuff that's unused at the moment,  but should be done
	byte spectator;
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

	//this 3 vars are not documented
	//but pfnGetPlayerInfo fill it
	//I don't know why...
	int unknown[3];

} hud_player_info_t;

typedef struct alias_s
{
  alias_s* Next;
  char* name[32]; //+ 4
  char* content[128]; // just a guess, never bothered to find the actual size
} alias_t;

typedef struct cl_enginefuncs_s
{
	// sprite handlers
	int						( *pfnSPR_Load )			( const char *szPicName );
	int							( *pfnSPR_Frames )			( int hPic );
	int							( *pfnSPR_Height )			( int hPic, int frame );
	int							( *pfnSPR_Width )			( int hPic, int frame );
	void						( *pfnSPR_Set )				( int hPic, int r, int g, int b );
	void						( *pfnSPR_Draw )			( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_DrawHoles )		( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_DrawAdditive )	( int frame, int x, int y, const wrect_t *prc );
	void						( *pfnSPR_EnableScissor )	( int x, int y, int width, int height );
	void						( *pfnSPR_DisableScissor )	( void );
	client_sprite_t				*( *pfnSPR_GetList )			(const char *psz, int *piCount );

	// screen handlers
	void						( *pfnFillRGBA )			( int x, int y, int width, int height, int r, int g, int b, int a );
	int							( *pfnGetScreenInfo ) 		( SCREENINFO *pscrinfo );
	void						( *pfnSetCrosshair )		( int hspr, wrect_t rc, int r, int g, int b );

	// cvar handlers
	struct cvar_s				*( *pfnRegisterVariable )	( char *szName, char *szValue, int flags );
	float						( *pfnGetCvarFloat )		( char *szName );
	char*						( *pfnGetCvarString )		( char *szName );

	// command handlers
	int							( *pfnAddCommand )			( char *cmd_name, void (*function)(void) );
	int							( *pfnHookUserMsg )			( char *szMsgName, pfnUserMsgHook pfn );
	int							( *pfnServerCmd )			( char *szCmdString );
	int							( *pfnClientCmd )			( char *szCmdString );
	void						( *pfnGetPlayerInfo )		( int ent_num, hud_player_info_t *pinfo );

	// sound handlers
	void						( *pfnPlaySoundByName )		( char *szSound, float volume );
	void						( *pfnPlaySoundByIndex )	( int iSound, float volume );

	// vector helpers
	void						( *pfnAngleVectors )		( const float * vecAngles, float * forward, float * right, float * up );

	// text message system
	client_textmessage_t		*( *pfnTextMessageGet )		( const char *pName );
	int							( *pfnDrawCharacter )		( int x, int y, int number, int r, int g, int b );
	int							( *pfnDrawConsoleString )	( int x, int y, char *string );
	void						( *pfnDrawSetTextColor )	( float r, float g, float b );
	void						( *pfnDrawConsoleStringLen )(  const char *string, int *length, int *height );
	void						( *pfnConsolePrint )		( const char *string );
	void						( *pfnCenterPrint )			( const char *string );

	// Added for user input processing
	int							( *GetWindowCenterX )		( void );
	int							( *GetWindowCenterY )		( void );
	void						( *GetViewAngles )			( float * );
	void						( *SetViewAngles )			( float * );
	int							( *GetMaxClients )			( void );
	void						( *Cvar_SetValue )			( char *cvar, float value );
	int       					(*Cmd_Argc)					(void);
	char						*( *Cmd_Argv )				( int arg );
	void						( *Con_Printf )				( char *fmt, ... );
	void						( *Con_DPrintf )			( char *fmt, ... );
	void						( *Con_NPrintf )			( int pos, char *fmt, ... );
	void						( *Con_NXPrintf )			( struct con_nprint_s *info, char *fmt, ... );
	const char					*( *PhysInfo_ValueForKey )	( const char *key );
	const char					*( *ServerInfo_ValueForKey )( const char *key );
	float						( *GetClientMaxspeed )		( void );
	int							( *CheckParm )				( char *parm, char **ppnext );
	void						( *Key_Event )				( int key, int down );
	void						( *GetMousePosition )		( int *mx, int *my );
	int							( *IsNoClipping )			( void );
	struct cl_entity_s			*( *GetLocalPlayer )		( void );
	struct cl_entity_s			*( *GetViewModel )			( void );
	struct cl_entity_s			*( *GetEntityByIndex )		( int idx );
	float						( *GetClientTime )			( void );
	void						( *V_CalcShake )			( void );
	void						( *V_ApplyShake )			( float *origin, float *angles, float factor );
	int							( *PM_PointContents )		( float *point, int *truecontents );
	int							( *PM_WaterEntity )			( float *p );
	struct pmtrace_s			*( *PM_TraceLine )			( float *start, float *end, int flags, int usehull, int ignore_pe );
	struct model_s				*( *CL_LoadModel )			( const char *modelname, int *index );
	int							( *CL_CreateVisibleEntity )	( int type, struct cl_entity_s *ent );
	const struct model_s *		( *GetSpritePointer )		( int /*HSPRITE*/);
  void						( *pfnPlaySoundByNameAtLocation )	( char *szSound, float volume, float *origin );
	unsigned short				( *pfnPrecacheEvent )		( int type, const char* psz );
	void						( *pfnPlaybackEvent )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	void						( *pfnWeaponAnim )			( int iAnim, int body );
	float						( *pfnRandomFloat )			( float flLow, float flHigh );
	long						( *pfnRandomLong )			( long lLow, long lHigh );
	void						( *pfnHookEvent )			( char *name, void ( *pfnEvent )( struct event_args_s *args ) );
	int							(*Con_IsVisible)			();
	const char					*( *pfnGetGameDirectory )	( void );
	struct cvar_s				*( *pfnGetCvarPointer )		( const char *szName );
	const char					*( *Key_LookupBinding )		( const char *pBinding );
	const char					*( *pfnGetLevelName )		( void );
	void						( *pfnGetScreenFade )		( struct screenfade_s *fade );
	void						( *pfnSetScreenFade )		( struct screenfade_s *fade );
	void                        *( *VGui_GetPanel )         ( );
	void                         ( *VGui_ViewportPaintBackground ) (int extents[4]);
	byte*						(*COM_LoadFile)				( char *path, int usehunk, int *pLength );
	char*						(*COM_ParseFile)			( char *data, char *token );
	void						(*COM_FreeFile)				( void *buffer );
	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int							( *IsSpectateOnly ) ( void );
	struct model_s				*( *LoadMapSprite )			( const char *filename );

	// file search functions
	void						( *COM_AddAppDirectoryToSearchPath ) ( const char *pszBaseDir, const char *appName );
	int							( *COM_ExpandFilename)				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char					*( *PlayerInfo_ValueForKey )( int playerNum, const char *key );
	void						( *PlayerInfo_SetValueForKey )( const char *key, const char *value );

  // Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean					(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);

	// TrackerID access
	int							(*GetTrackerIDForPlayer)(int playerSlot);
	int							(*GetPlayerForTrackerID)(int trackerID);

	// Same as pfnServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int							( *pfnServerCmdUnreliable )( char *szCmdString );
	void						( *pfnGetMousePos )( struct tagPOINT *ppt );
	void						( *pfnSetMousePos )( int x, int y );
	void						( *pfnSetMouseEnable )( qboolean fEnable );

	// Extended Functions   (not in 1.5!!!)
	/*
	struct cvar_s*	( *pfnGetCvarList )( void );
	struct cmd_s* ( *pfnGetCmdList )( void );
	char* ( *pfnGetCvarName )( struct cvar_s* cvar );
	char* ( *pfnGetCmdName )( struct cmd_s* cmd );
	float ( *pfnGetServerTime )( void );
	float ( *pfnGetGravity )( void );
	const struct model_s* ( *pfnPrecacheSprite )( int spr );
	void ( *OverrideLightmap )( int override );
	void ( *SetLightmapColor )( float r, float g, float b );
	void ( *SetLightmapDarkness )( float dark );
	int ( *pfnGetSequenceByName )( int flags, const char* seq );
	void ( *pfnSPR_DrawGeneric )( int frame, int x, int y, const wrect_t *prc, int blendsrc, int blenddst, int unknown1, int unknown2 );
	void ( *pfnLoadSentence )( void );
	int ( *pfnDrawLocalizedHudString )( int x, int y, const char* str, int r, int g, int b );
	int ( *pfnDrawLocalizedConsoleString )( int x, int y, const char* str );
	const char	*(*LocalPlayerInfo_ValueForKey)( const char* key );
	void ( *pfnDrawText_0 )( int x, int y, const char* text, unsigned long font );
	int ( *pfnDrawUnicodeCharacter )( int x, int y, short number, int r, int g, int b, unsigned long hfont );
	int ( *pfnCheckSoundFile )( const char* path );
	void* ( *GetCareerGameInterface )( void );
	void ( *pfnCvar_Set )( const char* cvar, const char* value );
	int ( *IsSinglePlayer )( void );
	void ( *pfnPlaySound )( const char* sound, float vol, float pitch );
	void ( *pfnPlayMp3 )( const char* mp3, int flags );
	float ( *Sys_FloatTime )( void );
	void ( *pfnSetArray )( int* array, int size );
	void ( *pfnSetClearArray )( int* array, int size );
	void ( *pfnClearArray )( void );
	void ( *pfnPlaySound2 )( const char* sound, float vol, float pitch );
	void	( *pfnTintRGBA )( int x, int y, int width, int height, int r, int g, int b, int a );
	void ( *pfnSetServerPassword )( const char* password );
  alias_t*( *pfnGetAliases)( void );
  */
} cl_enginefunc_t;

typedef struct exporttable_s
{
  int( *Initialize )( cl_enginefunc_t *, int );
  int( *HUD_Init )( void );
  int( *HUD_VidInit )( void );
  int( *HUD_Redraw )( float, int );
  int( *HUD_UpdateClientData )( client_data_t *, float );
  int( *HUD_Reset )( void );
  void( *HUD_PlayerMove )( struct playermove_s *, int );
  void( *HUD_PlayerMoveInit )( struct playermove_s * );
  char( *HUD_PlayerMoveTexture )( char * );
  void( *IN_ActivateMouse )( void );
  void( *IN_DeactivateMouse )( void );
  void( *IN_MouseEvent )( int mstate );
  void( *IN_ClearStates )( void );
  void( *IN_Accumulate )( void );
  void( *CL_CreateMove )( float, struct usercmd_s *, int );
  int( *CL_IsThirdPerson )( void );
  void( *CL_CameraOffset )( float * );
  struct kbutton_s *( *KB_Find )( const char *name );
  void( *CAM_Think )( void );
  void( *V_CalcRefdef )( struct ref_params_s *pparams );
  int( *HUD_AddEntity )( int, struct cl_entity_s *, const char * );
  void( *HUD_CreateEntities )( void );
  void( *HUD_DrawNormalTriangles )( void );
  void( *HUD_DrawTransparentTriangles )( void );
  void( *HUD_StudioEvent )( const struct mstudioevent_s *, const struct cl_entity_s * );
  void( *HUD_PostRunCmd )( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
  void( *HUD_Shutdown )( void );
  void( *HUD_TxferLocalOverrides )( struct entity_state_s *, const struct clientdata_s * );
  void( *HUD_ProcessPlayerState )( struct entity_state_s *, const struct entity_state_s * );
  void( *HUD_TxferPredictionData )( struct entity_state_s *, const struct entity_state_s *, struct clientdata_s *, const struct clientdata_s *, struct weapon_data_s *, const struct weapon_data_s * );
  void( *Demo_ReadBuffer )( int, unsigned char * );
  int( *HUD_ConnectionlessPacket )( struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
  int( *HUD_GetHullBounds )( int hullnumber, float *mins, float *maxs );
  void( *HUD_Frame )( double time );
  int( *HUD_Key_Event )( int, int, const char * );
  void( *HUD_TempEntUpdate )( double, double, double, struct tempent_s **, struct tempent_s **, int( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ), void( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
  struct cl_entity_s *( *HUD_GetUserEntity )( int index );
  int( *HUD_VoiceStatus )( int entindex, qboolean bTalking );
  int (*HUD_DirectorEvent) (unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags);
  int (*HUD_GetStudioModelInterface)(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
  //not in 1.5
	//void (*HUD_ChatInputPosition)(int *x,int *y);
	//Not in Steam
  //int (*HUD_GetPlayerTeam)(int iplayer );
  //void (*ClientFactory)();
} exporttable_t;

#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

#define CLDLL_INTERFACE_VERSION		7

extern void ClientDLL_Init( void ); // from cdll_int.c
extern void ClientDLL_Shutdown( void );
extern void ClientDLL_HudInit( void );
extern void ClientDLL_HudVidInit( void );
extern void	ClientDLL_UpdateClientData( void );
extern void ClientDLL_Frame( double time );
extern void ClientDLL_HudRedraw( int intermission );
extern void ClientDLL_MoveClient( struct playermove_s *ppmove );
extern void ClientDLL_ClientMoveInit( struct playermove_s *ppmove );
extern char ClientDLL_ClientTextureType( char *name );

extern void ClientDLL_CreateMove( float frametime, struct usercmd_s *cmd, int active );
extern void ClientDLL_ActivateMouse( void );
extern void ClientDLL_DeactivateMouse( void );
extern void ClientDLL_MouseEvent( int mstate );
extern void ClientDLL_ClearStates( void );
extern int ClientDLL_IsThirdPerson( void );
extern void ClientDLL_GetCameraOffsets( float *ofs );
extern int ClientDLL_GraphKeyDown( void );
extern struct kbutton_s *ClientDLL_FindKey( const char *name );
extern void ClientDLL_CAM_Think( void );
extern void ClientDLL_IN_Accumulate( void );
extern void ClientDLL_CalcRefdef( struct ref_params_s *pparams );
extern int ClientDLL_AddEntity( int type, struct cl_entity_s *ent );
extern void ClientDLL_CreateEntities( void );

extern void ClientDLL_DrawNormalTriangles( void );
extern void ClientDLL_DrawTransparentTriangles( void );
extern void ClientDLL_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
extern void ClientDLL_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
extern void ClientDLL_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
extern void ClientDLL_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
extern void ClientDLL_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
extern void ClientDLL_ReadDemoBuffer( int size, unsigned char *buffer );
extern int ClientDLL_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
extern int ClientDLL_GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void ClientDLL_VGui_ConsolePrint(const char* text);

extern int ClientDLL_Key_Event( int down, int keynum, const char *pszCurrentBinding );
extern void ClientDLL_TempEntUpdate( double ft, double ct, double grav, struct tempent_s **ppFreeTE, struct tempent_s **ppActiveTE, int ( *addTEntity )( struct cl_entity_s *pEntity ), void ( *playTESound )( struct tempent_s *pTemp, float damp ) );
extern struct cl_entity_s *ClientDLL_GetUserEntity( int index );
extern void ClientDLL_VoiceStatus(int entindex, qboolean bTalking);
extern void ClientDLL_DirectorMessage( int iSize, void *pbuf );

#ifdef __cplusplus
}
#endif

#endif // CDLL_INT_H
