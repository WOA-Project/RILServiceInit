#pragma once
#include <Windows.h>

enum WWAN_INTF_OPCODE {
	WwanIntfOpcodePin = 0,
	WwanIntfOpcodeRadioState = 1,
	WwanIntfOpcodePreferredProviders = 2,
	WwanIntfOpcodeCurrentConnection = 3,
	WwanIntfOpcodeProvisionedContexts = 4,
	WwanIntfOpcodeActivateUserAccount = 5,
	WwanIntfOpcodeVendorSpecific = 6,
	WwanIntfOpcodeInterfaceObject = 7,
	WwanIntfOpcodeConnectionObject = 8,
	WwanIntfOpcodeAcState = 9,
	WwanIntfOpcodeClearManualConnectState = 10,
	WwanIntfOpcodeGetStoredRadioState = 11,
	WwanIntfOpcodeGetRadioInfo = 12,
	WwanIntfOpcodeHomeProvider = 13,
	WwanIntfOpcodeDataEnablement = 15
};

enum WWAN_INTERFACE_STATE {
	WwanInterfaceStateNotReady = 0,
	WwanInterfaceStateDeviceLocked = 1,
	WwanInterfaceStateUserAccountNotActivated = 2,
	WwanInterfaceStateRegistered = 3,
	WwanInterfaceStateRegistering = 4,
	WwanInterfaceStateDeregistered = 5,
	WwanInterfaceStateAttached = 6,
	WwanInterfaceStateAttaching = 7,
	WwanInterfaceStateDetaching = 8,
	WwanInterfaceStateActivated = 9,
	WwanInterfaceStateActivating = 10,
	WwanInterfaceStateDeactivating = 11
};

typedef struct _WWAN_DATA_ENABLEMENT {
	GUID                 guidProfileSet;
	DWORD                dwEnable;
} WWAN_DATA_ENABLEMENT, *PWWAN_DATA_ENABLEMENT;

typedef struct _WWAN_INTERFACE_STATUS {
	BOOL                 fInitialized;
	WWAN_INTERFACE_STATE InterfaceState;
} WWAN_INTERFACE_STATUS, * PWWAN_INTERFACE_STATUS;

typedef struct _WWAN_INTERFACE_INFO {
	GUID                  InterfaceGuid;
	WCHAR                 strInterfaceDescription[256];
	WWAN_INTERFACE_STATUS InterfaceStatus;
	DWORD                 dwReserved1;
	GUID                  guidReserved;
	GUID                  ParentInterfaceGuid;
	DWORD                 dwReserved2;
	DWORD                 dwIndex;
	DWORD                 dwReserved3;
	DWORD                 dwReserved4;
} WWAN_INTERFACE_INFO, * PWWAN_INTERFACE_INFO;

typedef struct _WWAN_INTERFACE_INFO_LIST {
	DWORD               dwNumberOfItems;
#ifdef __midl
	[unique, size_is(dwNumberOfItems)] WWAN_INTERFACE_INFO InterfaceInfo[*];
#else
	WWAN_INTERFACE_INFO InterfaceInfo[1];
#endif
} WWAN_INTERFACE_INFO_LIST, * PWWAN_INTERFACE_INFO_LIST;

GUID WWAN_PROFILE_SET_ALL = { 0x31A32D76, 0xE07F, 0x499D, {0x8F, 0xA3, 0xDD, 0xA7, 0x3B, 0xA7, 0x70, 0x57 } };

typedef ULONG WWAN_STATUS;

