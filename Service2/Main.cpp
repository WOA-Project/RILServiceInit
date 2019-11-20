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

#pragma endregion


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
		std::cout << "Service restart: Waiting..." << std::endl;
		Sleep(1000);
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

#include "WNFHandler.h"

void mainRIL()
{
	BOOL result;
	BOOL DisableCallReg = FALSE;

	//
	// This is the main SIM UICC Application
	//
	HUICCAPP app = 0x00010002;

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

	WNFHandler wnfHandler;

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
				if (!DisableCallReg)
				{
					std::cout << "Attempting to write WNF registration information for phone service." << std::endl;
					wnfHandler.WriteConfiguredLineData(i, cardinfo.IccId);
				}

				std::cout << "Enabling the current executor." << std::endl;
				RILEXECUTORCONFIG RilExecutorConfig = { 20, RIL_EXECUTORFLAG_ENABLED, 1, { app, 0 } };
				result = SetExecutorConfig(i, &RilExecutorConfig);
				if (result)
				{
					std::cout << "Ok." << std::endl;
				}
				std::cout << std::endl;

				BOOL enable = TRUE;

				RILEXECUTORRFSTATE rfstate;
				result = GetExecutorRFState(i, &rfstate);
				if (result)
				{
					std::cout << "Ok" << std::endl;
					enable = !rfstate.fExecutorRFState;
				}
				std::cout << std::endl;

				//
				// Turn on antennas for our slot
				//
				if (enable)
				{

				}
				std::cout << "Turning on the antenna for the current executor." << std::endl;
				result = SetExecutorRFState(i, TRUE);
				if (result)
				{
					std::cout << "Ok" << std::endl;
				}
				std::cout << std::endl;

				Sleep(1000);

				DisplayCurrentRegStatus(i);
			}
		}
	}
	std::cout << std::endl;
	
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