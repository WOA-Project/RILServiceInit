// RILTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <rilapi.h>
#include <rilapitypes.h>

HANDLE ResultEvent;

DWORD RdwCode;
LPVOID RusersContext;
const void* RlpData;
DWORD RcbData;
LPVOID RlpParam;

void CALLBACK RILResultCallback(
	HRIL            hRil,         // @parm RIL handle to current RIL instance
	DWORD           dwCode,       // @parm result code
	LPVOID          usersContext, // @parm users context assigned by the call that originated this response
	const void* lpData,       // @parm data associated with the notification
	DWORD           cbData,       // @parm size of the strcuture pointed to lpData
	LPVOID          lpParam       // @parm parameter passed to <f RIL_Initialize>
)
{
	RdwCode = dwCode;
	RusersContext = usersContext;
	RlpData = lpData;
	RcbData = cbData;
	RlpParam = lpParam;

	SetEvent(ResultEvent);
}

void CALLBACK RILNotifyCallback(
	HRIL  hRil,             // @parm RIL handle to current RIL instance
	DWORD dwCode,           // @parm notification code
	const void* lpData,     // @parm data associated with the notification
	DWORD cbData,           // @parm size of the strcuture pointed to lpData
	LPVOID lpParam          // @parm parameter passed to <f RIL_Initialize> 
)
{

}