#define WWAN_STATUS_SUCCESS                           STATUS_SUCCESS
#define WWAN_STATUS_BUSY                              0xC0040002
#define WWAN_STATUS_FAILURE                           0xC0040003
#define WWAN_STATUS_SIM_NOT_INSERTED                  0xC0040004
#define WWAN_STATUS_BAD_SIM                           0xC0040005
#define WWAN_STATUS_PIN_REQUIRED                      0xC0040006
#define WWAN_STATUS_PIN_DISABLED                      0x40040007
#define WWAN_STATUS_NOT_REGISTERED                    0x40040008
#define WWAN_STATUS_PROVIDERS_NOT_FOUND               0x40040009
#define WWAN_STATUS_NO_DEVICE_SUPPORT                 0xC004000a
#define WWAN_STATUS_PROVIDER_NOT_VISIBLE              0x4004000b
#define WWAN_STATUS_DATA_CLASS_NOT_AVAILABLE          0x4004000c
#define WWAN_STATUS_PACKET_SVC_DETACHED               0xC004000d
#define WWAN_STATUS_MAX_ACTIVATED_CONTEXTS            0xC004000e
#define WWAN_STATUS_NOT_INITIALIZED                   0xC004000f
#define WWAN_STATUS_VOICE_CALL_IN_PROGRESS            0x40040010
#define WWAN_STATUS_CONTEXT_NOT_ACTIVATED             0xC0040011
#define WWAN_STATUS_SERVICE_NOT_ACTIVATED             0xC0040012
#define WWAN_STATUS_INVALID_ACCESS_STRING             0xC0040013
#define WWAN_STATUS_INVALID_USER_NAME_PWD             0xC0040014
#define WWAN_STATUS_RADIO_POWER_OFF                   0xC0040015
#define WWAN_STATUS_INVALID_PARAMETERS                0xC0040016
#define WWAN_STATUS_READ_FAILURE                      0xC0040017
#define WWAN_STATUS_WRITE_FAILURE                     0xC0040018
#define WWAN_STATUS_DENIED_POLICY                     0xC0040019
#define WWAN_STATUS_INVALID_DEVICE_SERVICE_OPERATION  0xC004001a
#define WWAN_STATUS_MORE_DATA                         0xC004001b

//SMS specific error codes
#define WWAN_STATUS_SMS_OPERATION_NOT_ALLOWED         0xC0040100
#define WWAN_STATUS_SMS_MEMORY_FAILURE                0xC0040101
#define WWAN_STATUS_SMS_INVALID_MEMORY_INDEX          0xC0040102
#define WWAN_STATUS_SMS_UNKNOWN_SMSC_ADDRESS          0xC0040103
#define WWAN_STATUS_SMS_NETWORK_TIMEOUT               0xC0040104
#define WWAN_STATUS_SMS_MEMORY_FULL                   0xC0040105
#define WWAN_STATUS_SMS_UNKNOWN_ERROR                 0xC0040106
#define WWAN_STATUS_SMS_FILTER_NOT_SUPPORTED          0xC0040107
#define WWAN_STATUS_SMS_MORE_DATA                     0x40040108
#define WWAN_STATUS_SMS_LANG_NOT_SUPPORTED            0xC0040109
#define WWAN_STATUS_SMS_ENCODING_NOT_SUPPORTED        0xC004010A
#define WWAN_STATUS_SMS_FORMAT_NOT_SUPPORTED          0xC004010B

//Authentication Algorithm specific error codes
#if ( _WIN32_WINNT >= _WIN32_WINNT_WIN8 || NTDDI_VERSION >= NTDDI_WIN8 || NDIS_SUPPORT_NDIS630 )
#define WWAN_STATUS_AUTH_INCORRECT_AUTN               0xC0040200
#define WWAN_STATUS_AUTH_SYNC_FAILURE                 0xC0040201
#define WWAN_STATUS_AUTH_AMF_NOT_SET                  0xC0040202
#endif

extern "C" __declspec(dllimport) HRESULT __stdcall WwanFreeMemory
(
	_In_ LPVOID pMem
);
extern "C" __declspec(dllimport) HRESULT __stdcall WwanEnumerateInterfaces
(
	_In_     HANDLE                     hClientHandle,
	_In_opt_ DWORD*                     pdwReserved,
	_Out_    WWAN_INTERFACE_INFO_LIST** ppInterfaceList
);
extern "C" __declspec(dllimport) HRESULT __stdcall WwanCloseHandle
(
	_In_       HANDLE hClientHandle,
	_Reserved_ VOID*  pReserved
);
extern "C" __declspec(dllimport) HRESULT __stdcall WwanSetInterface
(
	_In_ HANDLE           hClientHandle,
	_In_ const GUID* pInterfaceGuid,
	_In_ WWAN_INTF_OPCODE OpCode,
	_In_ DWORD            dwDataSize,
	_In_ const PVOID      pData,
	_In_ PVOID            pReserved1,
	_In_ PVOID            pReserved2,
	_In_ PVOID            pReserved3
);
extern "C" __declspec(dllimport) HRESULT __stdcall WwanOpenHandle
(
	_In_       DWORD   dwClientVersion,
	_Reserved_ VOID*   pReserved,
	_Out_      DWORD*  pdwNegotiatedVersion,
	_Out_      HANDLE* phClientHandle
);