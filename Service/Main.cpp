#include "Main.h"

int _tmain(int argc, TCHAR* argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		//return GetLastError();
		// The application was ran by the user
		// So we run our main procedure

		return ServiceWorkerThread(NULL);
	}

	return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	DWORD Status = E_FAIL;
	HANDLE hThread;

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	/*
	 * Perform tasks neccesary to start the service here
	 */

	 // Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Start the thread that will perform the main task of the service
	hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	/*
	 * Perform any cleanup tasks
	 */
	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

EXIT:
	return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		 * Perform tasks neccesary to stop the service here
		 */

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}
}

#include <iostream>
#include <Windows.h>
#include "rilapitypes.h"
#include "rilapi.h"
#include "Wwapi.h"

#pragma region Function implementations

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

BOOL GetDriverVersion(DWORD dwMinVersion, DWORD dwMaxVersion, DWORD* version)
{
	std::cout << "[Function Implementation] " << "Gathering Driver Version" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetDriverVersion(hRil, &context, dwMinVersion, dwMaxVersion);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != sizeof(DWORD))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*version = *(DWORD*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetDevCaps(DWORD dwCapsType)
{
	std::cout << "[Function Implementation] " << "Gathering Device Capabilities" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetRadioConfiguration(DWORD* radioConfiguration)
{
	std::cout << "[Function Implementation] " << "Gathering Radio Configuration" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != sizeof(DWORD))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*radioConfiguration = *(DWORD*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetDeviceInfo(DWORD dwExecutor, RILDEVICEINFORMATION dwDeviceInformation, WCHAR** result, DWORD* length)
{
	std::cout << "[Function Implementation] " << "Gathering Device Information" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	*result = (WCHAR*)RlpData;
	*length = RcbData / sizeof(WCHAR);

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL EnumerateSlots(RILUICCSLOTINFO* slotinfo)
{
	std::cout << "[Function Implementation] " << "Enumerating slots" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(DWORD) * 4)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*slotinfo = *(RILUICCSLOTINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetCardInfo(DWORD dwSlotIndex, RILUICCCARDINFO* cardinfo)
{
	std::cout << "[Function Implementation] " << "Getting Card info" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(DWORD) * 3 + sizeof(BOOL) + sizeof(BYTE) * 10)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*cardinfo = *(RILUICCCARDINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetEquipmentState(RILEQUIPMENTSTATE* equipmentstate)
{
	std::cout << "[Function Implementation] " << "Getting Equipment state" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != sizeof(DWORD))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*equipmentstate = *(RILEQUIPMENTSTATE*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetAllEmergencyNumbers(BYTE** emergencynumberslist)
{
	std::cout << "[Function Implementation] " << "Getting all emergency numbers" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(DWORD) * 2)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*emergencynumberslist = (BYTE*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetCurrentRegStatus(DWORD dwExecutor, RILREGSTATUSINFO* regstatusinfo)
{
	std::cout << "[Function Implementation] " << "Getting current registration status" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILREGSTATUSINFO))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*regstatusinfo = *(RILREGSTATUSINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetExecutorRFState(DWORD dwExecutor, RILEXECUTORRFSTATE* executorrfstate)
{
	std::cout << "[Function Implementation] " << "Getting executor RF state" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetExecutorRFState(hRil, &context, dwExecutor);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILEXECUTORRFSTATE))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*executorrfstate = *(RILEXECUTORRFSTATE*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetUiccRecordStatus(const RILUICCFILEPATH* lpFilePath, RILUICCRECORDSTATUS* recordstatus)
{
	std::cout << "[Function Implementation] " << "Getting UICC Record Status" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCRECORDSTATUS))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*recordstatus = *(RILUICCRECORDSTATUS*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetExecutorConfig(DWORD dwExecutor, const RILEXECUTORCONFIG* lpRilExecutorConfig)
{
	std::cout << "[Function Implementation] " << "Setting Executor Config" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SendRestrictedUiccCmd(RILUICCCOMMAND dwCommand, const RILUICCCMDPARAMETERS* lpParameters, const BYTE* lpbData, DWORD dwSize, const RILUICCLOCKCREDENTIAL* lpLockVerification)
{
	std::cout << "[Function Implementation] " << "Sending Restricted Uicc Cmd" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SendRestrictedUiccCmd(hRil, &context, dwCommand, lpParameters, lpbData, dwSize, lpLockVerification);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCRESPONSE))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetExecutorRFState(DWORD dwExecutor, BOOL fExecutorRFState)
{
	std::cout << "[Function Implementation] " << "Setting Executor RF State" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetExecutorRFState(hRil, &context, dwExecutor, fExecutorRFState);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetEquipmentState(DWORD dwEquipmentState)
{
	std::cout << "[Function Implementation] " << "Setting equipment state" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetSlotPower(DWORD dwSlotIndex, BOOL fPowerOn)
{
	std::cout << "[Function Implementation] " << "Setting slot power" << std::endl;

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

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetDMProfileConfigInfo(DWORD dwExecutor, DWORD dwConfigItem, const RILDMCONFIGINFOVALUE* rciValue)
{
	std::cout << "[Function Implementation] " << "Setting DM Profile Config Information" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetDMProfileConfigInfo(hRil, &context, dwExecutor, dwConfigItem, rciValue);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL SetCellBroadcastMsgConfig(HUICCAPP hUiccApp, const RILCBMSGCONFIG* lpCbMsgConfigInfo)
{
	std::cout << "[Function Implementation] " << "Setting Cellular Broadcast Message Configuration" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_SetCellBroadcastMsgConfig(hRil, &context, hUiccApp, lpCbMsgConfigInfo);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData != 0)
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetSignalQuality(DWORD dwExecutor, RILSIGNALQUALITY** signalquality, DWORD* length)
{
	std::cout << "[Function Implementation] " << "Getting signal quality" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetSignalQuality(hRil, &context, dwExecutor);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILSIGNALQUALITY))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*length = RcbData;
	*signalquality = *(RILSIGNALQUALITY**)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetIMSStatus(DWORD dwExecutor, RILIMSSTATUS* imsstatus)
{
	std::cout << "[Function Implementation] " << "Getting IMS status" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetIMSStatus(hRil, &context, dwExecutor);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILIMSSTATUS))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*imsstatus = *(RILIMSSTATUS*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetUiccATR(DWORD dwSlotIndex, RILUICCATRINFO* uiccatrinfo)
{
	std::cout << "[Function Implementation] " << "Getting UICC ATR" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetUiccATR(hRil, &context, dwSlotIndex);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCATRINFO))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*uiccatrinfo = *(RILUICCATRINFO*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}

BOOL GetSubscriberNumbers(HUICCAPP hUiccApp, RILUICCSUBSCRIBERNUMBERS* uiccsubscribernumbers)
{
	std::cout << "[Function Implementation] " << "Getting subscriber numbers" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetSubscriberNumbers(hRil, &context, hUiccApp);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCSUBSCRIBERNUMBERS))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*uiccsubscribernumbers = *(RILUICCSUBSCRIBERNUMBERS*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}


BOOL GetUiccAppPersoCheckState(HUICCAPP hUiccApp, RILUICCAPPPERSOCHECKSTATUS* uiccapppersocheckstatus)
{
	std::cout << "[Function Implementation] " << "Getting UICC Application Personalisation Check State" << std::endl;

	DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
	wchar_t clientName[] = L"RILClient";
	HRIL hRil;
	DWORD context = 20;

	HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hr = RIL_GetUiccAppPersoCheckState(hRil, &context, hUiccApp);
	if (hr != ERROR_SUCCESS)
	{
		return FALSE;
	}

	std::cout << "[Function Implementation] " << "Waiting for the RIL function to return a result via our callback." << std::endl;
	DWORD dwWaitResult = WaitForSingleObject(ResultEvent, INFINITE);
	ResetEvent(ResultEvent);

	std::cout << "[Function Implementation] " << "Result: " << std::hex << RdwCode << std::endl;

	if (*(DWORD*)RusersContext != context)
	{
		std::cout << "[Function Implementation] " << "The returned result isn't for us." << std::endl;
		return FALSE;
	}

	if (RcbData < sizeof(RILUICCAPPPERSOCHECKSTATUS))
	{
		std::cout << "[Function Implementation] " << "The size of the returned result is not the size we expected." << std::endl;
		return FALSE;
	}

	*uiccapppersocheckstatus = *(RILUICCAPPPERSOCHECKSTATUS*)RlpData;

	hr = RIL_Deinitialize(hRil);
	if (hr != ERROR_SUCCESS)
	{
		std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
	}

	return TRUE;
}


#pragma endregion

#pragma region Display functions

void PrintBanner()
{
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
	std::cout << "      `8b 88ooood8 88'  `88 88   d8' 88 88'  `\"\" 88ooood8 " << std::endl;
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

void DisplayCurrentRegStatus(DWORD dwExecutor)
{
	RILREGSTATUSINFO regstatusinfo;
	BOOL result = GetCurrentRegStatus(dwExecutor, &regstatusinfo);
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

void DisplayAllEmergencyNumbers()
{
	BYTE* emergencynumberslist = nullptr;
	BOOL result = GetAllEmergencyNumbers(&emergencynumberslist);
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
}

#pragma endregion

typedef struct _WNF_STATE_NAME
{
	ULONG Data[2];
} WNF_STATE_NAME, * PWNF_STATE_NAME;

typedef struct _WNF_TYPE_ID
{
	GUID TypeId;
} WNF_TYPE_ID, * PWNF_TYPE_ID;
typedef const WNF_TYPE_ID* PCWNF_TYPE_ID;

typedef ULONG WNF_CHANGE_STAMP, * PWNF_CHANGE_STAMP;

typedef struct WNF_CALL_REGISTRATION
{
	DWORD dwSize = 116;
	DWORD dwReserved1 = 0;
	DWORD dwReserved2 = 0;
	BYTE ICCID1[10];
	short shReserved1 = 0;
	DWORD dwReserved3 = 1;
	BYTE ICCID2[10];
	short shReserved2 = 0xA005i16;
	DWORD dwReserved4 = 0x87000000;

	//
	// These values seem unique per SIM (?)
	// Only the ones I highlighted do change between SIMs and looks like they must
	// be valid in order for calls to work properly in windows
	// No idea where they come from. Change if you have problems.
	// They are from HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Notifications\Data 0D8A0B2EA3BDF475
	// which is actually this entire struct if you take into consideration the change counter at the very beginning (DWORD size 4)
	//
	BYTE bfReserved[72] = { 16, 2, 0xFF, 0x33 /*diff*/, 0xFF, 0xFF, 0x89, 5 /*diff*/, 17 /*diff*/, 0, 0xFF, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0 };

} WNF_CALL_REGISTRATION;

extern "C"
NTSTATUS
NTAPI
NtQueryWnfStateData(
	_In_ PWNF_STATE_NAME StateName,
	_In_opt_ PWNF_TYPE_ID TypeId,
	_In_opt_ const VOID * ExplicitScope,
	_Out_ PWNF_CHANGE_STAMP ChangeStamp,
	_Out_writes_bytes_to_opt_(*BufferSize, *BufferSize) PVOID Buffer,
	_Inout_ PULONG BufferSize);

extern "C"
NTSTATUS
NTAPI
NtUpdateWnfStateData(
	_In_ PWNF_STATE_NAME StateName,
	_In_reads_bytes_opt_(Length) const VOID * Buffer,
	_In_opt_ ULONG Length,
	_In_opt_ PCWNF_TYPE_ID TypeId,
	_In_opt_ const PVOID ExplicitScope,
	_In_ WNF_CHANGE_STAMP MatchingChangeStamp,
	_In_ ULONG CheckStamp);

bool IsBuild18912OrGreater()
{
	typedef NTSTATUS(WINAPI* FRtlGetVersion)(LPOSVERSIONINFOEXW);

	HMODULE hNtdll = GetModuleHandle("ntdll.dll");
	FRtlGetVersion RtlGetVersion = (FRtlGetVersion)GetProcAddress(hNtdll, "RtlGetVersion");

	OSVERSIONINFOEXW v = { 0 };
	v.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	RtlGetVersion(&v);

	return v.dwBuildNumber >= 18912;
}

void RestartWWANSVC()
{

	//
	// Give us a bit of time to fully bring up the network before starting
	// to restart WwanSvc
	//
	// We check when WwanSvc reports that the device is fully connected and
	// then we restart the service.
	//
	HANDLE hClient = NULL;
	DWORD dwMaxClient = 1;
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;
	int iRet = 0;

	WCHAR GuidString[40] = { 0 };

	PWWAN_INTERFACE_INFO_LIST pIfList = NULL;

	dwResult = WwanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
	if (dwResult != ERROR_SUCCESS)
	{
		return;
	}

	DWORD counter = 0;
	while (true)
	{
		counter++;
		dwResult = WwanEnumerateInterfaces(hClient, NULL, &pIfList);
		if (dwResult != ERROR_SUCCESS)
		{
			return;
		}
		else
		{
			BOOL Activated = false;
			for (int i = 0; i < (int)pIfList->dwNumberOfItems; i++)
			{
				WWAN_INTERFACE_INFO pIfInfo = pIfList->InterfaceInfo[i];
				if (pIfInfo.InterfaceStatus.InterfaceState == WWAN_INTERFACE_STATE::WwanInterfaceStateActivated)
				{
					Activated = true;
				}
			}

			if (Activated)
				break;
		}
		if (counter >= 120)
		{
			if (pIfList != NULL) {
				WwanFreeMemory(pIfList);
				pIfList = NULL;
			}

			if (hClient != NULL)
				WwanCloseHandle(hClient, NULL);

			return;
		}
		Sleep(10000);
	}

	if (pIfList != NULL) {
		WwanFreeMemory(pIfList);
		pIfList = NULL;
	}

	if (hClient != NULL)
		WwanCloseHandle(hClient, NULL);

	//
	// The actual service restart portion
	//
	std::string Service = "WwanSvc";
	SERVICE_STATUS Status;

	SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE SHandle = OpenService(SCManager, Service.c_str(), SC_MANAGER_ALL_ACCESS);

	if (SHandle == NULL)
	{
		CloseServiceHandle(SCManager);
		return;
	}

	if (!ControlService(SHandle, SERVICE_CONTROL_STOP, &Status))
	{
		CloseServiceHandle(SCManager);
		CloseServiceHandle(SHandle);
		return;
	}

	do
	{
		QueryServiceStatus(SHandle, &Status);
	} while (Status.dwCurrentState != SERVICE_STOPPED);


	if (!StartService(SHandle, 0, NULL))
	{
		CloseServiceHandle(SCManager);
		CloseServiceHandle(SHandle);
		return;
	}

	do
	{
		QueryServiceStatus(SHandle, &Status);
	} while (Status.dwCurrentState != SERVICE_RUNNING);

	CloseServiceHandle(SCManager);
	CloseServiceHandle(SHandle);
}

void mainRIL()
{
	//
	// This is the main SIM UICC Application
	//
	HUICCAPP app = 0x00010002;

	//
	// This is executor for the first SIM slot
	// later we may want to handle multi sim scenarios
	//
	DWORD executor = 0;
	DWORD executor2 = 1;
	BOOL initializeSecondExecutor = FALSE;

	BOOL DisableCallReg = FALSE;

	HKEY hKey;
	LSTATUS nResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\OEM\\RILINITSVC", 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	DWORD dwData;
	DWORD cbData = sizeof(DWORD);

	bool Logging = FALSE;

	if (nResult == ERROR_SUCCESS)
	{
		nResult = ::RegQueryValueEx(hKey, "DisableCallReg", NULL, NULL, (LPBYTE)&dwData, &cbData);

		if (nResult == ERROR_SUCCESS)
			if (dwData == 1)
				DisableCallReg = TRUE;

		/*nResult = ::RegQueryValueEx(hKey, "Executor", NULL, NULL, (LPBYTE)&dwData, &cbData);

		if (nResult == ERROR_SUCCESS)
			if (dwData == 1)
				executor = dwData;*/

		RegCloseKey(hKey);
	}

	//
	// Initialize the event that we use to get notified of the results of every call
	//
	ResultEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		"ResultEvent");

	DWORD version = 0;
	BOOL result = GetDriverVersion(0x20000, 0x320000, &version);
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

	result = GetDeviceInfo(executor, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_GW, &serialnumbergw, &lengthstr);
	if (result)
	{
		std::cout << "IMEI is: ";
		wprintf_s(serialnumbergw);
		std::cout << std::endl;
	}
	else
	{
		std::cout << "Device does not have any broadband adapter, leaving." << std::endl;
		return;
	}
	std::cout << std::endl;

	result = GetDeviceInfo(executor2, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_GW, &serialnumbergw, &lengthstr);
	if (result)
	{
		std::cout << "IMEI is: ";
		wprintf_s(serialnumbergw);
		std::cout << std::endl;
		initializeSecondExecutor = TRUE;
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

					/*std::cout << "App name: ";
					printf(appinfo.cszAppName);
					std::cout << std::endl;*/

					// todo proper parsing workaround as struct size varies...;
				}

				if (i == 0 && !DisableCallReg)
				{
					std::cout << "Attempting to write WNF registration information for phone service." << std::endl;

					WNF_CALL_REGISTRATION callRegistration;

					memcpy(callRegistration.ICCID1, cardinfo.IccId, 10);
					memcpy(callRegistration.ICCID2, cardinfo.IccId, 10);

					struct CALLREGDATA {
						DWORD ModCount = 2;
						WNF_CALL_REGISTRATION callreg_struct;
					} callregdata;

					callregdata.callreg_struct = callRegistration;

					HKEY hKey;
					HRESULT nError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Notifications\\Data", NULL, KEY_ALL_ACCESS, &hKey);
					if (nError == ERROR_SUCCESS)
					{
						nError = RegSetValueEx(hKey, "0D8A0B2EA3BDF475", NULL, REG_BINARY, (LPBYTE)&callregdata, 0x78);
						if (nError == ERROR_SUCCESS)
						{
							nError = RegCloseKey(hKey);

							if (nError == ERROR_SUCCESS)
							{
								std::cout << "Successfully wrote WNF registration information for phone service." << std::endl;
							}
						}
					}

					WNF_STATE_NAME wnf_machine_store{ 0xA3BDF475, 0x0D8A0B2E };
					WNF_CHANGE_STAMP wnf_change_stamp;

					BYTE callreg_query[0x74] = { 0 };
					ULONG size;
					nError = NtQueryWnfStateData(&wnf_machine_store, nullptr, nullptr, &wnf_change_stamp, &callreg_query, &size);

					if (nError != ERROR_SUCCESS)
					{
						std::cout << "Failed to query WNF registration information for phone service. hResult=" << std::hex << nError << std::endl;
					}

					nError = NtUpdateWnfStateData(&wnf_machine_store, &callRegistration, 0x74, nullptr, nullptr, wnf_change_stamp, 1);

					if (nError != ERROR_SUCCESS)
					{
						std::cout << "Failed to write WNF registration information for phone service. hResult=" << std::hex << nError << std::endl;
					}
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

	DisplayAllEmergencyNumbers();

	result = GetDevCaps(1);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	DisplayCurrentRegStatus(executor);
	if (initializeSecondExecutor)
		DisplayCurrentRegStatus(executor2);

	RILSIGNALQUALITY* signalquality = nullptr;
	DWORD length = 0;
	result = GetSignalQuality(executor, &signalquality, &length);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetSignalQuality(executor2, &signalquality, &length);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	RILIMSSTATUS imsstatus = { 0 };
	result = GetIMSStatus(executor, &imsstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetIMSStatus(executor2, &imsstatus);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	//RIL_COMMAND_GETPSMEDIACONFIGURATION 0 (not implemented in wmril, normal)

	RILEXECUTORRFSTATE executorrfstate;
	result = GetExecutorRFState(executor, &executorrfstate);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetExecutorRFState(executor2, &executorrfstate);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	WCHAR* manufacturer = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(executor, RILDEVICEINFORMATION::RIL_DEVICEINFO_MANUFACTURER, &manufacturer, &lengthstr);
	if (result)
	{
		std::cout << "Manufacturer is: ";
		wprintf_s(manufacturer);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetDeviceInfo(executor2, RILDEVICEINFORMATION::RIL_DEVICEINFO_MANUFACTURER, &manufacturer, &lengthstr);
		if (result)
		{
			std::cout << "Manufacturer is: ";
			wprintf_s(manufacturer);
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	RILUICCATRINFO uiccatrinfo = { 0 };
	result = GetUiccATR(executor, &uiccatrinfo);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetUiccATR(executor2, &uiccatrinfo);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	RILUICCFILEPATH filePath = { 1lu, 2lu, { 0x3f00, 0x2f05 } };
	RILUICCRECORDSTATUS recordstatus;
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	RILEXECUTORCONFIG RilExecutorConfig = { 20, 1, 1, { app, 0 } };
	result = SetExecutorConfig(executor, &RilExecutorConfig);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = SetExecutorConfig(executor2, &RilExecutorConfig);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	imsstatus = { 0 };
	result = GetIMSStatus(executor, &imsstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetIMSStatus(executor2, &imsstatus);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	BYTE setmsgbroadcastconfigparamsbuffer[] = {
		2, 0, 1, 0, 0xc0, 4, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0,
		1, 0, 0, 0, 0x32, 0, 0, 0, 0x32, 0, 0, 0, 1, 0, 0, 0,
		7, 2, 0, 0, 7, 2, 0, 0, 1, 0, 0, 0, 0x97, 3, 0, 0,
		0x97, 3, 0, 0, 1, 0, 0, 0, 0, 0x11, 0, 0, 2, 0x11, 0, 0,
		1, 0, 0, 0, 0x12, 0x11, 0, 0, 0x1d, 0x11, 0, 0, 1, 0, 0, 0,
		0x1f, 0x11, 0, 0, 0x2a, 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0
	};

	RILSETCELLBROADCASTMSGCONFIGPARAMS params2 = *(RILSETCELLBROADCASTMSGCONFIGPARAMS*)setmsgbroadcastconfigparamsbuffer;

	result = SetCellBroadcastMsgConfig(app, &params2.rmCBConfig);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	RILUICCSUBSCRIBERNUMBERS uiccsubscribernumbers = { 0 };
	result = GetSubscriberNumbers(app, &uiccsubscribernumbers); if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//GETIMSI hUiccApp (main)

	WCHAR* model = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(executor, RILDEVICEINFORMATION::RIL_DEVICEINFO_MODEL, &model, &lengthstr);
	if (result)
	{
		std::cout << "Model is: ";
		wprintf_s(model);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetDeviceInfo(executor2, RILDEVICEINFORMATION::RIL_DEVICEINFO_MODEL, &model, &lengthstr);
		if (result)
		{
			std::cout << "Model is: ";
			wprintf_s(model);
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	RILUICCCMDPARAMETERS uiccCommandParameters;
	uiccCommandParameters.cbSize = 0x2C;
	uiccCommandParameters.dwParams = 9;
	filePath = { 0 };
	filePath.hUiccApp = 1;
	filePath.dwFilePathLen = 2;
	WORD wFilePath[2] = { 0x053F, 0xD12F };
	memcpy(filePath.wFilePath, wFilePath, 2);
	uiccCommandParameters.filePath = filePath;
	RILUICCLOCKCREDENTIAL lockVertification = { 0 };

	result = SendRestrictedUiccCmd(RILUICCCOMMAND::RIL_UICCCMD_READBINARY, &uiccCommandParameters, NULL, NULL, &lockVertification);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	DisplayAllEmergencyNumbers();

	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange

	//
	// Turn on antennas for our slot
	//
	result = SetExecutorRFState(executor, TRUE);
	if (result)
	{
		std::cout << "Ok" << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = SetExecutorRFState(executor2, TRUE);
		if (result)
		{
			std::cout << "Ok" << std::endl;
		}
		std::cout << std::endl;
	}

	result = SetEquipmentState(2);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//getuicclockstate

	RILUICCAPPPERSOCHECKSTATUS uiccapppersocheckstatus = { 0 };
	result = GetUiccAppPersoCheckState(app, &uiccapppersocheckstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	WCHAR* revision = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(executor, RILDEVICEINFORMATION::RIL_DEVICEINFO_REVISION, &revision, &lengthstr);
	if (result)
	{
		std::cout << "Revision is: ";
		wprintf_s(revision);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetDeviceInfo(executor2, RILDEVICEINFORMATION::RIL_DEVICEINFO_REVISION, &revision, &lengthstr);
		if (result)
		{
			std::cout << "Revision is: ";
			wprintf_s(revision);
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	//watchuicc
	//watchuicc
	//getuicclockstate

	serialnumbergw = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(executor, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_GW, &serialnumbergw, &lengthstr);
	if (result)
	{
		std::cout << "IMEI is: ";
		wprintf_s(serialnumbergw);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetDeviceInfo(executor2, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_GW, &serialnumbergw, &lengthstr);
		if (result)
		{
			std::cout << "IMEI is: ";
			wprintf_s(serialnumbergw);
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	//getimsi

	WCHAR* serialnumbercdma = nullptr;
	lengthstr = 0;

	result = GetDeviceInfo(executor, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_CDMA, &serialnumbercdma, &lengthstr);
	if (result)
	{
		std::cout << "MEID is: ";
		wprintf_s(serialnumbercdma);
		std::cout << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = GetDeviceInfo(executor2, RILDEVICEINFORMATION::RIL_DEVICEINFO_SERIALNUMBER_CDMA, &serialnumbercdma, &lengthstr);
		if (result)
		{
			std::cout << "MEID is: ";
			wprintf_s(serialnumbercdma);
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	//
	// Turn on power for the SIM slot
	//
	result = SetSlotPower(executor, TRUE);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = SetSlotPower(executor2, TRUE);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	uiccsubscribernumbers = { 0 };
	result = GetSubscriberNumbers(app, &uiccsubscribernumbers); if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	filePath = { app, 2lu, { 0x7fff, 0x6fc7 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;


	//getuiccprlid
	//getimsi

	DisplayCurrentRegStatus(executor);
	if (initializeSecondExecutor)
		DisplayCurrentRegStatus(executor2);

	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange
	//watchuiccfilechange

	filePath = { app, 2lu, { 0x7fff, 0x6f46 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;


	filePath = { app, 2lu, { 0x7fff, 0x6f3e } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	filePath = { app, 2lu, { 0x7fff, 0x6fc5 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//getpersodeactstate

	uiccsubscribernumbers = { 0 };
	result = GetSubscriberNumbers(app, &uiccsubscribernumbers); if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;


	filePath = { app, 2lu, { 0x7fff, 0x6fc7 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//getuiccprlid
	//getimsi

	DisplayCurrentRegStatus(executor);
	if (initializeSecondExecutor)
		DisplayCurrentRegStatus(executor2);

	filePath = { app, 2lu, { 0x7fff, 0x6f3e } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	filePath = { app, 2lu, { 0x7fff, 0x6f46 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	filePath = { app, 2lu, { 0x7fff, 0x6fc5 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	filePath = { app, 2lu, { 0x7fff, 0x6f17 } };
	recordstatus = { 0 };
	result = GetUiccRecordStatus(&filePath, &recordstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//sendrestricteduiccmd bigbuffer
	//sendrestricteduiccmd bigbuffer

	//getuiccrecordstatus

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

	//
	// TODO: parse this struct
	//
	BYTE rilCellularBroadcastMessageConfigurationBuffer[] = {
		0xc0, 4, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0,
		1, 0, 0, 0, 0x32, 0, 0, 0, 0x32, 0, 0, 0, 1, 0, 0, 0,
		7, 2, 0, 0, 7, 2, 0, 0, 1, 0, 0, 0, 0x97, 3, 0, 0,
		0x97, 3, 0, 0, 1, 0, 0, 0, 0, 0x11, 0, 0, 2, 0x11, 0, 0,
		1, 0, 0, 0, 0x12, 0x11, 0, 0, 0x1d, 0x11, 0, 0, 1, 0, 0, 0,
		0x1f, 0x11, 0, 0, 0x2a, 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0
	};

	RILCBMSGCONFIG rilCellularBroadcastMessageConfiguration = *(RILCBMSGCONFIG*)rilCellularBroadcastMessageConfigurationBuffer;

	//
	// Set the cellular broadcast message configuration for the main UICC application
	//
	result = SetCellBroadcastMsgConfig(app, &rilCellularBroadcastMessageConfiguration);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	DisplayCurrentRegStatus(executor);
	if (initializeSecondExecutor)
	{
		DisplayCurrentRegStatus(executor2);
	}

	//
	// Turn on power for the SIM slot
	//
	result = SetSlotPower(executor, TRUE);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = SetSlotPower(executor2, TRUE);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	//
	// Set the DM profile configuration information to disabled for type 0x210
	//
	RILDMCONFIGINFOVALUE additional = { 528, RILDMCONFIGINFOTYPE::RIL_DMCV_TYPE_BOOLEAN, FALSE, 0, NULL };
	result = SetDMProfileConfigInfo(executor, 32, &additional);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	if (initializeSecondExecutor)
	{
		result = SetDMProfileConfigInfo(executor2, 32, &additional);
		if (result)
		{
			std::cout << "Ok." << std::endl;
		}
		std::cout << std::endl;
	}

	//
	// Restart service
	// WwanSvc won't pick up new changes from the MBB driver
	// if we don't force it to reload, so do that
	// (otherwise the data connection won't be established)
	//
	RestartWWANSVC();

	//
	// We're done
	//
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	//
	// First begin to check if we are on a problematic
	// Windows version with no RIL support in WWAN-SVC
	//
	if (!IsBuild18912OrGreater())
	{
		// We don't need to run then
		return ERROR_SUCCESS;
	}

	//
	// Print banner for the CLI tool
	//
	PrintBanner();

	//
	// Loop until RILAdaptation is loaded and ready
	// In case we get told to stop by the service control, stop.
	//
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		//
		// We need to give RIL a little bit of extra time, 7.5 seconds looks like the sweet spot
		// for ril to be fully loaded in time when we get loaded by the OS
		//
		Sleep(7500);

		//
		// wait until RIL is initialized
		//
		DWORD enabledNotifications[] = { RIL_NCLASS_FUNCRESULT, RIL_NCLASS_NOTIFICATIONS };
		wchar_t clientName[] = L"RILClient";
		HRIL hRil;
		DWORD context = 20;

		//
		// Attempt to initialize RIL communication via ril proxy
		//
		HRESULT hr = RIL_Initialize(0, RILResultCallback, RILNotifyCallback, enabledNotifications, 2, &context, clientName, &hRil);
		if (hr != ERROR_SUCCESS)
		{
			//
			// If we failed, continue to loop
			//
			continue;
		}

		//
		// Deinitialize our link with ril proxy
		// We don't need it anymore in this function
		// NOTE: this doesn't cause MSRil to unload the ril, it just cuts communication with us
		//
		hr = RIL_Deinitialize(hRil);
		if (hr != ERROR_SUCCESS)
		{
			std::cout << "[Function Implementation] " << "Unable to close handle" << std::endl;
		}

		//
		// Proceed to RIL initialization
		//
		mainRIL();

		//
		// Break out of the loop, we're done
		//
		break;
	}

	return ERROR_SUCCESS;
}