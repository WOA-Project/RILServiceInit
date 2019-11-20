#include <Windows.h>
#include "WNFHandler.h"
#include <iostream>

extern "C"
{
	NTSTATUS NTAPI NtQueryWnfStateData(
			_In_ PWNF_STATE_NAME StateName,
			_In_opt_ PWNF_TYPE_ID TypeId,
			_In_opt_ const VOID* ExplicitScope,
			_Out_ PWNF_CHANGE_STAMP ChangeStamp,
			_Out_writes_bytes_to_opt_(*BufferSize, *BufferSize) PVOID Buffer,
			_Inout_ PULONG BufferSize);

	NTSTATUS NTAPI NtUpdateWnfStateData(
			_In_ PWNF_STATE_NAME StateName,
			_In_reads_bytes_opt_(Length) const VOID* Buffer,
			_In_opt_ ULONG Length,
			_In_opt_ PCWNF_TYPE_ID TypeId,
			_In_opt_ const PVOID ExplicitScope,
			_In_ WNF_CHANGE_STAMP MatchingChangeStamp,
			_In_ ULONG CheckStamp);
}

void WNFHandler::WriteConfiguredLineData(DWORD dwCan, BYTE* ICCID)
{
	WNF_CELL_CONFIGURED_LINES_CAN_STRUCT configuredLine;

	memcpy(configuredLine.ICCID1, ICCID, 10);
	memcpy(configuredLine.ICCID2, ICCID, 10);

	WNF_STATE_NAME stateName = WNF_CELL_CONFIGURED_LINES_CAN0;
	if (dwCan == 1)
	{
		stateName = WNF_CELL_CONFIGURED_LINES_CAN1;
	}

	HRESULT nError = NtUpdateWnfStateData(&stateName, &configuredLine, 0x74, nullptr, nullptr, 0, 0);

	if (nError != ERROR_SUCCESS)
	{
		std::cout << "Failed to write WNF line registration information for phone service using NtUpdateWnfStateData. hResult=" << std::hex << nError << std::endl;
	}
}
