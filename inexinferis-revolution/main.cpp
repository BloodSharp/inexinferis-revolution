#include "screenshots.h"
#include "opengl.h"
#include "client.h"
#include "utils.h"
#include "cvar.h"
#include "main.h"
#include "offsets.h"
#include "xorstr.h"

HINSTANCE g_hInstance=NULL;
DWORD g_dwModSize=0;

//paths!
CHAR dllpath[MAX_PATH]={0};
CHAR chtpath[MAX_PATH]={0};
CHAR inipath[MAX_PATH]={0};
CHAR cfgpath[MAX_PATH]={0};
CHAR waypath[MAX_PATH]={0};
CHAR nkspath[MAX_PATH]={0};
CHAR scrpath[MAX_PATH]={0};

BOOL iniFound=FALSE;
BOOL cfgFound=FALSE;
BOOL wayFound=FALSE;
BOOL nksFound=FALSE;
BOOL scrFound=FALSE;

//Cheat Flags
BOOL bSteam=FALSE,bSteamSW=FALSE;
BOOL bNeedPath=TRUE;
BOOL bInvalidVideoMode=FALSE;

//engine pointers
exporttable_t *pExport=NULL;
cl_enginefunc_t *pEngfuncs=NULL;
engine_studio_api_t *pEngStudio=NULL;
r_studio_interface_t *pInterface=NULL;
studio_model_renderer_t *pRenderer=NULL;
//playermove_t *ppmove=NULL;

//engine pointers copy
exporttable_t gExport;
cl_enginefuncs_s gEngfuncs;
engine_studio_api_t gEngStudio;
r_studio_interface_t gInterface;
studio_model_renderer_t gRenderer;

PVOID PreSDynamicSoundAddress;
double* globalTime=NULL;
tGetBaseGun pGetBaseGun=NULL;

VOID HookExport(exporttable_t* pExport){
  //hook HUD_Frame
  //pExport->HUD_Frame=HUD_Frame;
  //hook V_CalcRefdef
  pExport->V_CalcRefdef=V_CalcRefdef;
  //hook HUD_Key_Event
  pExport->HUD_Key_Event=HUD_Key_Event;
  //hook HUD_PlayerMove
  pExport->HUD_PlayerMove=HUD_PlayerMove;
  //hook HUD_PlayerMoveInit
  pExport->HUD_PlayerMoveInit=HUD_PlayerMoveInit;
  //hook HUD_UpdateClientData
  pExport->HUD_UpdateClientData=HUD_UpdateClientData;
  //hook HUD_PostRunCmd
  pExport->HUD_PostRunCmd=HUD_PostRunCmd;
  //hook HUD_Redraw
  pExport->HUD_Redraw=HUD_Redraw;
  //hook CL_CreateMove
  pExport->CL_CreateMove=CL_CreateMove;
  //hook HUD_AddEntity
  pExport->HUD_AddEntity=HUD_AddEntity;
}

VOID HookEngFunc(cl_enginefuncs_s *pEngfuncs){
  //path engfuncs...
  //path engfuncs...
  PDWORD pAddress=(PDWORD)&gEngfuncs;
  for(int i=0;i<80;i++){
    pAddress[i]=(DWORD)RedirectFunc((PBYTE)pAddress[i]);
  }
}

VOID HookStudio(r_studio_interface_t* pInterface,engine_studio_api_t *pStudioApi,studio_model_renderer_t* pRenderer){
  //fix aim behind me
  pStudioCheckBBox=pStudioApi->StudioCheckBBox;
  pStudioApi->StudioCheckBBox=hStudioCheckBBox;
  //hook StudioEntityLight
  pStudioEntityLight=pStudioApi->StudioEntityLight;
  pStudioApi->StudioEntityLight=hStudioEntityLight;
  //hook StudioDrawPlayer
  pStudioDrawPlayer=pInterface->StudioDrawPlayer;
  pInterface->StudioDrawPlayer=hStudioDrawPlayer;
  //hook StudioDrawModel
  pStudioDrawModel=pInterface->StudioDrawModel;
  pInterface->StudioDrawModel=hStudioDrawModel;
  //hook StudioModelRenderer (for chams -> this is a thiscall method!)
  //Can't replace in UCP!
  /*VirtualProtect((PVOID)&pRenderer->StudioRenderModel,4,PAGE_READWRITE,&dwback);
  pStudioRenderModel=pRenderer->StudioRenderModel;
  pRenderer->StudioRenderModel=hStudioRenderModel;
  VirtualProtect((PVOID)&pRenderer->StudioRenderModel,4,dwback,&dwback);*/
  //hwbp
  pStudioRenderModel=(tStudioRenderModel)HookFunc((PBYTE)pRenderer->StudioRenderModel,(PBYTE)hStudioRenderModel);
}

