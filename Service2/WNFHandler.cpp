#include <Windows.h>
#include "WNFHandler.h"
#include <iostream>

extern "C"
{
	NTSTATUS NTAPI RtlPublishWnfStateData(
		_In_ WNF_STATE_NAME StateName,
		_In_opt_ PCWNF_TYPE_ID TypeId,
		_In_reads_bytes_opt_(Length) const VOID* Buffer,
		_In_opt_ ULONG Length,
		_In_opt_ const PVOID ExplicitScope);
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

	HRESULT nError = RtlPublishWnfStateData(stateName, nullptr, &configuredLine, 0x74, nullptr);

	if (nError != ERROR_SUCCESS)
	{
		std::cout << "Failed to write WNF line registration information for phone service using NtUpdateWnfStateData. hResult=" << std::hex << nError << std::endl;
	}
}

void WNFHandler::WriteBlankConfiguredLineData(DWORD dwCan)
{
	WNF_CELL_CONFIGURED_LINES_CAN_STRUCT configuredLine = { 0 };

	configuredLine.dwSize = 116;
	configuredLine.dwReserved1 = 1;

	WNF_STATE_NAME stateName = WNF_CELL_CONFIGURED_LINES_CAN0;
	if (dwCan == 1)
	{
		stateName = WNF_CELL_CONFIGURED_LINES_CAN1;
	}

	HRESULT nError = RtlPublishWnfStateData(stateName, nullptr, &configuredLine, 0x74, nullptr);

	if (nError != ERROR_SUCCESS)
	{
		std::cout << "Failed to write WNF line registration information for phone service using NtUpdateWnfStateData. hResult=" << std::hex << nError << std::endl;
	}
}
