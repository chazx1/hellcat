#include "sdk.hpp"


#include "../helpers/utils.hpp"

IVEngineClient*       g_EngineClient = nullptr;
IBaseClientDLL*       g_CHLClient = nullptr;
IClientEntityList*    g_EntityList = nullptr;
CGlobalVarsBase*      g_GlobalVars = nullptr;
IEngineTrace*         g_EngineTrace = nullptr;
ICvar*                g_CVar = nullptr;
IPanel*               g_VGuiPanel = nullptr;
IClientMode*          g_ClientMode = nullptr;
IVDebugOverlay*       g_DebugOverlay = nullptr;
ISurface*             g_VGuiSurface = nullptr;
CInput*               g_Input = nullptr;
IVModelInfoClient*    g_MdlInfo = nullptr;
IVModelRender*        g_MdlRender = nullptr;
IVRenderView*         g_RenderView = nullptr;
IMaterialSystem*      g_MatSystem = nullptr;
IGameEventManager2*   g_GameEvents = nullptr;
IMoveHelper*          g_MoveHelper = nullptr;
IMDLCache*            g_MdlCache = nullptr;
IPrediction*          g_Prediction = nullptr;
CGameMovement*        g_GameMovement = nullptr;
IEngineSound*         g_EngineSound = nullptr;
CGlowObjectManager*   g_GlowObjManager = nullptr;
IViewRender*          g_ViewRender = nullptr;
IDirect3DDevice9*     g_D3DDevice9 = nullptr;
CClientState*         g_ClientState = nullptr;
IPhysicsSurfaceProps* g_PhysSurface = nullptr;
C_LocalPlayer         g_LocalPlayer;
IVEffects*            g_Dlight = nullptr;
CNetworkStringTableContainer* g_ClientStringTableContainer = nullptr;
ILocalize*            g_Localize = nullptr;
RecvVarProxyFn		  o_nSequence = nullptr;
IMemAlloc*            g_pMemAlloc = nullptr;
IViewRenderBeams*	  g_RenderBeams = nullptr;
IInputSystem*         g_InputSystem = nullptr;

namespace Offsets
{
	DWORD invalidateBoneCache = 0x00;
	DWORD smokeCount = 0x00;
	DWORD playerResource = 0x00;
	DWORD bOverridePostProcessingDisable = 0x00;
	DWORD getSequenceActivity = 0x00;
	DWORD lgtSmoke = 0x00;
	DWORD dwCCSPlayerRenderablevftable = 0x00;
	DWORD reevauluate_anim_lod = 0x00;
}