VOID OffsetsThread(){
  DWORD hwSize,pPreSDynamicSound=0,pPreSStaticSound=0;
  BOOL bExportFound=FALSE,bEngFuncsFound=FALSE,bEngStudioFound=FALSE;
  //get client data
  GetEngineBaseAndSize(&hwSize,&bSteam,&bSteamSW);
  //client is loaded after...
  GetClientBaseAndSize(NULL,bSteam);//need to be done here for UCP
  while(!bExportFound||!bEngFuncsFound||!bEngStudioFound){
    if(!bExportFound){
      if(!pExport){
        pExport=(exporttable_t*)FindExport();
      }
      if(pExport){
        if(pExport->HUD_GetStudioModelInterface){
          memcpy(&gExport,pExport,sizeof(exporttable_t));
          HookExport(pExport);
          bExportFound=TRUE;
        }
      }
    }
    if(!bEngFuncsFound&&bExportFound){
      if(!pEngfuncs){
        pEngfuncs=(cl_enginefunc_t*)FindEngine((DWORD)*pExport->Initialize);
      }
      if(pEngfuncs){
        if(pEngfuncs->pfnHookEvent){
          memcpy(&gEngfuncs,pEngfuncs,sizeof(cl_enginefunc_t));
          HookEngFunc(pEngfuncs);
          bEngFuncsFound=TRUE;
        }
      }
    }
    if(!bEngStudioFound&&bExportFound){
      if(!pEngStudio){
        pEngStudio=(engine_studio_api_t*)FindStudio((DWORD)*pExport->HUD_GetStudioModelInterface);
      }
      if(!pInterface){
        pInterface=(r_studio_interface_t*)FindInterface((DWORD)*pExport->HUD_GetStudioModelInterface);
      }
      if(pEngStudio&&pInterface){
        if(pEngStudio->Mem_Calloc&&pInterface->StudioDrawModel){
          if(!pRenderer){
            pRenderer=(studio_model_renderer_t*)FindRenderer((DWORD)pInterface->StudioDrawModel);
          }
          if(pRenderer&&pRenderer->Init){
            memcpy(&gEngStudio,pEngStudio,sizeof(engine_studio_api_s));
            memcpy(&gInterface,pInterface,sizeof(r_studio_interface_s));
            memcpy(&gRenderer,pRenderer,sizeof(studio_model_renderer_t));
            HookStudio(pInterface,pEngStudio,pRenderer);
            bEngStudioFound=TRUE;
          }
        }
      }
    }
    Sleep(bEngFuncsFound ? 100 : 10);
  }
  bInvalidVideoMode=!pEngStudio->IsHardware();
  //pEventList
  pEventList=(eventlist_t**)OffsetHookEventList();
  if(pEventList){
    HookEvents();
  }
  //pUserMsgList
  pUserMsgList=(usermsglist_t**)OffsetUserMsgList();
  if(pUserMsgList){
    HookUserMsgs();
  }
  //pSrvCmdList
  pSrvCmdList=(srvcmdlist_t**)OffsetSrvCmdList();
  if(pSrvCmdList){
    HookSrvCmds();
  }
  //SOUND ESP
  pPreSDynamicSound=OffsetPreSDynSound();
  if(pPreSDynamicSound){
    pPreS_DynamicSound=(tPreS_Sound)HookFunc((PBYTE)pPreSDynamicSound,(PBYTE)hPreS_DynamicSound);
  }
  pPreSStaticSound=OffsetPreSStatSound();
  if(pPreSStaticSound){
    pPreS_StaticSound=(tPreS_Sound)HookFunc((PBYTE)pPreSStaticSound,(PBYTE)hPreS_StaticSound);
  }
  //GlobalTime
  globalTime=OffsetGlobalTime((DWORD)pEngfuncs->pNetAPI->SendRequest);
  //Base Gun Spread...
  pGetBaseGun=(tGetBaseGun)OffsetGetBaseGun();
  cvar.Init();
}

