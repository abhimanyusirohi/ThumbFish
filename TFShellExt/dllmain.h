// dllmain.h : Declaration of module class.

class CThumbFishModule : public ATL::CAtlDllModuleT< CThumbFishModule >
{
public :
	DECLARE_LIBID(LIBID_ThumbFishLib)
	//DECLARE_REGISTRY_APPID_RESOURCEID(IDR_THUMBFISH, "{49EC783C-4B9B-4A32-969F-D640F6FC7E76}")
};

extern class CThumbFishModule _AtlModule;