namespace Interfaces
{
	CreateInterfaceFn get_module_factory(HMODULE module)
	{
		return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, "CreateInterface"));
	}

	template<typename T>
	T* get_interface(CreateInterfaceFn f, const char* szInterfaceVersion)
	{
		auto result = reinterpret_cast<T*>(f(szInterfaceVersion, nullptr));

		if (!result) {
			throw std::runtime_error(std::string("[get_interface] Failed to GetOffset interface: ") + szInterfaceVersion);
		}

		return result;
	}

	void Initialize()
	{

		auto pfnFactory = get_module_factory(GetModuleHandleW(L"inputsystem.dll"));
		auto engineFactory = get_module_factory(GetModuleHandleW(L"engine.dll"));
		auto clientFactory = get_module_factory(GetModuleHandleW(L"client_panorama.dll"));
		auto valveStdFactory = get_module_factory(GetModuleHandleW(L"vstdlib.dll"));
		auto vguiFactory = get_module_factory(GetModuleHandleW(L"vguimatsurface.dll"));
		auto vgui2Factory = get_module_factory(GetModuleHandleW(L"vgui2.dll"));
		auto matSysFactory = get_module_factory(GetModuleHandleW(L"materialsystem.dll"));
		auto dataCacheFactory = get_module_factory(GetModuleHandleW(L"datacache.dll"));
		auto vphysicsFactory = get_module_factory(GetModuleHandleW(L"vphysics.dll"));
		auto LocalizeFactory = get_module_factory(GetModuleHandleW(L"localize.dll"));

		g_CHLClient = get_interface<IBaseClientDLL>(clientFactory, "VClient018");
		g_EntityList = get_interface<IClientEntityList>(clientFactory, "VClientEntityList003");
		g_Prediction = get_interface<IPrediction>(clientFactory, "VClientPrediction001");
		g_GameMovement = get_interface<CGameMovement>(clientFactory, "GameMovement001");
		g_MdlCache = get_interface<IMDLCache>(dataCacheFactory, "MDLCache004");
		g_EngineClient = get_interface<IVEngineClient>(engineFactory, "VEngineClient014");
		g_MdlInfo = get_interface<IVModelInfoClient>(engineFactory, "VModelInfoClient004");
		g_MdlRender = get_interface<IVModelRender>(engineFactory, "VEngineModel016");
		g_RenderView = get_interface<IVRenderView>(engineFactory, "VEngineRenderView014");
		g_EngineTrace = get_interface<IEngineTrace>(engineFactory, "EngineTraceClient004");
		g_DebugOverlay = get_interface<IVDebugOverlay>(engineFactory, "VDebugOverlay004");
		g_GameEvents = get_interface<IGameEventManager2>(engineFactory, "GAMEEVENTSMANAGER002");
		g_EngineSound = get_interface<IEngineSound>(engineFactory, "IEngineSoundClient003");
		g_MatSystem = get_interface<IMaterialSystem>(matSysFactory, "VMaterialSystem080");
		g_CVar = get_interface<ICvar>(valveStdFactory, "VEngineCvar007");
		g_VGuiPanel = get_interface<IPanel>(vgui2Factory, "VGUI_Panel009");
		g_VGuiSurface = get_interface<ISurface>(vguiFactory, "VGUI_Surface031");
		g_PhysSurface = get_interface<IPhysicsSurfaceProps>(vphysicsFactory, "VPhysicsSurfaceProps001");
		g_Dlight = get_interface<IVEffects>(engineFactory, "VEngineEffects001");
		g_ClientStringTableContainer = get_interface<CNetworkStringTableContainer>(engineFactory, "VEngineClientStringTable001");
		g_Localize = get_interface<ILocalize>(LocalizeFactory, "Localize_001");
		g_InputSystem = get_interface<IInputSystem>(pfnFactory, "InputSystemVersion001");

		g_pMemAlloc = *(IMemAlloc**)(GetProcAddress(GetModuleHandle(L"tier0.dll"), "g_pMemAlloc"));

		auto client = GetModuleHandleW(L"client_panorama.dll");
		auto engine = GetModuleHandleW(L"engine.dll");
		auto dx9api = GetModuleHandleW(L"shaderapidx9.dll");


		g_RenderBeams = *(IViewRenderBeams**)(Utils::PatternScan(client, "A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 0x1);

		g_GlobalVars = **(CGlobalVarsBase***)((*(DWORD**)g_CHLClient)[0] + 0x1B);
		g_ClientMode = *(IClientMode**)(Utils::PatternScan(client, "B9 ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 53") + 1);
		g_Input = *(CInput**)(Utils::PatternScan(client, "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 1);
		g_MoveHelper = **(IMoveHelper***)(Utils::PatternScan(client, "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01") + 2);
		g_GlowObjManager = *(CGlowObjectManager**)(Utils::PatternScan(client, "0F 11 05 ? ? ? ? 83 C8 01") + 3);
		g_ViewRender = *(IViewRender**)(Utils::PatternScan(client, "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10") + 1);
		g_D3DDevice9 = **(IDirect3DDevice9***)(Utils::PatternScan(dx9api, "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
		g_ClientState = **(CClientState***)(Utils::PatternScan(engine, "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);

		g_LocalPlayer = *(C_LocalPlayer*)(Utils::PatternScan(client, "8B 0D ? ? ? ? 83 FF FF 74 07") + 2);

		Offsets::smokeCount = *(DWORD*)(Utils::PatternScan(client, "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);

		//g_LocalPlayer = *(C_LocalPlayer*)(Utils::PatternScan(client, "8B 0D ? ? ? ? 83 FF FF 74 07") + 2);
	}
}