VOID ShowInfo(){
  Sleep(2000);//Fix 1.5 bug...
  gEngfuncs.pfnConsolePrint(/*============================================\n\t\t\t\t\tInexinferis Revolution 2023 - BloodSharp's fork\n============================================\n\nOptions:\n----------\n* F4 - Disable\n* F5/Middle Button - Menu\n* F6 - Console\n\nExtra:\n-------\n* Alt - (In Triggerbot 2) Activate Triggerbot\n* Bloq Mayus - AutoRoute\n\nOriginal author and credits to Karman\n\n============================================\n*/XorStr<0xB5, 393, 0x13371337>("\x88\x8B\x8A\x85\x84\x87\x86\x81\x80\x83\x82\xFD\xFC\xFF\xFE\xF9\xF8\xFB\xFA\xF5\xF4\xF7\xF6\xF1\xF0\xF3\xF2\xED\xEC\xEF\xEE\xE9\xE8\xEB\xEA\xE5\xE4\xE7\xE6\xE1\xE0\xE3\xE2\xDD\xEB\xEB\xEA\xED\xEC\xEF\xAE\x86\x8C\x92\x82\x82\x8B\x8B\x9D\x99\x82\xD2\xA1\x91\x83\x99\x9B\x8D\x8D\x93\x94\x92\xDD\xCC\xCF\x32\x32\x22\x2E\x24\x47\x6A\x68\x67\x6D\x59\x63\x6D\x7F\x7E\x28\x63\x31\x74\x7C\x66\x7E\x1C\x2A\x25\x24\x27\x26\x21\x20\x23\x22\x1D\x1C\x1F\x1E\x19\x18\x1B\x1A\x15\x14\x17\x16\x11\x10\x13\x12\x0D\x0C\x0F\x0E\x09\x08\x0B\x0A\x05\x04\x07\x06\x01\x00\x03\x02\x7D\x7C\x7F\x49\x4E\x0A\x36\x33\x21\x26\x24\x38\x76\x47\x63\x62\x7D\x7C\x7F\x7E\x79\x78\x7B\x7A\x52\x73\x7A\x1D\x68\x7D\x73\x7F\x24\x08\x11\x02\x06\x09\x03\x6D\x42\x49\x2C\x5E\x43\x20\x07\x0B\x14\x1D\x17\x53\x36\x00\x02\x03\x17\x17\x5A\x56\x5C\x30\x1B\x11\xF5\x8B\xA8\xA3\xC2\xB3\xA6\xAA\xA8\xCA\xE5\xE5\xFF\xE2\xE2\xEA\x9A\x9B\xD7\xEB\xE0\xE7\xF7\xAD\x92\xB4\xB7\xB6\xB1\xB0\xB3\xB2\xAA\x8B\x82\xE2\xC8\xD1\x86\x8A\x88\x81\xE3\xC5\x8C\xF9\xDC\xC6\xD7\xD6\xD7\xC1\xD6\xDA\xC2\x97\x8A\x90\x9A\xFA\xDF\xC9\xD7\xC9\xA1\xB5\xA7\xE3\x90\xB7\xAF\xA0\xAF\xAC\xB8\xA9\xA3\xB9\xC4\xE5\xF0\x93\xBE\xBC\xA5\xF5\x9B\xB6\xA1\xAC\xA9\xFB\xF1\xFD\x9F\xAA\x94\x8E\xB0\x8C\x91\x91\x83\xED\xE2\xA6\x98\x82\x8B\x84\x80\x8E\x9C\xD1\x93\x86\x80\x9D\x99\x85\xD8\x98\x94\x9F\xDC\x9E\x8C\x9A\x64\x68\x76\x70\x24\x71\x69\x27\x43\x68\x78\x66\x6D\x63\x04\x05\x2D\x2C\x2F\x2E\x29\x28\x2B\x2A\x25\x24\x27\x26\x21\x20\x23\x22\x1D\x1C\x1F\x1E\x19\x18\x1B\x1A\x15\x14\x17\x16\x11\x10\x13\x12\x0D\x0C\x0F\x0E\x09\x08\x0B\x0A\x05\x04\x07\x06\x36" + 0x13371337).s);
  if(bInvalidVideoMode)
    gEngfuncs.pfnConsolePrint(" **********      WARNING! Invalid Video Mode!      ***********\n\
============================================\n");
  gEngfuncs.pfnClientCmd((char*)"fs_lazy_precache 1");
  gEngfuncs.pfnClientCmd((char*)"toggleconsole");
  //unbind cheat keys
  gEngfuncs.pfnClientCmd((char*)"unbind F4");
  gEngfuncs.pfnClientCmd((char*)"unbind F5");
  gEngfuncs.pfnClientCmd((char*)"unbind F6");
}

DWORD WINAPI StartThread(HINSTANCE hInstance){
  OffsetsThread();
  if(!bInvalidVideoMode){
    HookOpenGl();
  }
  ShowInfo();
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD fdwReason,LPVOID lpvReserved){
  BOOL bRet=TRUE;
  switch(fdwReason){
    case DLL_PROCESS_ATTACH: {
      //init
      g_hInstance=hInstance;
      GetModuleFileName(hInstance,dllpath,MAX_PATH);

      //Make Paths...
      strcpy(chtpath,dllpath);
      CHAR *tmp=chtpath;
      while(*(++tmp));
      while(*(--tmp)!='\\');
      *(tmp)=0;

      strcat(inipath,"\\inexinferis.ini");
      iniFound=FileExists(inipath);

      strcat(cfgpath,"\\inexinferis.cfg");
      cfgFound=FileExists(cfgpath);

      strcat(nkspath,"\\inexinferis.txt");
      nksFound=FileExists(nkspath);

      strcat(waypath,"\\WPoints");
      wayFound=DirectoryExist(waypath);

      strcat(scrpath,"\\ScreenShots");
      scrFound=DirectoryExist(scrpath);

      //cheat thread...
      CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartThread,hInstance,0,NULL);
    }
    break;
    default:// DLL_THREAD_ATTACH, DLL_THREAD_DETACH
      return FALSE;
  }
  return bRet;
}