BOOL GetDriverVersion(DWORD* version)
{
	std::cout << "Gathering Driver Version" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	BYTE buffer[] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x32, 0x0 };
	RILGETDRIVERVERSIONPARAMS params = *(RILGETDRIVERVERSIONPARAMS*)buffer;

	hr = RIL_GetDriverVersion(hRil, &context, params.dwMinVersion, params.dwMaxVersion);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != sizeof(DWORD))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*version = *(DWORD*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetDevCaps(DWORD dwCapsType)
{
	std::cout << "Gathering Device Capabilities" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetDevCaps(hRil, &context, dwCapsType);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetRadioConfiguration(DWORD* radioConfiguration)
{
	std::cout << "Gathering Radio Configuration" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetRadioConfiguration(hRil, &context);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != sizeof(DWORD))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*radioConfiguration = *(DWORD*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetDeviceInfo(DWORD dwExecutor, RILDEVICEINFORMATION dwDeviceInformation, WCHAR** result, DWORD* length)
{
	std::cout << "Gathering Device Information" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetDeviceInfo(hRil, &context, dwExecutor, dwDeviceInformation);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	*result = (WCHAR*)RlpData;
	*length = RcbData / sizeof(WCHAR);

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL EnumerateSlots(RILUICCSLOTINFO* slotinfo)
{
	std::cout << "Enumerating slots" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_EnumerateSlots(hRil, &context);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(DWORD) * 4)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*slotinfo = *(RILUICCSLOTINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetCardInfo(DWORD dwSlotIndex, RILUICCCARDINFO* cardinfo)
{
	std::cout << "Getting Card info" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetCardInfo(hRil, &context, dwSlotIndex);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(DWORD) * 3 + sizeof(BOOL) + sizeof(BYTE) * 10)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*cardinfo = *(RILUICCCARDINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetEquipmentState(RILEQUIPMENTSTATE* equipmentstate)
{
	std::cout << "Getting Equipment state" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetEquipmentState(hRil, &context);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != sizeof(DWORD))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*equipmentstate = *(RILEQUIPMENTSTATE*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetAllEmergencyNumbers(BYTE** emergencynumberslist)
{
	std::cout << "Getting all emergency numbers" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetAllEmergencyNumbers(hRil, &context);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(DWORD) * 2)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*emergencynumberslist = (BYTE*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetCurrentRegStatus(DWORD dwExecutor, RILREGSTATUSINFO* regstatusinfo)
{
	std::cout << "Getting current registration status" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetCurrentRegStatus(hRil, &context, dwExecutor);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILREGSTATUSINFO))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*regstatusinfo = *(RILREGSTATUSINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetExecutorRFState(DWORD dwExecutor, RILEXECUTORRFSTATE* executorrfstate)
{
	std::cout << "Setting executor RF state" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetExecutorRFState(hRil, &context, dwExecutor); //WTF??? RIL_GetExecutorRFState is in reality RIL_SetExecutorRFState when calling IOControl and RIL_GetExecutorRFState is RIL_ResetModem
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILEXECUTORRFSTATE))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*executorrfstate = *(RILEXECUTORRFSTATE*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL OpenUiccLogicalChannel()
{
	std::cout << "OpenUiccLogicalChannel" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_OpenUiccLogicalChannel(hRil, &context, 0, 0, 0, NULL, 0);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCRECORDSTATUS))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetUiccRecordStatus(const RILUICCFILEPATH* lpFilePath, RILUICCRECORDSTATUS* recordstatus)
{
	std::cout << "GetUiccRecordStatus" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetUiccRecordStatus(hRil, &context, lpFilePath);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCRECORDSTATUS))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	*recordstatus = *(RILUICCRECORDSTATUS*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetExecutorConfig(DWORD dwExecutor, const RILEXECUTORCONFIG* lpRilExecutorConfig)
{
	std::cout << "SetExecutorConfig" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetExecutorConfig(hRil, &context, dwExecutor, lpRilExecutorConfig);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SendRestrictedUiccCmd1()
{
	std::cout << "SendRestrictedUiccCmd1" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// gave up parsing this so...
	BYTE buffer[] = { 0x44, 0x1, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x2c, 0x0, 0x0, 0x0, 0x9, 0x0, 0x0, 0x0,
						0x1, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x5, 0x2f, 0xd1, 0x1, 0x0, 0x0,
						0xf0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x0, 0x0 };

	RILSENDRESTRICTEDUICCCMDPARAMS params = *(RILSENDRESTRICTEDUICCCMDPARAMS*)buffer;

	hr = RIL_SendRestrictedUiccCmd(hRil, &context, params.dwCommand, &params.rscpParameters, params.pbData, params.dwDataSize, &params.lockVerification);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCRESPONSE))
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL ResetModem(DWORD dwExecutor, BOOL fExecutorRFState)
{
	std::cout << "Resetting modem" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetExecutorRFState(hRil, &context, dwExecutor, fExecutorRFState); //WTF??? RIL_GetExecutorRFState is in reality RIL_SetExecutorRFState when calling IOControl and RIL_GetExecutorRFState is RIL_ResetModem
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetEquipmentState(DWORD dwEquipmentState)
{
	std::cout << "Setting equipment state" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetEquipmentState(hRil, &context, dwEquipmentState);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetSlotPower(DWORD dwSlotIndex, BOOL fPowerOn)
{
	std::cout << "Setting slot power" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetSlotPower(hRil, &context, dwSlotIndex, fPowerOn);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "Waiting" << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "This result wasn't for us it seems" << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "This result isn't expected" << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

void PrintBanner()
{
	/*std::cout << "  __  __           _                  _          _                         " << std::endl;
	std::cout << " |  \\/  |         | |                | |        (_)                        " << std::endl;
	std::cout << " | \\  / | ___   __| | ___ _ __ ___   | |__  _ __ _ _ __   __ _ _   _ _ __  " << std::endl;
	std::cout << " | |\\/| |/ _ \\ / _` |/ _ \\ '_ ` _ \\  | '_ \\| '__| | '_ \\ / _` | | | | '_ \\ " << std::endl;
	std::cout << " | |  | | (_) | (_| |  __/ | | | | | | |_) | |  | | | | | (_| | |_| | |_) |" << std::endl;
	std::cout << " |_|  |_|\\___/ \\__,_|\\___|_| |_| |_| |_.__/|_|  |_|_| |_|\\__, |\\__,_| .__/ " << std::endl;
	std::cout << "                                                          __/ |     | |    " << std::endl;
	std::cout << "                                                         |___/      |_|    " << std::endl;*/

	std::cout << std::endl;
	std::cout << " 888888ba  dP dP                                          " << std::endl;
	std::cout << " 88    `8b 88 88                                          " << std::endl;
	std::cout << "a88aaaa8P' 88 88                                          " << std::endl;
	std::cout << " 88   `8b. 88 88                                          " << std::endl;
	std::cout << " 88     88 88 88                                          " << std::endl;
	std::cout << " dP     dP dP 88888888P                                   " << std::endl;
	std::cout << "                                                          " << std::endl;
	std::cout << "                                                          " << std::endl;
	std::cout << ".d88888b                             oo                   " << std::endl;
	std::cout << "88.    \"'                                                 " << std::endl;
	std::cout << "`Y88888b. .d8888b. 88d888b. dP   .dP dP .d8888b. .d8888b. " << std::endl;
	std::cout << "      `8b 88ooood8 88'  `88 88   d8' 88 88'  `"" 88ooood8 " << std::endl;
	std::cout << "d8'   .8P 88.  ... 88       88 .88'  88 88.  ... 88.  ... " << std::endl;
	std::cout << " Y88888P  `88888P' dP       8888P'   dP `88888P' `88888P' " << std::endl;
	std::cout << "                                                          " << std::endl;
	std::cout << "                                                          " << std::endl;
	std::cout << "dP          oo   dP                                       " << std::endl;
	std::cout << "88               88                                       " << std::endl;
	std::cout << "88 88d888b. dP d8888P                                     " << std::endl;
	std::cout << "88 88'  `88 88   88                                       " << std::endl;
	std::cout << "88 88    88 88   88                                       " << std::endl;
	std::cout << "dP dP    dP dP   dP                                       " << std::endl;
	std::cout << std::endl;
	std::cout << "Version 0.1 - LumiaWOA (c) 2018-2019" << std::endl;
	std::cout << std::endl;
}

void DisplayCurrentRegStatus()
{
	RILREGSTATUSINFO regstatusinfo;
	BOOL result = GetCurrentRegStatus(0, &regstatusinfo);
	if (result)
	{
		std::cout << "Executor: " << regstatusinfo.dwExecutor << std::endl;

		std::cout << "Registration status: ";
		switch (regstatusinfo.dwRegStatus)
		{
		case RILREGSTAT::RIL_REGSTAT_ATTEMPTING:
		{
			std::cout << "attempting" << std::endl;
			break;
		}
		case RILREGSTAT::RIL_REGSTAT_DENIED:
		{
			std::cout << "denied" << std::endl;
			break;
		}
		case RILREGSTAT::RIL_REGSTAT_HOME:
		{
			std::cout << "home" << std::endl;
			break;
		}
		case RILREGSTAT::RIL_REGSTAT_ROAMING:
		{
			std::cout << "roaming" << std::endl;
			break;
		}
		case RILREGSTAT::RIL_REGSTAT_ROAMING_DOMESTIC:
		{
			std::cout << "roaming domestic" << std::endl;
			break;
		}
		case RILREGSTAT::RIL_REGSTAT_UNKNOWN:
		{
			std::cout << "unknown" << std::endl;
			break;
		}
		case RILREGSTAT::RIL_REGSTAT_UNREGISTERED:
		{
			std::cout << "unregistered" << std::endl;
			break;
		}
		}

		std::cout << "Registration reject reason: " << regstatusinfo.dwRegRejectReason << std::endl;
		std::cout << "System capabilities: " << regstatusinfo.dwSystemCaps << std::endl;

		std::cout << "Voice domain: ";
		switch (regstatusinfo.dwVoiceDomain)
		{
		case RILVOICEDOMAIN::RIL_VOICE_DOMAIN_3GPP:
		{
			std::cout << "3GPP" << std::endl;
			break;
		}
		case RILVOICEDOMAIN::RIL_VOICE_DOMAIN_3GPP2:
		{
			std::cout << "3GPP2" << std::endl;
			break;
		}
		case RILVOICEDOMAIN::RIL_VOICE_DOMAIN_IMS:
		{
			std::cout << "IMS" << std::endl;
			break;
		}
		case RILVOICEDOMAIN::RIL_VOICE_DOMAIN_NONE:
		{
			std::cout << "none" << std::endl;
			break;
		}
		}


		std::cout << "Access technology: " << std::endl;
		std::cout << "System type: " << regstatusinfo.ratAccessTechnology.dwSystemType << std::endl;

		if (regstatusinfo.ratAccessTechnology.kindUnion.dwEvdoKind != NULL)
		{
			std::cout << "System type: EVDO/";
			switch (regstatusinfo.ratAccessTechnology.kindUnion.dwEvdoKind)
			{
			case RILEVDOKIND::RIL_EVDOKIND_REV0:
			{
				std::cout << "REV0" << std::endl;
				break;
			}
			case RILEVDOKIND::RIL_EVDOKIND_REVA:
			{
				std::cout << "REVA" << std::endl;
				break;
			}
			case RILEVDOKIND::RIL_EVDOKIND_REVB:
			{
				std::cout << "REVB" << std::endl;
				break;
			}
			}
		}
		else if (regstatusinfo.ratAccessTechnology.kindUnion.dwGsmKind != NULL)
		{
			std::cout << "System type: GSM/";
			switch (regstatusinfo.ratAccessTechnology.kindUnion.dwGsmKind)
			{
			case RILGSMKIND::RIL_GSMKIND_EDGE:
			{
				std::cout << "EDGE" << std::endl;
				break;
			}
			case RILGSMKIND::RIL_GSMKIND_GPRS:
			{
				std::cout << "GPRS" << std::endl;
				break;
			}
			case RILGSMKIND::RIL_GSMKIND_GSM:
			{
				std::cout << "GSM" << std::endl;
				break;
			}
			}
		}
		else if (regstatusinfo.ratAccessTechnology.kindUnion.dwLteKind != NULL)
		{
			std::cout << "System type: LTE/";
			switch (regstatusinfo.ratAccessTechnology.kindUnion.dwLteKind)
			{
			case RILLTEKIND::RIL_LTEKIND_UNKNOWN:
			{
				std::cout << "Unknown" << std::endl;
				break;
			}
			case RILLTEKIND::RIL_LTEKIND_FDD:
			{
				std::cout << "FDD" << std::endl;
				break;
			}
			case RILLTEKIND::RIL_LTEKIND_TDD:
			{
				std::cout << "TDD" << std::endl;
				break;
			}
			case RILLTEKIND::RIL_LTEKIND_RESERVED:
			{
				std::cout << "Reserved" << std::endl;
				break;
			}
			case RILLTEKIND::RIL_LTEKIND_UNKNOWN_CA:
			{
				std::cout << "Unknown CA" << std::endl;
				break;
			}
			case RILLTEKIND::RIL_LTEKIND_FDD_CA:
			{
				std::cout << "FDD CA" << std::endl;
				break;
			}
			case RILLTEKIND::RIL_LTEKIND_TDD_CA:
			{
				std::cout << "TDD CA" << std::endl;
				break;
			}
			}
		}
		else if (regstatusinfo.ratAccessTechnology.kindUnion.dwTdscdmaKind != NULL)
		{
			std::cout << "System type: TDSCD CDMA/";
			switch (regstatusinfo.ratAccessTechnology.kindUnion.dwTdscdmaKind)
			{
			case RILTDSCDMAKIND::RIL_TDSCDMAKIND_UMTS:
			{
				std::cout << "UMTS" << std::endl;
				break;
			}
			case RILTDSCDMAKIND::RIL_TDSCDMAKIND_HSDPA:
			{
				std::cout << "HSDPA" << std::endl;
				break;
			}
			case RILTDSCDMAKIND::RIL_TDSCDMAKIND_HSUPA:
			{
				std::cout << "HSUPA" << std::endl;
				break;
			}
			case RILTDSCDMAKIND::RIL_TDSCDMAKIND_HSPAPLUS:
			{
				std::cout << "HSPA PLUS" << std::endl;
				break;
			}
			case RILTDSCDMAKIND::RIL_TDSCDMAKIND_DC_HSPAPLUS:
			{
				std::cout << "DC HSPA PLUS" << std::endl;
				break;
			}
			}
		}
		else if (regstatusinfo.ratAccessTechnology.kindUnion.dwUmtsKind != NULL)
		{
			std::cout << "System type: UMTS/" << std::endl;
			switch (regstatusinfo.ratAccessTechnology.kindUnion.dwUmtsKind)
			{
			case RILUMTSKIND::RIL_UMTSKIND_UMTS:
			{
				std::cout << "UMTS" << std::endl;
				break;
			}
			case RILUMTSKIND::RIL_UMTSKIND_HSDPA:
			{
				std::cout << "HSDPA" << std::endl;
				break;
			}
			case RILUMTSKIND::RIL_UMTSKIND_HSUPA:
			{
				std::cout << "HSUPA" << std::endl;
				break;
			}
			case RILUMTSKIND::RIL_UMTSKIND_HSPAPLUS:
			{
				std::cout << "HSPA PLUS" << std::endl;
				break;
			}
			case RILUMTSKIND::RIL_UMTSKIND_DC_HSPAPLUS:
			{
				std::cout << "DC HSPA PLUS" << std::endl;
				break;
			}
			case RILUMTSKIND::RIL_UMTSKIND_HSPAPLUS_64QAM:
			{
				std::cout << "HSPA PLUS 64QAM" << std::endl;
				break;
			}
			}
		}

		std::cout << "Network Code: ";
		printf("dwParams=%lu , dwExecutor=%lu , dwMCC=%lu , dwMNC=%lu , dwSID=%lu , dwNID=%lu , dwRI=%lu \n",
			regstatusinfo.rncNetworkCode.dwParams,
			regstatusinfo.rncNetworkCode.dwExecutor,
			regstatusinfo.rncNetworkCode.dwMCC,
			regstatusinfo.rncNetworkCode.dwMNC,
			regstatusinfo.rncNetworkCode.dwSID,
			regstatusinfo.rncNetworkCode.dwNID,
			regstatusinfo.rncNetworkCode.dwRI);

		std::cout << "Current operator:" << std::endl;

		std::cout << "Country code: ";
		wprintf_s(regstatusinfo.ronCurrentOperator.wszCountryCode);
		std::cout << std::endl;

		std::cout << "Long name: ";
		wprintf_s(regstatusinfo.ronCurrentOperator.wszLongName);
		std::cout << std::endl;

		std::cout << "Num name: ";
		wprintf_s(regstatusinfo.ronCurrentOperator.wszNumName);
		std::cout << std::endl;

		std::cout << "Short name: ";
		wprintf_s(regstatusinfo.ronCurrentOperator.wszShortName);
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	PrintBanner();

	// Initialize the event that we use to get notified of the results of every call
	ResultEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"ResultEvent");

	DWORD version = 0;
	BOOL result = GetDriverVersion(&version);
	if (result)
	{
		std::cout << "RIL Driver version is: 0x" << std::hex << version << std::endl;
	}
	std::cout << std::endl;

	result = GetDevCaps(5);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	DWORD radioconfiguration = 0;
	result = GetRadioConfiguration(&radioconfiguration);
	if (result)
	{
		std::cout << "RIL Radio configuration is: 0x" << std::hex << radioconfiguration << std::endl;
	}
	std::cout << std::endl;

	WCHAR* serialnumbergw = nullptr;
	DWORD lengthstr = 0;

	result = GetDeviceInfo(0, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_GW, &serialnumbergw, &lengthstr);
	if (result)
	{
		std::cout << "IMEI is: ";
		wprintf_s(serialnumbergw);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	RILUICCSLOTINFO slotinfo;
	result = EnumerateSlots(&slotinfo);
	if (result)
	{
		std::cout << "Number of slots: " << slotinfo.dwNumOfUiccSlots << std::endl;
		std::cout << "Slot info param mask: ";

		if (slotinfo.dwParams == RIL_PARAM_SLOTINFO_NUMSLOTS)
		{
			std::cout << "number of slots" << std::endl;
		}
		else if (slotinfo.dwParams == RIL_PARAM_SLOTINFO_SLOTSTATE)
		{
			std::cout << "state of slots" << std::endl;
		}
		else if (slotinfo.dwParams == RIL_PARAM_SLOTINFO_ALL)
		{
			std::cout << "all" << std::endl;
		}
		else
		{
			std::cout << "unknown" << std::endl;
		}

		for (DWORD i = 0; i < slotinfo.dwNumOfUiccSlots; i++)
		{
			std::cout << std::endl << "Slot " << i << std::endl;
			std::cout << "Slot state: ";

			switch (slotinfo.dwSlotState[i])
			{
			case RIL_UICCSLOT_OFF_EMPTY:
			{
				std::cout << "off and empty" << std::endl;
				break;
			}
			case RIL_UICCSLOT_OFF:
			{
				std::cout << "off" << std::endl;
				break;
			}
			case RIL_UICCSLOT_EMPTY:
			{
				std::cout << "empty" << std::endl;
				break;
			}
			case RIL_UICCSLOT_NOT_READY:
			{
				std::cout << "not ready" << std::endl;
				break;
			}
			case RIL_UICCSLOT_ACTIVE:
			{
				std::cout << "active" << std::endl;
				break;
			}
			case RIL_UICCSLOT_ERROR:
			{
				std::cout << "error" << std::endl;
				break;
			}
			}
			std::cout << std::endl;

			RILUICCCARDINFO cardinfo;
			result = GetCardInfo(i, &cardinfo);
			if (result)
			{
				std::cout << "Virtual card: " << cardinfo.fIsVirtualCard << std::endl;
				std::cout << "ICCID: ";

				for (unsigned long i = 0; i < 10; i++)
				{
					BYTE b = cardinfo.IccId[i];
					std::cout << "0x" << std::hex << (int)b << " ";
				}
				std::cout << std::endl;

				std::cout << "Number of apps: " << cardinfo.dwNumApps << std::endl;

				for (DWORD i = 0; i < cardinfo.dwNumApps; i++)
				{
					RILUICCAPPINFO appinfo = cardinfo.AppInfo[i];
					// todo;
				}
			}
		}
	}
	std::cout << std::endl;

	RILEQUIPMENTSTATE equipmentstate;
	result = GetEquipmentState(&equipmentstate);
	if (result)
	{
		std::cout << "Equipment state: ";
		switch (equipmentstate)
		{
		case RIL_EQSTATE_MINIMUM:
		{
			std::cout << "minimum" << std::endl;
			break;
		}
		case RIL_EQSTATE_FULL:
		{
			std::cout << "full" << std::endl;
			break;
		}
		case RIL_EQSTATE_SHUTDOWN:
		{
			std::cout << "shutdown" << std::endl;
			break;
		}
		}
	}
	std::cout << std::endl;

	BYTE* emergencynumberslist = nullptr;
	result = GetAllEmergencyNumbers(&emergencynumberslist);
	if (result)
	{
		DWORD dwRilENSize = *(DWORD*)(emergencynumberslist + 4);
		std::cout << "Number of emergency numbers: " << dwRilENSize << std::endl;

		std::cout << std::endl;
		DWORD size = 0;

		for (DWORD i = 0; i < dwRilENSize; i++)
		{
			RILEMERGENCYNUMBER en = *(RILEMERGENCYNUMBER*)(emergencynumberslist + sizeof(DWORD) * 2 + size);
			size += en.cbSize;

			// do parammask
			std::cout << "Executor: " << en.dwExecutor << std::endl;
			std::cout << "Size: " << en.cbSize << std::endl;

			std::cout << "Number: ";
			wprintf_s(en.wszEmergencyNumber);
			std::cout << std::endl;

			std::cout << "Category: ";
			switch (en.dwCategory)
			{
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_ALL:
			{
				std::cout << "all" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_AMBULANCE:
			{
				std::cout << "ambulance" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_AUTO_ECALL:
			{
				std::cout << "auto ecall" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_FIRE_BRIGADE:
			{
				std::cout << "fire brigade" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_MANUAL_ECALL:
			{
				std::cout << "manual ecall" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_MARINE_GUARD:
			{
				std::cout << "marine guard" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_MOUNTAIN_RESCUE:
			{
				std::cout << "mountain rescue" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_POLICE:
			{
				std::cout << "police" << std::endl;
				break;
			}
			default:
			{
				std::cout << "unspecified" << std::endl;
			}
			}
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;

	result = GetDevCaps(1);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	DisplayCurrentRegStatus();

	//getsignalquality 0
	//getpositioninfo 0
	//enablemodemfilters 0 (not implemented in wmril, normal)

	RILEXECUTORRFSTATE executorrfstate;
	result = SetExecutorRFState(0, &executorrfstate);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	WCHAR* manufacturer = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(0, RILDEVICEINFORMATION::RIL_DEVICEINFO_MANUFACTURER, &manufacturer, &lengthstr);
	if (result)
	{
		std::cout << "Manufacturer is: ";
		wprintf_s(manufacturer);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	//OpenUiccLogicalChannel(); no idea why this is broken for now

	RILUICCFILEPATH filePath = { 0x00000001lu, 0x00000002lu, { 0x3f00, 0x2f05, 0xd101, 0x0000, 0x08f0, 0x0000, 0x0000, 0x0000 } };
	RILUICCRECORDSTATUS recordstatus;
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	RILEXECUTORCONFIG RilExecutorConfig{ 0x14, 1, 1, { 0x00010002, 0 } };
	result = SetExecutorConfig(0, &RilExecutorConfig);
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	// GetPositionInfo 0

	//RIL_COMMAND_GETMSGINUICCSTATUS hugebuffer
	//GETSUBSCRIBERNUMBERS  0x2, 0x0, 0x1, 0x0

	// GETIMSI 0x2, 0x0, 0x1, 0x0

	WCHAR* model = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(0, RILDEVICEINFORMATION::RIL_DEVICEINFO_MODEL, &model, &lengthstr);
	if (result)
	{
		std::cout << "Model is: ";
		wprintf_s(model);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	result = SendRestrictedUiccCmd1();
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	emergencynumberslist = nullptr;
	result = GetAllEmergencyNumbers(&emergencynumberslist);
	if (result)
	{
		DWORD dwRilENSize = *(DWORD*)(emergencynumberslist + 4);
		std::cout << "Number of emergency numbers: " << dwRilENSize << std::endl;

		std::cout << std::endl;
		DWORD size = 0;

		for (DWORD i = 0; i < dwRilENSize; i++)
		{
			RILEMERGENCYNUMBER en = *(RILEMERGENCYNUMBER*)(emergencynumberslist + sizeof(DWORD) * 2 + size);
			size += en.cbSize;

			// do parammask
			std::cout << "Executor: " << en.dwExecutor << std::endl;
			std::cout << "Size: " << en.cbSize << std::endl;

			std::cout << "Number: ";
			wprintf_s(en.wszEmergencyNumber);
			std::cout << std::endl;

			std::cout << "Category: ";
			switch (en.dwCategory)
			{
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_ALL:
			{
				std::cout << "all" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_AMBULANCE:
			{
				std::cout << "ambulance" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_AUTO_ECALL:
			{
				std::cout << "auto ecall" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_FIRE_BRIGADE:
			{
				std::cout << "fire brigade" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_MANUAL_ECALL:
			{
				std::cout << "manual ecall" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_MARINE_GUARD:
			{
				std::cout << "marine guard" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_MOUNTAIN_RESCUE:
			{
				std::cout << "mountain rescue" << std::endl;
				break;
			}
			case RILEMERGENCYNUMBERCATEGORY::RIL_ENUM_POLICE:
			{
				std::cout << "police" << std::endl;
				break;
			}
			default:
			{
				std::cout << "unspecified" << std::endl;
			}
			}
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;

	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange

	std::cout << "Drumroll..." << std::endl;
	result = ResetModem(0, TRUE);
	if (result)
	{
		std::cout << "Modem reset with settings saved!" << std::endl;
	}
	std::cout << std::endl;

	result = SetEquipmentState(2);
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//getuicclockstate
	//getuiccappersocheckstate

	WCHAR* revision = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(0, RILDEVICEINFORMATION::RIL_DEVICEINFO_REVISION, &revision, &lengthstr);
	if (result)
	{
		std::cout << "Revision is: ";
		wprintf_s(revision);
		std::cout << std::endl;
	}
	std::cout << std::endl;


	//watchuicc
	//watchuicc
	//getuicclockstate


	serialnumbergw = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(0, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_GW, &serialnumbergw, &lengthstr);
	if (result)
	{
		std::cout << "IMEI is: ";
		wprintf_s(serialnumbergw);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	//getimsi

	WCHAR* serialnumbercdma = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(0, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_CDMA, &serialnumbercdma, &lengthstr);
	if (result)
	{
		std::cout << "MEID is: ";
		wprintf_s(serialnumbercdma);
		std::cout << std::endl;
	}
	std::cout << std::endl;


	result = SetSlotPower(0, TRUE);
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//getsubnumbers
	//getuiccrecordstatus
	//getuiccprlid
	//getimsi

	DisplayCurrentRegStatus();

	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange
	//getuiccrecordstatus
	//getuiccrecordstatus
	//getuiccrecordstatus
	//getpersodeactstate
	//getsubnumbers
	//getuiccrecordstatus
	//getuiccprlid
	//getimsi

	DisplayCurrentRegStatus();

	//getuiccrecordstatus
	//getuiccrecordstatus
	//getuiccrecordstatus
	//getuiccrecordstatus

	//sendrestricteduiccmd bigbuffer
	//sendrestricteduiccmd bigbuffer
	//sendrestricteduiccmd bigbuffer
	//sendrestricteduiccmd bigbuffer


	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange

	//getuiccrecordstatus
	//getuiccrecordstatus

	//sendrestricteduiccmd bigbuffer

	//getimsi
	//getmsginuiccstatus

	DisplayCurrentRegStatus();

	result = SetSlotPower(0, TRUE);
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//writeadditionalnumberstring
}
