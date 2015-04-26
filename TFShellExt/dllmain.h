// dllmain.h : Declaration of module class.

#if _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define MYDEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
	#define new MYDEBUG_NEW
#endif

class CThumbFishModule : public ATL::CAtlDllModuleT< CThumbFishModule >
{
public :
	DECLARE_LIBID(LIBID_ThumbFishLib)
	//DECLARE_REGISTRY_APPID_RESOURCEID(IDR_THUMBFISH, "{49EC783C-4B9B-4A32-969F-D640F6FC7E76}")
};

extern class CThumbFishModule _AtlModule;
