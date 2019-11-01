// RILTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18995.0/um/rilapitypes.h"


#pragma region RILImports

#define NCLASS_FROM_NOTIFICATION(code)              ((code) & NOTIFICATION_CLASS_MASK)

typedef HANDLE HRIL, * LPHRIL;

typedef void (CALLBACK* RILRESULTCALLBACK)(
	HRIL            hRil,         // @parm RIL handle to current RIL instance
	DWORD           dwCode,       // @parm result code
	LPVOID          usersContext, // @parm users context assigned by the call that originated this response
	const void* lpData,       // @parm data associated with the notification
	DWORD           cbData,       // @parm size of the strcuture pointed to lpData
	LPVOID          lpParam       // @parm parameter passed to <f RIL_Initialize>
	);

typedef void (CALLBACK* RILNOTIFYCALLBACK)(
	HRIL  hRil,             // @parm RIL handle to current RIL instance
	DWORD dwCode,           // @parm notification code
	const void* lpData,     // @parm data associated with the notification
	DWORD cbData,           // @parm size of the strcuture pointed to lpData
	LPVOID lpParam          // @parm parameter passed to <f RIL_Initialize> 
	);

typedef HRESULT(*FRIL_Initialize)(
	_In_ DWORD dwIndex,                      // @param index of the RIL port to use )(e.g., 1 for RIL1:)
	_In_ RILRESULTCALLBACK pfnResult,        // @param function result callback
	_In_ RILNOTIFYCALLBACK pfnNotify,        // @param notification callback
	_In_ DWORD* lpdwNotifications,           // @param array of notifications to be enabled for this client
	_In_ DWORD dwNotificationCount,          // @param the number of notifications to be enabled for this client    
	_In_ LPVOID lpParam,                     // @param custom parameter passed to result and notification callbacks
	_In_z_ WCHAR* pwszClientName,            // @param the name of the RIL client
	_Out_ HRIL* lphRil                       // @param returned handle to RIL instance
	);

// de-initializes the RIL, which ultimately results in a CloseHandle to the MSRIL file handle held by the RIL proxy. It does not directly result in a request to the IHVRIL.
typedef HRESULT(*FRIL_Deinitialize)(
	HRIL hRil                           // @param handle to an RIL instance returned by <f RIL_Initialize>
	);

// returns the number of logical modem devices )(instances of the RIL driver). This function is implemented in the RIL proxy and does not result in a RIL_IOControl request.
typedef HRESULT(*FRIL_GetNumberOfModems)(
	DWORD* lpdwNumModem
	);

// enables additional classes of notifications for a client. This is an asynchronous call, but it is implemented internally within the RIL proxy and MSRIL, and does not result in a RIL_IOControl request to IHVRIL. 
typedef HRESULT(*FRIL_EnableNotifications)(
	HRIL hRil,                          // @param handle to RIL instance returned by <f RIL_Initialize>
	LPVOID usersContext,                // @param given returned the call back function to identify this particular call
	DWORD* lpdwNotifications,           // @param array of notifications to be enabled for this client
	DWORD dwNotificationCount           // @param the number of notifications to be enabled for this client    
	);

// disables classes of notifications for a client. This is an asynchronous call, but it is implemented internally within the RIL proxy and MSRIL, and does not result in a RIL_IOControl request to IHVRIL.
typedef HRESULT(*FRIL_DisableNotifications)(
	HRIL hRil,                          // @param handle to RIL instance returned by <f RIL_Initialize>
	LPVOID usersContext,                // @param given returned the call back function to identify this particular call
	DWORD* lpdwNotifications,           // @param array of notifications to be disable for this client
	DWORD dwNotificationCount           // @param the number of notifications to be disable for this client    
	);


//////////////////////////////////////////////////
// Asynchronous Functions and Notifications
//////////////////////////////////////////////////

// retrieves the driver version information.
typedef HRESULT(*FRIL_GetDriverVersion)(HRIL hRil, LPVOID lpContext, DWORD dwMinVersion, DWORD dwMaxVersion);
// RIL_COMMAND_GETDRIVERVERSION
// In: RILGETDRIVERVERSIONPARAMS
// Async out: DWORD

// queries to retrieve specific device capabilities.
typedef HRESULT(*FRIL_GetDevCaps)(HRIL hRil, LPVOID lpContext, DWORD dwCapsType);
// RIL_COMMAND_GETDEVCAPS
// In: DWORD
// Async out: varies
// dwCapsType    Specifies the capability to be queried. These are itemized in the following sections.

// returns specific device information as requested by the caller
typedef HRESULT(*FRIL_GetDeviceInfo)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILDEVICEINFORMATION dwId);
// RIL_COMMAND_GETDEVICEINFO
// In: DEVICEINFO_PARAMS
// Async out: array of WCHAR


//////////////////////////////////////////////////
// Device State
//////////////////////////////////////////////////

// retrieves the current equipment state. This function can be called to determine both if the radio is on and what features are ready.
typedef HRESULT(*FRIL_GetEquipmentState)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETEQUIPMENTSTATE
// In: none
// Async out: RILEQUIPMENTSTATE

// sets the radio equipment state. It is called to turn the radio on or off. It is also used to quiesce the driver and modem and save all state as part of a system shutdown.
typedef HRESULT(*FRIL_SetEquipmentState)(HRIL hRil, LPVOID lpContext, DWORD dwEquipmentState);
//RIL_COMMAND_SETEQUIPMENTSTATE
	// In: DWORD
	// Async out: none )(status only)

// used to enable or disable certain notifications at the modem. When the application processor goes into a low power state with the backlight off it can turn off certain notifications such as signal quality reports in order to save current. When a notification is transitioned from enable to disable. The modem shall keep track of the change in notifications even if they are disabled. If the source of a notification at the modem side changes state while disabled, it shall send the new state immediately after being enabled.
typedef HRESULT(*FRIL_SetNotificationFilterState)(HRIL hRil, LPVOID lpContext, DWORD dwFilterMask, DWORD dwFilterState);
// RIL_COMMAND_SETNOTIFICATIONFILTERSTATE
// In: SETNOTIFICATIONFILTERSTATE_PARAMS
// Async out: none )(status only)

// retrieves the current notification filter state.
typedef HRESULT(*FRIL_GetNotificationFilterState)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETNOTIFICATIONFILTERSTATE
// In: None
// Async Out: DWORD dwFilterStateMask 

// requests a modem reset.
typedef HRESULT(*FRIL_ResetModem)(HRIL hRil, LPVOID lpContext, RILRESETMODEMKIND dwResetKind);
// RIL_COMMAND_RESETMODEM
// In: RILRESETMODEMKIND
// Async out: none )(status only)

//////////////////////////////////////////////////
// UICC Slots and Cards
//////////////////////////////////////////////////

// returns information about the number of UICC slots and the state of each.
typedef HRESULT(*FRIL_EnumerateSlots)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_ENUMERATESLOTS
// In: none
// Async out: RILUICCSLOTINFO

// retrieves information about the UICC card in a specified slot.
typedef HRESULT(*FRIL_GetCardInfo)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex);
// RIL_COMMAND_GETCARDINFO
// In: DWORD
// Async out: RILUICCCARDINFO

// enables/disables power to a specified UICC card slot.
typedef HRESULT(*FRIL_SetSlotPower)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, BOOL fPowerOn);
// RIL_COMMAND_SETSLOTPOWER
// In: SETSLOTPOWER_PARAMS
// Async out: none )(status only)

// retrieves information about a specified UICC file.
typedef HRESULT(*FRIL_GetUiccRecordStatus)(HRIL hRil, LPVOID lpContext, const RILUICCFILEPATH* lpFilePath);
// RIL_COMMAND_GETUICCRECORDSTATUS
// In: RILUICCFILEPATH
// Async out: RILUICCRECORDSTATUS

// sends a specified restricted command to the UICC.
typedef HRESULT(*FRIL_SendRestrictedUiccCmd)(HRIL hRil, LPVOID lpContext, RILUICCCOMMAND dwCommand, const RILUICCCMDPARAMETERS* lpParameters, const BYTE* lpbData, DWORD dwSize, const RILUICCLOCKCREDENTIAL* lpLockVerification);
// RIL_COMMAND_SENDRESTRICTEDCOMMAND
// In: SENDRESTRICTEDUICCCMD_PARAMS
// Async out: RILUICCRESPONSE

// specifies a set of files to RIL driver for which data change notifications are expected. Each call replaces the old list of files watched. On bootup or on UICC power toggle, the RIL driver will not persist the file list. To watch the files, the API will have to be called in these conditions.
typedef HRESULT(*FRIL_WatchUiccFileChange)(HRIL hRil, LPVOID lpContext, const RILUICCFILES* lpUiccFiles);
// RIL_COMMAND_WATCHUICCFILECHANGE
// In: RILUICCFILES
// Async out: RILUICCRESPONSE

// returns information pertaining to an UICC application's PRL ID.
typedef HRESULT(*FRIL_GetUiccPRLID)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETUICCPRLID
// In: HUICCAPP
// Async out: DWORD

// retrieves the International Mobile Subscriber Identity )(IMSI) of the specified UICC application.
typedef HRESULT(*FRIL_GetIMSI)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETIMSI
// In: HUICCAPP
// Async out: array characters

// retrieves the subscriber numbers from the specified UICC application.
typedef HRESULT(*FRIL_GetSubscriberNumbers)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETSUBSCRIBERNUMBERS
// In: HUICCAPP
// Async out: RILUICCSUBSCRIBERNUMBERS

//retrieves the Answer To Reset message that was received from a UICC card in a specified slot
typedef HRESULT(*FRIL_GetUiccATR)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex);
// RIL_COMMAND_GETUICCATR
// In: DWORD
// Async out: RILUICCATRINFO

//opens a logical channel on the specified UICC by selecting a UICC application using its application identifier
typedef HRESULT(*FRIL_OpenUiccLogicalChannel)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, DWORD dwChannelGroup, DWORD dwAppIdLength, const BYTE* pbAppId, DWORD dwSelectP2Arg);
// RIL_COMMAND_OPENUICCLOGICALCHANNEL
// In: RILOPENUICCLOGICALCHANNELPARAMS
// Async out: RILOPENUICCLOGICALCHANNELINFO

//closes an open logical channel on a specified UICC card
typedef HRESULT(*FRIL_CloseUiccLogicalChannel)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, DWORD dwChannelId);
// RIL_COMMAND_CLOSEUICCLOGICALCHANNEL
// In: RILCLOSEUICCLOGICALCHANNELPARAMS
// Async out: none )(status only)

//closes all open logical channels in a specified group on a specified UICC card
typedef HRESULT(*FRIL_CloseUiccLogicalChannelGroup)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, DWORD dwChannelGroup);
// RIL_COMMAND_CLOSEUICCLOGICALCHANNELGROUP
// In: RILCLOSEUICCLOGICALCHANNELGROUPPARAMS
// Async out: none )(status only)

//exchanges an Application Protocol Data Unit with a specified UICC card over a specified channel
typedef HRESULT(*FRIL_ExchangeUiccAPDU)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, DWORD dwChannelId, DWORD dwAPDULength, const BYTE* bAPDU);
// RIL_COMMAND_EXCHANGEUICCAPDU
// In: RILEXCHANGEUICCAPDUPARAMS
// Async out: RILEXCHANGEUICCAPDURESPONSE

/////////////////////////////////////
// Locking and UICC Security
/////////////////////////////////////

// queries the state of a UICC lock, asynchronously returning a RILUICCLOCKSTATE. In the case of a virtual R-UIM and applications which do not have any lock associated with them )(indicated by key references set to 0s in RILUICCAPPINFO), the asynchronous result will indicate an error RIL_E_INVALIDUICCKEYREF.
typedef HRESULT(*FRIL_GetUiccLockState)(HRIL hRil, LPVOID lpContext, const RILUICCLOCK* lpRilUiccLock);
// RIL_COMMAND_GETUICCLOCKSTATE
// In: RILUICCLOCK
// Async out: RILUICCLOCKSTATE

// queries a service to get the service lock, if any. For services such as fixed dialing this represents the PIN2 state for that operation.
typedef HRESULT(*FRIL_GetUiccServiceLock)(HRIL hRil, LPVOID lpContext, const RILUICCSERVICE* lpService);
// RIL_COMMAND_GETUICCSERVICE
// In: RILUICCSERVICE
// Async out: RILUICCLOCK

typedef HRESULT(*FRIL_VerifyUiccLock)(HRIL hRil, LPVOID lpContext, const RILUICCLOCKCREDENTIAL* lpVerification);
// RIL_COMMAND_VERIFYUICCLOCK
// In: RILUICCLOCKCREDENTIAL
// Async out: none )(status only)

// enables or disables a UICC lock.
typedef HRESULT(*FRIL_SetUiccLockEnabled)(HRIL hRil, LPVOID lpContext, const RILUICCLOCKCREDENTIAL* lpLockCredential, BOOL fEnable);
// RIL_COMMAND_SETUICCLOCKENABLED
// In: RILSETUICCLOCKENABLED_PARAMS
// Async out: none )(status only)

// unblocks a UICC lock that has been blocked by too many failed verification attempts and sets a new PIN password.
typedef HRESULT(*FRIL_UnblockUiccLock)(HRIL hRil, LPVOID lpContext, const RILUICCLOCKCREDENTIAL* lpLockCredential, LPCSTR lpszNewPassword);
// RIL_COMMAND_UNBLOCKUICCLOCK
// In: RILUNBLOCKUICCLOCK_PARAMS
// Async out: none )(status only)

// changes the lock verification password. )(This is the PIN password; there is no API to change the PUK password.)
typedef HRESULT(*FRIL_ChangeUiccLockPassword)(HRIL hRil, LPVOID lpContext, const RILUICCLOCKCREDENTIAL* lpLockCredential, LPCSTR lpszNewPassword);
// RIL_COMMAND_CHANGEUICCLOCKPASSWORD
// In: CHANGEUICCLOCKPASSWORD_PARAMS
// Async out: none )(status only)

// gets the personalization check state of a UICC application.
typedef HRESULT(*FRIL_GetUiccAppPersoCheckState)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETUICCAPPPERSOCHECKSTATE
// In: HUICCAPP
// Async out: RILUICCAPPPERSOCHECKSTATUS

// gets the deactivation state information for a specific perso feature.
typedef HRESULT(*FRIL_GetPersoDeactivationState)(HRIL hRil, LPVOID lpContext, DWORD dwPersoFeature);
// RIL_COMMAND_GETPERSODEACTIVATIONSTATE
// In: DWORD
// Async out: RILPERSODEACTIVATIONSTATE

// deactivates a perso lock on the device.
typedef HRESULT(*FRIL_DeactivatePerso)(HRIL hRil, LPVOID lpContext, DWORD dwPersoFeature, LPCSTR lpszPassword);
// RIL_COMMAND_DEACTIVATEPERSO
// In: DEACTIVATEPERSO_PARAMS
// Async out: none )(status only)

///////////////////////
// FDN
/////////////////////
typedef HRESULT(*FRIL_GetUiccServiceState)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILUICCSERVICESERVICE dwService);
// RIL_COMMAND_GETUICCSERVICESTATE
// In: RILUICCSERVICE
// Async out: RILUICCSERVICESTATE

typedef HRESULT(*FRIL_SetUiccServiceState)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILUICCSERVICESERVICE dwService, const RILUICCLOCKCREDENTIAL* lpLockCredential, BOOL fEnable);
// RIL_COMMAND_SETUICCSERVICESTATE
// In: RILUICCSERVICEPARAMS
// Async out: RILUICCSERVICESTATE


////////////////////////
// Phonebook
////////////////////////

// reads phonebook entries from the specified range of indices of the current storage location.
typedef HRESULT(*FRIL_ReadPhonebookEntries)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILPHONEENTRYSTORELOCATION dwStoreLocation, DWORD dwStartIndex, DWORD dwEndIndex);
// RIL_COMMAND_READPHONEBOOKENTRIES
// In: READPHONEBOOKENTRIES_PARAMS
// Async out: array of RILPHONEBOOKENTRY

// writes a phone book entry to the current storage location.
typedef HRESULT(*FRIL_WritePhonebookEntry)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILPHONEENTRYSTORELOCATION dwStoreLocation, const RILPHONEBOOKENTRY* lpEntry, const RILUICCLOCKCREDENTIAL* lpLockVerification);
// RIL_COMMAND_WRITEPHONEBOOKENTRY
// In: WRITEPHONEBOOKENTRY_PARAMS
// Async out: none )(status only)

// deletes a phonebook entry from the current storage location.
typedef HRESULT(*FRIL_DeletePhonebookEntry)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILPHONEENTRYSTORELOCATION dwStoreLocation, DWORD dwIndex, const RILUICCLOCKCREDENTIAL* lpLockVerification);
// RIL_COMMAND_DELETEPHONEBOOKENTRY
// In: DELETEPHONEBOOKENTRY_PARAMS
// Async out: none )(status only)

// retrieves the current phonebook options.
typedef HRESULT(*FRIL_GetPhonebookOptions)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILPHONEENTRYSTORELOCATION dwStoreLocation);
// RIL_COMMAND_GETPHONEBOOKOPTIONS
// In: GETPHONEBOOKOPTIONS_PARAMS
// Async out: RILPHONEBOOKINFO

// retrieves the additional numbers.
typedef HRESULT(*FRIL_GetAllAdditionalNumberStrings)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETALLADDITIONALNUMBERSTRINGS
// In: GETALLADDITIONALNUMBERSTRINGS_PARAMS
// Async out: RILPHONEBOOKADDITIONALNUMBERINFO

// writes an additional number string for the UICC phonebook
typedef HRESULT(*FRIL_WriteAdditionalNumberString)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, const RILPHONEBOOKADDITIONALNUMBERSTRING* lpRilPBANS);
// RIL_COMMAND_WRITEADDITIONALNUMBERSTRING
// In: RILWRITEADDITIONALNUMBERSTRINGPARAMS
// Async out: none )(status only)

// deletes an additional number string for the UICC phonebook
typedef HRESULT(*FRIL_DeleteAdditionalNumberString)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, DWORD dwNumId);
//RIL_COMMAND_DELETEADDITIONALNUMBERSTRING
// In: RILDELETEADDITIONALNUMBERSTRINGPARAMS
// Async out: none )(status only)

// retrieves all the emergency numbers known by the modem. This includes emergency numbers provisioned in the modem itself and all emergency numbers defined by the UICC cards attached to that modem.
typedef HRESULT(*FRIL_GetAllEmergencyNumbers)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETALLEMERGENCYNUMBERS
// In: none
// Async out: RILEMERGENCYNUMBERSLIST 


////////////////////////////////////////
// Radio Network Registration
////////////////////////////////////////

// specifies the configuration to be used by the RIL instance.  This setting persists until the function is called again.  Default configuration index used is 0.
typedef HRESULT(*FRIL_SetRadioConfiguration)(HRIL hRil, LPVOID lpContext, DWORD dwConfigIdx);
// RIL_COMMAND_SETRADIOCONFIGURATION
// In: DWORD
// Async out: none )(status only)

// gets the current configuration used by the RIL instance.
typedef HRESULT(*FRIL_GetRadioConfiguration)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETRADIOCONFIGURATION
// In: none
// Async out: DWORD

// assigns subscription configuration parameters to an executor.
typedef HRESULT(*FRIL_SetExecutorConfig)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, const RILEXECUTORCONFIG* lpRilExecutorConfig);
// RIL_COMMAND_SETEXECUTORCONFIG
// In: SETEXECUTORCONFIG_PARAMS
// Async out: none )(status only)

// retrieves the configuration parameters that are currently associated with an executor.
typedef HRESULT(*FRIL_GetExecutorConfig)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETEXECUTORCONFIG
// In: DWORD
// Async out: RILEXECUTORCONFIG

// assigns user preference for radio system selection to an executor.
typedef HRESULT(*FRIL_SetSystemSelectionPrefs_V1)(HRIL hRil, LPVOID lpContext, RILSETSYSTEMSELECTIONPREFSFLAG dwFlags, const RILSYSTEMSELECTIONPREFS_V1* lpRilSystemSelectionPrefs);
// RIL_COMMAND_SETSYSTEMSELECTIONPREFS
// In: SETSYSTEMSELECTIONPREFS_PARAMS_V1
// Async out: none )(status only)

// assigns user preference for radio system selection to an executor.
typedef HRESULT(*FRIL_SetSystemSelectionPrefs)(HRIL hRil, LPVOID lpContext, RILSETSYSTEMSELECTIONPREFSFLAG dwFlags, const RILSYSTEMSELECTIONPREFS* lpRilSystemSelectionPrefs);
// RIL_COMMAND_SETSYSTEMSELECTIONPREFS
// In: SETSYSTEMSELECTIONPREFS_PARAMS
// Async out: none )(status only)

// retrieves the current system selection preferences from the RIL.
typedef HRESULT(*FRIL_GetSystemSelectionPrefs)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETSYSTEMSELECTIONPREFS
// In: DWORD
// Async out: RILSYSTEMSELECTIONPREFS

// request RIL driver to do an active radio scan, and returns the list of available operators.
typedef HRESULT(*FRIL_GetOperatorList)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, DWORD dwSystemTypes);
// RIL_COMMAND_GETOPERATORLIST
// In: GETOPERATORLIST_PARAMS
// Async out: array of RILOPERATORINFO

// retrieves the list of preferred operators.
typedef HRESULT(*FRIL_GetPreferredOperatorList)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, RILGETPREFERENCEDOPERATORLISTFORMAT dwFormat);
// RIL_COMMAND_GETPREFERREDOPERATORLIST
// In: GETPREFERREDOPERATORLIST_PARAMS
// Async out: array of RILOPERATORINFO

// Set the list of preferred operators.
typedef HRESULT(*FRIL_SetPreferredOperatorList)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, DWORD dwPreferredListSize, RILOPERATORNAMES* lpPreferredList);
// RIL_COMMAND_SETPREFERREDOPERATORLIST
// In:  RILSETPREFERREDOPERATORLISTPARAMS 
// Async out:  none )(status only)

// retrieves the radio network registration status, for a specified subscription supported by current radio configuration.
typedef HRESULT(*FRIL_GetCurrentRegStatus)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETCURRENTREGSTATUS
// In: DWORD
// Async out: RILREGSTATUSINFO


//////////////////////////////////////
// Signal Strength and NITZ
//////////////////////////////////////

// retrieves information about the received signal quality. 
typedef HRESULT(*FRIL_GetSignalQuality)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETSIGNALQUALITY
// In: DWORD dwExecutor
// Async out: array of 1 or 2 RILSIGNALQUALITY structures


////////////////////////////////////////////////////
// Card Application Toolkit )(UICC Toolkit)
////////////////////////////////////////////////////

// indicates to the RIL driver that the application processor is ready to handle UICC toolkit events )(notably the initial SETUP MENU).
typedef HRESULT(*FRIL_RegisterUiccToolkitService)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex);
// RIL_COMMAND_REGISTERUICCTOOLKITSERVICE
// In: DWORD
// Async out: none )(status only)

// sends the UICC Toolkit Profile to the radio. The sent profile will be stored in the radio/IHVRIL permanent storage and is not sent to the UICC Card until the next reset.
typedef HRESULT(*FRIL_SetUiccToolkitProfile)(HRIL hRil, LPVOID lpContext, const RILUICCTOOLKITPROFILE* lpToolkitProfile);
// RIL_COMMAND_SETUICCTOOLKITPROFILE
// In: RILSETUICCTOOLKITPROFILEPARAMS
// Async out: none )(status only)

// retrieves the toolkit profile. When called following RIL_SetUiccToolkitProfile, this function returns the profile downloaded and stored in the IHV RIL/modem and not the currently-active profile.
typedef HRESULT(*FRIL_GetUiccToolkitProfile)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETUICCTOOLKITPROFILE
// In: DWORD
// Async out: RILUICCTOOLKITPROFILE

// sends a response to an executed UICC toolkit command.
typedef HRESULT(*FRIL_SendUiccToolkitCmdResponse)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, const LPBYTE pbDetails, DWORD dwDetailSize);
// RIL_COMMAND_SENDUICCTOOLKITCMDRESPONSE
// In: RILSENDUICCTOOLKITCMDRESPONSEPARAMS
// Async out: none )(status only)

// sends an envelope command to the UICC. The envelope command is used by the application processor mainly for Menu Selection, Event Download and Call Control.
typedef HRESULT(*FRIL_SendUiccToolkitEnvelope)(HRIL hRil, LPVOID lpContext, DWORD dwSlotIndex, const LPBYTE pbEnvelope, DWORD dwEnvelopeSize);
// RIL_COMMAND_SENDUICCTOOLKITENVELOPE
// In: RILSENDUICCTOOLKITENVELOPEPARAMS
// Async out: none )(status only)

/////////////////////////////////////////
// Call Management )(Telephony)
/////////////////////////////////////////

// responsible for dialing circuit switched voice calls.
typedef HRESULT(*FRIL_Dial_V1)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, const RILADDRESS* lpraAddress, DWORD dwOptions);
// RIL_COMMAND_DIAL
// In: RILDIALPARAMS_V1
// Async out: DWORD

// responsible for dialing circuit switched or packet switched voice calls.
typedef HRESULT(*FRIL_Dial)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, const RILADDRESS* lpraAddress, DWORD dwOptions, RILCALLTYPE dwType, const LPRILCALLMEDIAOFFERANSWERSET lprcmOfferAnswer);
// RIL_COMMAND_DIAL
// In: RILDIALPARAMS_V2
// Async out: DWORD

// modifies the state of circuit switched calls.
typedef HRESULT(*FRIL_ManageCalls_V1)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILMANAGECALLPARAMSCOMMAND dwCommand, DWORD dwID);
// RIL_COMMAND_MANAGECALLS
// In: RILMANAGECALLSPARAMS_V1
// Async out: none )(status only)

// modifies the state of circuit switched or packet switched calls.
typedef HRESULT(*FRIL_ManageCalls_V2)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILMANAGECALLPARAMSCOMMAND dwCommand, DWORD dwID, const LPRILCALLMEDIAOFFERANSWERSET lprcmOfferAnswer);
// RIL_COMMAND_MANAGECALLS
// In: RILMANAGECALLSPARAMS_V2
// Async out: none )(status only)

// modifies the state of circuit switched or packet switched calls.
typedef HRESULT(*FRIL_ManageCalls_V3)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILMANAGECALLPARAMSCOMMAND dwCommand, DWORD dwID, const LPRILCALLMEDIAOFFERANSWERSET lprcmOfferAnswer, const LPRILADDRESS lpraAddress);
// RIL_COMMAND_MANAGECALLS
// In: RILMANAGECALLSPARAMS_V3
// Async out: none )(status only)

// modifies the state of circuit switched or packet switched calls.
typedef HRESULT(*FRIL_ManageCalls)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILMANAGECALLPARAMSCOMMAND dwCommand, DWORD dwID, const LPRILCALLMEDIAOFFERANSWERSET lprcmOfferAnswer, const LPRILADDRESS lpraAddress, const LPRILCALLRTT lpstRTTInfo);
// RIL_COMMAND_MANAGECALLS
// In: RILMANAGECALLSPARAMS_V4
// Async out: none )(status only)

typedef HRESULT(*FRIL_GetCallList)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
//RIL_COMMAND_GETCALLLIST
// In: DWORD
// Async out: RILCALLLIST

// controls the modem emergency mode. It is used to exit the CDMA emergency call back mode. It is also used to indicate to one modem in a dual active system that another modem is an emergency mode. 
typedef HRESULT(*FRIL_EmergencyModeControl)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILEMERGENCYMODECONTROLPARAMSCONTROL dwEmergencyModeControl);
// RIL_COMMAND_EMERGENCYMODECONTROL
// In: EMERGENCYMODECONTROL_PARAMS
// Async out: none )(status only)

// retrieves current call forwarding rules.
typedef HRESULT(*FRIL_GetCallForwardingSettings)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLFORWARDINGSETTINGSREASON dwReason, BOOL fAllClasses, DWORD dwInfoClasses);
// RIL_COMMAND_GETCALLFORWARDINGSETTINGS
// In: GETCALLFORWARDING_PARAMS
// Async out: array of RILCALLFORWARDINGSETTINGS

// enables or disables the specified call forwarding rule.
typedef HRESULT(*FRIL_SetCallForwardingStatus)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLFORWARDINGSETTINGSREASON dwReason, BOOL fAllClasses, DWORD dwInfoClasses, RILSERVICESETTINGSSTATUS dwStatus);
// RIL_COMMAND_SETCALLFORWARDINGSTATUS
// In: SETCALLFORWARDINGSTATUS_PARAMS
// Async out: none )(status only)

// adds a call forwarding rule.
typedef HRESULT(*FRIL_AddCallForwarding)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLFORWARDINGSETTINGSREASON dwReason, const RILCALLFORWARDINGSETTINGS* lpSettings);
// RIL_COMMAND_ADDCALLFORWARDING
// In: ADDCALLFORWARDING_PARAMS
// Async out: none )(status only)

// removes a call forwarding rule.
typedef HRESULT(*FRIL_RemoveCallForwarding)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLFORWARDINGSETTINGSREASON dwReason, DWORD dwInfoClasses);
// RIL_COMMAND_REMOVECALLFORWARDING
// In: REMOVECALLFORWARDING_PARAMS
// Async out: none )(status only)

// retrieves the status of the specified type of call barring.
typedef HRESULT(*FRIL_GetCallBarringStatus)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLBARRINGSTATUSPARAMSTYPE dwType, BOOL fAllClasses, DWORD dwInfoClasses);
// RIL_COMMAND_GETCALLBARRINGSTATUS
// In: GETCALLBARRINGSTATUS_PARAMS
// Async out: DWORD

// enables or disables the specified type of call barring.
typedef HRESULT(*FRIL_SetCallBarringStatus)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLBARRINGSTATUSPARAMSTYPE dwType, BOOL fAllClasses, DWORD dwInfoClasses, LPCSTR lpszPassword, RILCALLBARRINGSTATUSPARAMSSTATUS dwStatus);
// RIL_COMMAND_SETCALLBARRINGSTATUS
// In: SETCALLBARRINGSTATUS_PARAMS
// Async out: none )(status only)

// changes the password for the specified type of call barring.
typedef HRESULT(*FRIL_ChangeCallBarringPassword)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, RILCALLBARRINGSTATUSPARAMSTYPE dwType, LPCSTR lpwszOldPassword, LPCSTR lpwszNewPassword, LPCSTR lpszConfirmPassword);
// RIL_COMMAND_CHANGECALLBARRINGPASSWORD
// In: CHANGECALLBARRINGPASSWORD_PARAMS
// Async out: none )(status only)

// retrieves information classes for which call waiting is currently enabled.
typedef HRESULT(*FRIL_GetCallWaitingSettings)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, BOOL fAllClasses, DWORD dwInfoClasses);
// RIL_COMMAND_GETCALLWAITINGSETTINGS
// In: GETCALLWAITINGSETTINGS_PARAMS        
// Async out: DWORD

// enables or disables call waiting for the specified information class.
typedef HRESULT(*FRIL_SetCallWaitingStatus)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, BOOL fAllClasses, DWORD dwInfoClasses, RILSERVICESETTINGSSTATUS dwStatus);
// RIL_COMMAND_SETCALLWAITINGSTATUS
// In: SETCALLWAITINGSTATUS_PARAMS
// Async out: none )(status only)

// retrieves the current caller ID settings.
typedef HRESULT(*FRIL_GetCallerIdSettings)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETCALLERIDSETTINGS
// In: DWORD
// Async out: RILCALLERIDSETTINGS

// retrieves the current settings for the dialed ID.
typedef HRESULT(*FRIL_GetDialedIdSettings)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETDIALEDIDSETTINGS
// In: DWORD
// Async out: RILDIALEDIDSETTINGS

// retrieves the current settings for the Hide Connected ID option.
typedef HRESULT(*FRIL_GetHideConnectedIdSettings)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETHIDECONNECTEDIDSETTINGS
// In: DWORD
// Async out: RILHIDECONNECTEDIDSETTINGS

// retrieves the current settings for the Hide ID option.
typedef HRESULT(*FRIL_GetHideIdSettings)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETHIDEIDSETTINGS
// In: DWORD
// Async out: RILHIDEIDSETTINGS

// sends a flash message. A flash message updates the state of active, held and waiting calls on a CDMA device.
typedef HRESULT(*FRIL_SendFlash)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, LPRILADDRESS lpraRilAddress);
// RIL_COMMAND_SENDFLASH
// In: RILADDRESS
// Async out: none )(status only)

// sends Phase 1 unstructured supplementary service )(USSD) data.
typedef HRESULT(*FRIL_SendSupServiceData)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, const WCHAR* lpwszData);
// RIL_COMMAND_SENDSUPSERVICEDATA
// In: SENDSUPSERVICEDATA_PARAMS
// Async out: none )(status only)

// sends unstructured supplementary service )(USSD) data response to a USSD request from the network.
typedef HRESULT(*FRIL_SendSupServiceDataResponse)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, const WCHAR* lpwszData);
// RIL_COMMAND_SENDSUPSERVICEDATA
// In: SENDSUPSERVICEDATARESPONSE_PARAMS
// Async out: none )(status only)

// cancels an ongoing unstructured supplementary service )(USSD) data session.
typedef HRESULT(*FRIL_CancelSupServiceDataSession)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_SENDSUPSERVICEDATA
// In: CANCELSUPSERVICEDATA_PARAMS
// Async out: none )(status only)

// Responsible for sending RTT data text on packet switched calls.
typedef HRESULT(*FRIL_SendRTT)(HRIL hRil, LPVOID lpContext, DWORD dwID, DWORD dwExecutor, const WCHAR* lpwszRTTText);
// RIL_COMMAND_SENDRTT
// In: RILSENDRTTDATAPARAMS
// Async out: none )(status only)

//////////////////////////////////
// Audio and DTMF
//////////////////////////////////

// sends Dual-Tone Multifrequency )(DTMF) tones across an established voice call.
typedef HRESULT(*FRIL_SendDTMF)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, LPCSTR lpszChars, DWORD dwDigitOnTimeMs, DWORD dwDigitOffTimeMs);
// RIL_COMMAND_SENDDTMF
// In: SENDDTMF_PARAMS
// Async out: none )(status only)

// starts a DTMF tone across an established voice call. The tone is played until it is stopped with RIL_StopDTMF.
typedef HRESULT(*FRIL_StartDTMF)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, CHAR ch);
// RIL_COMMAND_STARTDTMF
// In: STARTDTMF_PARAMS
// Async out: none

// stops DTMF tones across an established voice call.
typedef HRESULT(*FRIL_StopDTMF)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_STOPDTMF
// In: DWORD
// Async out: none


///////////////////////////
// Messaging
///////////////////////////

// gets the current messaging service options.
typedef HRESULT(*FRIL_GetMsgServiceOptions)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETMSGSERVICEOPTIONS
// In: HUICCAPP
// Async out: RILMSGSERVICEINFO

// reads a message from the current storage location.
typedef HRESULT(*FRIL_ReadMsg)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, DWORD dwIndex);
// RIL_COMMAND_READMSG
// In: READMSG_PARAMS
// Async out: RILMESSAGEINFO 

// writes a message to the current storage location.
typedef HRESULT(*FRIL_WriteMsg)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, const RILMESSAGE* lpMessage, RILMESSAGESTATUS dwStatus);
// RIL_COMMAND_WRITEMSG
// In: WRITEMSG_PARAMS
// Async out: DWORDThe input parameter is a WRITEMSG_PARAMS struct:

// deletes a message from the current storage location.
typedef HRESULT(*FRIL_DeleteMsg)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, DWORD dwIndex);
// RIL_COMMAND_DELETEMSG
// In: DELETEMSG_PARAMS
// Async out: none )(status only 

// retrieves the cell broadcast messaging configuration.
typedef HRESULT(*FRIL_GetCellBroadcastMsgConfig)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETCELLBROADCASTMSGCONFIG
// In: DWORD
// Async out: RILCBMSGCONFIG 

// turns on and off notifications for cell broadcast messages with specific IDs and language settings. The GWL structs are used for setting the GW and LTE settings and the CDMA structs are for the CDMA settings.
typedef HRESULT(*FRIL_SetCellBroadcastMsgConfig)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, const RILCBMSGCONFIG* lpCbMsgConfigInfo);
// RIL_COMMAND_SETCELLBROADCASTMSGCONFIG
// In: SETBROADCAST_PARAMS
// Async out: none )(status only)

// checks with a specific SMS message in the UICC is read or unread.
typedef HRESULT(*FRIL_GetMsgInUiccStatus)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, DWORD dwIndex);
// RIL_COMMAND_GETMSGINUICCSTATUS
// In: GETMSGINUICCSTATUS_PARAMS
// Async out: DWORD

// sets the status of a message in the UICC as read or unread.
typedef HRESULT(*FRIL_SetMsgInUiccStatus)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, DWORD dwIndex, RILMESSAGESTATUS dwStatus);
// RIL_COMMAND_SETMSGINUICCSTATUS
// In: SETMSGINUICCSTATUS_PARAMS
// Async out: none )(status only)

// tells the radio whether storage for SMS messages is available. In turn, this informs the network of the availability of storage so that the network can suspend delivery where there is no storage for new messages. 
typedef HRESULT(*FRIL_SetMsgMemoryStatus)(HRIL hRil, LPVOID lpContext, BOOL bMsgMemoryFull);
// RIL_COMMAND_SETMSGMEMORYSTATUS
// In: BOOL
// Async out: none )(status only)

// sends a message.
typedef HRESULT(*FRIL_SendMsg)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, HUICCAPP hUiccApp, const RILMESSAGE* lpMessage, DWORD dwOptions);
// RIL_COMMAND_SENDMSG
// In: SENDMSG_PARAMS
// Async out: RILSENDMSGRESPONSE

// sends a message ACK.
typedef HRESULT(*FRIL_SendMsgAck_V1)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, HUICCAPP hUiccApp, DWORD dwAckID, RILMSGACKSTATUS dwMsgStatus);
// RIL_COMMAND_SENDMSGACK
// In: SENDMSG_PARAMS
// Async out: none )(status only)

// sends a message ACK.
typedef HRESULT(*FRIL_SendMsgAck)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, HUICCAPP hUiccApp, DWORD dwAckID, RILMSGACKSTATUS dwMsgStatus, RILSMSFORMAT dwSmsFormat, RILSMSACKOPT dwOptions);
// RIL_COMMAND_SENDMSGACK
// In: SENDMSG_PARAMS
// Async out: none )(status only)

// gets the address of the Short Message Service Center for a specified UICC application.
typedef HRESULT(*FRIL_GetSMSC)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp);
// RIL_COMMAND_GETSMSC
// In: HUICCAPP
// Async out: RILADDRESS

// sets the address of the Short Message Service Center for a specified UICC application.
typedef HRESULT(*FRIL_SetSMSC)(HRIL hRil, LPVOID lpContext, HUICCAPP hUiccApp, const RILADDRESS* lpraSvcCtrAddress);
// RIL_COMMAND_SETSMSC
// In: SETSMSC_PARAMS
// Async out: none )(status only)

// queries the IMS Status.
typedef HRESULT(*FRIL_GetIMSStatus)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETIMSSTATUS
// In: DWORD
// Async out: RILIMSSTATUS

// returns device management profile information as configured in the modem
typedef HRESULT(*FRIL_GetDMProfileConfigInfo)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, DWORD dwConfigItem);
// RIL_GetDMProfileConfigInfo
// In: DWORD
// Async out: RILDMCONFIGINFOVALUE

// returns device management profile information as configured in the modem
typedef HRESULT(*FRIL_SetDMProfileConfigInfo)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, DWORD dwConfigItem, const RILDMCONFIGINFOVALUE* rciValue);
// RIL_SetDMProfileConfigInfo
// In: DWORD, RILDMCONFIGINFOVALUE
// Async out: none )(status only)


////////////////////
// Location
////////////////////

// retrieves information about location data available to the phone.
typedef HRESULT(*FRIL_GetPositionInfo)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETPOSITIONINFO
// In: DWORD
// Async out: RILPOSITIONINFO

// sets the geolocation data )(i.e. civic address and/or {latitude, longitude} position) into modem
typedef HRESULT(*FRIL_SetGeolocationData)(HRIL hRil, LPVOID lpContext, const RILSETGEOLOCATIONDATAPARAMS* pGeolocationData);
// RIL_COMMAND_SETGEOLOCATIONDATA
// In: RILSETGEOLOCATIONDATAPARAMS
// Async out: none

////////////////////////////////////////////////
// OEM Field Service and Device-Specific
////////////////////////////////////////////////

// retrieves information about radio state grouping available from the phone.
typedef HRESULT(*FRIL_GetRadioStateGroups)(HRIL hRil, LPVOID lpContext, DWORD dwParentGroupId);
// RIL_COMMAND_GETRADIOSTATEGROUPS
// In: DWORD
// Async out: RILRADIOSTATEGROUPS

typedef HRESULT(*FRIL_GetRadioStateDetails)(HRIL hRil, LPVOID lpContext, DWORD dwGroupId, DWORD dwItemId);
// RIL_COMMAND_GETRADIOSTATEDETAILS
// In: GETRADIOSTATEDETAILS_PARAMS
// Async out: RILRADIOSTATEITEMS

typedef HRESULT(*FRIL_SetRadioStateDetails)(HRIL hRil, LPVOID lpContext, const RILRADIOSTATEITEMS* pItems);
// RIL_COMMAND_SETRADIOSTATEDETAILS
// In: RILRADIOSTATEITEMS
// Async out: DWORD

typedef HRESULT(*FRIL_RadioStatePasswordCompare)(HRIL hRil, LPVOID lpContext, const RILRADIOSTATEPASSWORD* lpRspRadioStatePassword);
// RIL_COMMAND_RADIOSTATEPASSWORDCOMPARE
// In: RILRADIOSTATEPASSWORD
// Async out: DWORD

// gets the retry count remaining for a given password entry.
typedef HRESULT(*FRIL_RadioStateGetPasswordRetryCount)(HRIL hRil, LPVOID lpContext, DWORD dwPasswordId);
// RIL_COMMAND_RADIOSTATEGETPASSWORDRETRYCOUNT
// In: DWORD
// Async out: DWORD

typedef HRESULT(*FRIL_DevSpecific)(HRIL hRil, LPVOID lpContext, const BYTE* lpbParams, DWORD dwSize);
// RIL_COMMAND_DEVSPECIFIC
// In: array of BYTE
// Async out: array of BYTE

typedef HRESULT(*FRIL_SetRFState_V1)(HRIL hRil, LPVOID lpContext, DWORD dwRFState);
// RIL_COMMAND_SETRFSTATE
// In: DWORD
// Async out: none )(Status only)

typedef HRESULT(*FRIL_SetRFState)(HRIL hRil, LPVOID lpContext, const LPRILRFSTATE lpRFState);
// RIL_COMMAND_SETRFSTATE
// In: RILRFSATE
// Async out: none )(Status only)

typedef HRESULT(*FRIL_GetRFState)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETRFSTATE
// In: none
// Async out: DWORD or RILRFSTATE

typedef HRESULT(*FRIL_GetExecutorFocus)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_GETEXECUTORFOCUS
// In: none
// Async out: RILEXECUTORFOCUS

typedef HRESULT(*FRIL_SetExecutorFocus)(HRIL hRil, LPVOID lpContext, BOOL* pfFocusState, DWORD dwExecutorCount);
// RIL_COMMAND_SETEXECUTORFOCUS
// In: array of BOOL holding the focus state for all executors and the number of executors.
// Async out: none )(Status only)

typedef HRESULT(*FRIL_GetEmergencyMode)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETEMERGENCYMODE
// In: DWORD
// Async out: BOOL

typedef HRESULT(*FRIL_GetExecutorRFState)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETEXECUTORRFSTATE
// In: DWORD
// Async out: RILEXECUTORRFSTATE

typedef HRESULT(*FRIL_SetExecutorRFState)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, BOOL fExecutorRFState);
// RIL_COMMAND_SETEXECUTORRFSTATE
// In: RILSETEXECUTORRFSTATEPARAMS
// Async out: none )(Status only)

typedef HRESULT(*FRIL_EnableModemFilters)(HRIL hRil, LPVOID lpContext, DWORD filterID);
// RIL_COMMAND_ENABLEMODEMFILTERS
// In: DWORD
// Async out: none )(Status only)

typedef HRESULT(*FRIL_DisableModemFilters)(HRIL hRil, LPVOID lpContext, DWORD filterID);
// RIL_COMMAND_DISABLEMODEMFILTERS
// In: DWORD
// Async out: none )(Status only)

typedef HRESULT(*FRIL_StartModemLogs)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_STARTMODEMLOGS
// In: none
// Async out: none )(Status only)

typedef HRESULT(*FRIL_StopModemLogs)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_STOPMODEMLOGS
// In: none
// Async out: none )(Status only)

typedef HRESULT(*FRIL_DrainModemLogs)(HRIL hRil, LPVOID lpContext);
// RIL_COMMAND_DRAINMODEMLOGS
// In: none
// Async out: none )(Status only)

typedef HRESULT(*FRIL_CancelGetOperatorList)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_CANCELGETOPERATORLIST
// In: DWORD
// Async out: None )(Status Only)

typedef HRESULT(*FRIL_AvoidCDMASystem)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor, enum RILCDMAAVOIDANCEREQUESTYPE requestType);
// RIL_COMMAND_AVOIDCDMASYSTEM
// In: AVOIDCDMASYSTEM PARAMS
// Async out: None )(Status Only)

typedef HRESULT(*FRIL_SetPSMediaConfiguration)(HRIL hRil, LPVOID lpContext, const LPRILPSMEDIACONFIGURATIONSET pConfigurationSet);
// RIL_COMMAND_SETPSMEDIACONFIGURATION
// In: RILPSMEDIACONFIGURATIONSET
// Async out: RILPSMEDIACONFIGURATIONSET

typedef HRESULT(*FRIL_GetPSMediaConfiguration)(HRIL hRil, LPVOID lpContext, DWORD dwExecutor);
// RIL_COMMAND_GETPSMEDIACONFIGURATION
// In: DWORD
// Async out: RILPSMEDIACONFIGURATIONSET

FRIL_AddCallForwarding RIL_AddCallForwarding;
FRIL_AvoidCDMASystem RIL_AvoidCDMASystem;
FRIL_CancelGetOperatorList RIL_CancelGetOperatorList;
FRIL_CancelSupServiceDataSession RIL_CancelSupServiceDataSession;
FRIL_ChangeCallBarringPassword RIL_ChangeCallBarringPassword;
FRIL_ChangeUiccLockPassword RIL_ChangeUiccLockPassword;
FRIL_CloseUiccLogicalChannel RIL_CloseUiccLogicalChannel;
FRIL_CloseUiccLogicalChannelGroup RIL_CloseUiccLogicalChannelGroup;
FRIL_DeactivatePerso RIL_DeactivatePerso;
FRIL_Deinitialize RIL_Deinitialize;
FRIL_DeleteAdditionalNumberString RIL_DeleteAdditionalNumberString;
FRIL_DeleteMsg RIL_DeleteMsg;
FRIL_DeletePhonebookEntry RIL_DeletePhonebookEntry;
FRIL_DevSpecific RIL_DevSpecific;
FRIL_Dial RIL_Dial;
FRIL_Dial_V1 RIL_Dial_V1;
FRIL_DisableModemFilters RIL_DisableModemFilters;
FRIL_DisableNotifications RIL_DisableNotifications;
FRIL_DrainModemLogs RIL_DrainModemLogs;
FRIL_EmergencyModeControl RIL_EmergencyModeControl;
FRIL_EnableModemFilters RIL_EnableModemFilters;
FRIL_EnableNotifications RIL_EnableNotifications;
FRIL_EnumerateSlots RIL_EnumerateSlots;
FRIL_ExchangeUiccAPDU RIL_ExchangeUiccAPDU;
FRIL_GetAllAdditionalNumberStrings RIL_GetAllAdditionalNumberStrings;
FRIL_GetAllEmergencyNumbers RIL_GetAllEmergencyNumbers;
FRIL_GetCallBarringStatus RIL_GetCallBarringStatus;
FRIL_GetCallForwardingSettings RIL_GetCallForwardingSettings;
FRIL_GetCallWaitingSettings RIL_GetCallWaitingSettings;
FRIL_GetCallerIdSettings RIL_GetCallerIdSettings;
FRIL_GetCardInfo RIL_GetCardInfo;
FRIL_GetCellBroadcastMsgConfig RIL_GetCellBroadcastMsgConfig;
FRIL_GetCurrentRegStatus RIL_GetCurrentRegStatus;
FRIL_GetDMProfileConfigInfo RIL_GetDMProfileConfigInfo;
FRIL_GetDevCaps RIL_GetDevCaps;
FRIL_GetDeviceInfo RIL_GetDeviceInfo;
FRIL_GetDialedIdSettings RIL_GetDialedIdSettings;
FRIL_GetDriverVersion RIL_GetDriverVersion;
FRIL_GetEmergencyMode RIL_GetEmergencyMode;
FRIL_GetEquipmentState RIL_GetEquipmentState;
FRIL_GetExecutorConfig RIL_GetExecutorConfig;
FRIL_GetExecutorFocus RIL_GetExecutorFocus;
FRIL_GetExecutorRFState RIL_GetExecutorRFState;
FRIL_GetHideConnectedIdSettings RIL_GetHideConnectedIdSettings;
FRIL_GetHideIdSettings RIL_GetHideIdSettings;
FRIL_GetIMSI RIL_GetIMSI;
FRIL_GetIMSStatus RIL_GetIMSStatus;
FRIL_GetMsgInUiccStatus RIL_GetMsgInUiccStatus;
FRIL_GetMsgServiceOptions RIL_GetMsgServiceOptions;
FRIL_GetNotificationFilterState RIL_GetNotificationFilterState;
FRIL_GetNumberOfModems RIL_GetNumberOfModems;
FRIL_GetOperatorList RIL_GetOperatorList;
FRIL_GetPSMediaConfiguration RIL_GetPSMediaConfiguration;
FRIL_GetPersoDeactivationState RIL_GetPersoDeactivationState;
FRIL_GetPhonebookOptions RIL_GetPhonebookOptions;
FRIL_GetPositionInfo RIL_GetPositionInfo;
FRIL_GetPreferredOperatorList RIL_GetPreferredOperatorList;
FRIL_GetRFState RIL_GetRFState;
FRIL_GetRadioConfiguration RIL_GetRadioConfiguration;
FRIL_GetRadioStateDetails RIL_GetRadioStateDetails;
FRIL_GetRadioStateGroups RIL_GetRadioStateGroups;
FRIL_GetSMSC RIL_GetSMSC;
FRIL_GetSignalQuality RIL_GetSignalQuality;
FRIL_GetSubscriberNumbers RIL_GetSubscriberNumbers;
FRIL_GetSystemSelectionPrefs RIL_GetSystemSelectionPrefs;
//FRIL_GetTerminalCapability RIL_GetTerminalCapability; missing typedef
FRIL_GetUiccATR RIL_GetUiccATR;
FRIL_GetUiccAppPersoCheckState RIL_GetUiccAppPersoCheckState;
FRIL_GetUiccLockState RIL_GetUiccLockState;
FRIL_GetUiccPRLID RIL_GetUiccPRLID;
FRIL_GetUiccRecordStatus RIL_GetUiccRecordStatus;
FRIL_GetUiccServiceLock RIL_GetUiccServiceLock;
FRIL_GetUiccServiceState RIL_GetUiccServiceState;
FRIL_GetUiccToolkitProfile RIL_GetUiccToolkitProfile;
FRIL_Initialize RIL_Initialize;
FRIL_ManageCalls RIL_ManageCalls;
FRIL_ManageCalls_V1 RIL_ManageCalls_V1;
FRIL_ManageCalls_V2 RIL_ManageCalls_V2;
FRIL_ManageCalls_V3 RIL_ManageCalls_V3;
FRIL_OpenUiccLogicalChannel RIL_OpenUiccLogicalChannel;
FRIL_RadioStateGetPasswordRetryCount RIL_RadioStateGetPasswordRetryCount;
FRIL_RadioStatePasswordCompare RIL_RadioStatePasswordCompare;
FRIL_ReadMsg RIL_ReadMsg;
FRIL_ReadPhonebookEntries RIL_ReadPhonebookEntries;
FRIL_RegisterUiccToolkitService RIL_RegisterUiccToolkitService;
FRIL_RemoveCallForwarding RIL_RemoveCallForwarding;
FRIL_ResetModem RIL_ResetModem;
FRIL_SendDTMF RIL_SendDTMF;
FRIL_SendFlash RIL_SendFlash;
FRIL_SendMsg RIL_SendMsg;
FRIL_SendMsgAck RIL_SendMsgAck;
FRIL_SendMsgAck_V1 RIL_SendMsgAck_V1;
FRIL_SendRTT RIL_SendRTT;
FRIL_SendRestrictedUiccCmd RIL_SendRestrictedUiccCmd;
FRIL_SendSupServiceData RIL_SendSupServiceData;
FRIL_SendSupServiceDataResponse RIL_SendSupServiceDataResponse;
FRIL_SendUiccToolkitCmdResponse RIL_SendUiccToolkitCmdResponse;
FRIL_SendUiccToolkitEnvelope RIL_SendUiccToolkitEnvelope;
FRIL_SetCallBarringStatus RIL_SetCallBarringStatus;
FRIL_SetCallForwardingStatus RIL_SetCallForwardingStatus;
FRIL_SetCallWaitingStatus RIL_SetCallWaitingStatus;
FRIL_SetCellBroadcastMsgConfig RIL_SetCellBroadcastMsgConfig;
FRIL_SetDMProfileConfigInfo RIL_SetDMProfileConfigInfo;
FRIL_SetEquipmentState RIL_SetEquipmentState;
FRIL_SetExecutorConfig RIL_SetExecutorConfig;
FRIL_SetExecutorFocus RIL_SetExecutorFocus;
FRIL_SetExecutorRFState RIL_SetExecutorRFState;
FRIL_SetGeolocationData RIL_SetGeolocationData;
FRIL_SetMsgInUiccStatus RIL_SetMsgInUiccStatus;
FRIL_SetMsgMemoryStatus RIL_SetMsgMemoryStatus;
FRIL_SetNotificationFilterState RIL_SetNotificationFilterState;
FRIL_SetPSMediaConfiguration RIL_SetPSMediaConfiguration;
FRIL_SetPreferredOperatorList RIL_SetPreferredOperatorList;
FRIL_SetRFState RIL_SetRFState;
FRIL_SetRFState_V1 RIL_SetRFState_V1;
FRIL_SetRadioConfiguration RIL_SetRadioConfiguration;
FRIL_SetRadioStateDetails RIL_SetRadioStateDetails;
FRIL_SetSMSC RIL_SetSMSC;
FRIL_SetSlotPower RIL_SetSlotPower;
FRIL_SetSystemSelectionPrefs RIL_SetSystemSelectionPrefs;
FRIL_SetSystemSelectionPrefs_V1 RIL_SetSystemSelectionPrefs_V1;
//FRIL_SetTerminalCapability RIL_SetTerminalCapability; // missing typedef
FRIL_SetUiccLockEnabled RIL_SetUiccLockEnabled;
FRIL_SetUiccServiceState RIL_SetUiccServiceState;
FRIL_SetUiccToolkitProfile RIL_SetUiccToolkitProfile;
FRIL_StartDTMF RIL_StartDTMF;
FRIL_StartModemLogs RIL_StartModemLogs;
FRIL_StopDTMF RIL_StopDTMF;
FRIL_StopModemLogs RIL_StopModemLogs;
FRIL_UnblockUiccLock RIL_UnblockUiccLock;
FRIL_VerifyUiccLock RIL_VerifyUiccLock;
FRIL_WatchUiccFileChange RIL_WatchUiccFileChange;
FRIL_WriteAdditionalNumberString RIL_WriteAdditionalNumberString;
FRIL_WriteMsg RIL_WriteMsg;
FRIL_WritePhonebookEntry RIL_WritePhonebookEntry;


void InitializeRILFunctions()
{
	HMODULE hDll = LoadLibrary(L"rilproxy.dll");

	RIL_AddCallForwarding = (FRIL_AddCallForwarding)GetProcAddress(hDll, "RIL_AddCallForwarding");
	RIL_AvoidCDMASystem = (FRIL_AvoidCDMASystem)GetProcAddress(hDll, "RIL_AvoidCDMASystem");
	RIL_CancelGetOperatorList = (FRIL_CancelGetOperatorList)GetProcAddress(hDll, "RIL_CancelGetOperatorList");
	RIL_CancelSupServiceDataSession = (FRIL_CancelSupServiceDataSession)GetProcAddress(hDll, "RIL_CancelSupServiceDataSession");
	RIL_ChangeCallBarringPassword = (FRIL_ChangeCallBarringPassword)GetProcAddress(hDll, "RIL_ChangeCallBarringPassword");
	RIL_ChangeUiccLockPassword = (FRIL_ChangeUiccLockPassword)GetProcAddress(hDll, "RIL_ChangeUiccLockPassword");
	RIL_CloseUiccLogicalChannel = (FRIL_CloseUiccLogicalChannel)GetProcAddress(hDll, "RIL_CloseUiccLogicalChannel");
	RIL_CloseUiccLogicalChannelGroup = (FRIL_CloseUiccLogicalChannelGroup)GetProcAddress(hDll, "RIL_CloseUiccLogicalChannelGroup");
	RIL_DeactivatePerso = (FRIL_DeactivatePerso)GetProcAddress(hDll, "RIL_DeactivatePerso");
	RIL_Deinitialize = (FRIL_Deinitialize)GetProcAddress(hDll, "RIL_Deinitialize");
	RIL_DeleteAdditionalNumberString = (FRIL_DeleteAdditionalNumberString)GetProcAddress(hDll, "RIL_DeleteAdditionalNumberString");
	RIL_DeleteMsg = (FRIL_DeleteMsg)GetProcAddress(hDll, "RIL_DeleteMsg");
	RIL_DeletePhonebookEntry = (FRIL_DeletePhonebookEntry)GetProcAddress(hDll, "RIL_DeletePhonebookEntry");
	RIL_DevSpecific = (FRIL_DevSpecific)GetProcAddress(hDll, "RIL_DevSpecific");
	RIL_Dial = (FRIL_Dial)GetProcAddress(hDll, "RIL_Dial");
	RIL_Dial_V1 = (FRIL_Dial_V1)GetProcAddress(hDll, "RIL_Dial_V1");
	RIL_DisableModemFilters = (FRIL_DisableModemFilters)GetProcAddress(hDll, "RIL_DisableModemFilters");
	RIL_DisableNotifications = (FRIL_DisableNotifications)GetProcAddress(hDll, "RIL_DisableNotifications");
	RIL_DrainModemLogs = (FRIL_DrainModemLogs)GetProcAddress(hDll, "RIL_DrainModemLogs");
	RIL_EmergencyModeControl = (FRIL_EmergencyModeControl)GetProcAddress(hDll, "RIL_EmergencyModeControl");
	RIL_EnableModemFilters = (FRIL_EnableModemFilters)GetProcAddress(hDll, "RIL_EnableModemFilters");
	RIL_EnableNotifications = (FRIL_EnableNotifications)GetProcAddress(hDll, "RIL_EnableNotifications");
	RIL_EnumerateSlots = (FRIL_EnumerateSlots)GetProcAddress(hDll, "RIL_EnumerateSlots");
	RIL_ExchangeUiccAPDU = (FRIL_ExchangeUiccAPDU)GetProcAddress(hDll, "RIL_ExchangeUiccAPDU");
	RIL_GetAllAdditionalNumberStrings = (FRIL_GetAllAdditionalNumberStrings)GetProcAddress(hDll, "RIL_GetAllAdditionalNumberStrings");
	RIL_GetAllEmergencyNumbers = (FRIL_GetAllEmergencyNumbers)GetProcAddress(hDll, "RIL_GetAllEmergencyNumbers");
	RIL_GetCallBarringStatus = (FRIL_GetCallBarringStatus)GetProcAddress(hDll, "RIL_GetCallBarringStatus");
	RIL_GetCallForwardingSettings = (FRIL_GetCallForwardingSettings)GetProcAddress(hDll, "RIL_GetCallForwardingSettings");
	RIL_GetCallWaitingSettings = (FRIL_GetCallWaitingSettings)GetProcAddress(hDll, "RIL_GetCallWaitingSettings");
	RIL_GetCallerIdSettings = (FRIL_GetCallerIdSettings)GetProcAddress(hDll, "RIL_GetCallerIdSettings");
	RIL_GetCardInfo = (FRIL_GetCardInfo)GetProcAddress(hDll, "RIL_GetCardInfo");
	RIL_GetCellBroadcastMsgConfig = (FRIL_GetCellBroadcastMsgConfig)GetProcAddress(hDll, "RIL_GetCellBroadcastMsgConfig");
	RIL_GetCurrentRegStatus = (FRIL_GetCurrentRegStatus)GetProcAddress(hDll, "RIL_GetCurrentRegStatus");
	RIL_GetDMProfileConfigInfo = (FRIL_GetDMProfileConfigInfo)GetProcAddress(hDll, "RIL_GetDMProfileConfigInfo");
	RIL_GetDevCaps = (FRIL_GetDevCaps)GetProcAddress(hDll, "RIL_GetDevCaps");
	RIL_GetDeviceInfo = (FRIL_GetDeviceInfo)GetProcAddress(hDll, "RIL_GetDeviceInfo");
	RIL_GetDialedIdSettings = (FRIL_GetDialedIdSettings)GetProcAddress(hDll, "RIL_GetDialedIdSettings");
	RIL_GetDriverVersion = (FRIL_GetDriverVersion)GetProcAddress(hDll, "RIL_GetDriverVersion");
	RIL_GetEmergencyMode = (FRIL_GetEmergencyMode)GetProcAddress(hDll, "RIL_GetEmergencyMode");
	RIL_GetEquipmentState = (FRIL_GetEquipmentState)GetProcAddress(hDll, "RIL_GetEquipmentState");
	RIL_GetExecutorConfig = (FRIL_GetExecutorConfig)GetProcAddress(hDll, "RIL_GetExecutorConfig");
	RIL_GetExecutorFocus = (FRIL_GetExecutorFocus)GetProcAddress(hDll, "RIL_GetExecutorFocus");
	RIL_GetExecutorRFState = (FRIL_GetExecutorRFState)GetProcAddress(hDll, "RIL_GetExecutorRFState");
	RIL_GetHideConnectedIdSettings = (FRIL_GetHideConnectedIdSettings)GetProcAddress(hDll, "RIL_GetHideConnectedIdSettings");
	RIL_GetHideIdSettings = (FRIL_GetHideIdSettings)GetProcAddress(hDll, "RIL_GetHideIdSettings");
	RIL_GetIMSI = (FRIL_GetIMSI)GetProcAddress(hDll, "RIL_GetIMSI");
	RIL_GetIMSStatus = (FRIL_GetIMSStatus)GetProcAddress(hDll, "RIL_GetIMSStatus");
	RIL_GetMsgInUiccStatus = (FRIL_GetMsgInUiccStatus)GetProcAddress(hDll, "RIL_GetMsgInUiccStatus");
	RIL_GetMsgServiceOptions = (FRIL_GetMsgServiceOptions)GetProcAddress(hDll, "RIL_GetMsgServiceOptions");
	RIL_GetNotificationFilterState = (FRIL_GetNotificationFilterState)GetProcAddress(hDll, "RIL_GetNotificationFilterState");
	RIL_GetNumberOfModems = (FRIL_GetNumberOfModems)GetProcAddress(hDll, "RIL_GetNumberOfModems");
	RIL_GetOperatorList = (FRIL_GetOperatorList)GetProcAddress(hDll, "RIL_GetOperatorList");
	RIL_GetPSMediaConfiguration = (FRIL_GetPSMediaConfiguration)GetProcAddress(hDll, "RIL_GetPSMediaConfiguration");
	RIL_GetPersoDeactivationState = (FRIL_GetPersoDeactivationState)GetProcAddress(hDll, "RIL_GetPersoDeactivationState");
	RIL_GetPhonebookOptions = (FRIL_GetPhonebookOptions)GetProcAddress(hDll, "RIL_GetPhonebookOptions");
	RIL_GetPositionInfo = (FRIL_GetPositionInfo)GetProcAddress(hDll, "RIL_GetPositionInfo");
	RIL_GetPreferredOperatorList = (FRIL_GetPreferredOperatorList)GetProcAddress(hDll, "RIL_GetPreferredOperatorList");
	RIL_GetRFState = (FRIL_GetRFState)GetProcAddress(hDll, "RIL_GetRFState");
	RIL_GetRadioConfiguration = (FRIL_GetRadioConfiguration)GetProcAddress(hDll, "RIL_GetRadioConfiguration");
	RIL_GetRadioStateDetails = (FRIL_GetRadioStateDetails)GetProcAddress(hDll, "RIL_GetRadioStateDetails");
	RIL_GetRadioStateGroups = (FRIL_GetRadioStateGroups)GetProcAddress(hDll, "RIL_GetRadioStateGroups");
	RIL_GetSMSC = (FRIL_GetSMSC)GetProcAddress(hDll, "RIL_GetSMSC");
	RIL_GetSignalQuality = (FRIL_GetSignalQuality)GetProcAddress(hDll, "RIL_GetSignalQuality");
	RIL_GetSubscriberNumbers = (FRIL_GetSubscriberNumbers)GetProcAddress(hDll, "RIL_GetSubscriberNumbers");
	RIL_GetSystemSelectionPrefs = (FRIL_GetSystemSelectionPrefs)GetProcAddress(hDll, "RIL_GetSystemSelectionPrefs");
	//RIL_GetTerminalCapability = (FRIL_GetTerminalCapability)GetProcAddress(hDll, "RIL_GetTerminalCapability");
	RIL_GetUiccATR = (FRIL_GetUiccATR)GetProcAddress(hDll, "RIL_GetUiccATR");
	RIL_GetUiccAppPersoCheckState = (FRIL_GetUiccAppPersoCheckState)GetProcAddress(hDll, "RIL_GetUiccAppPersoCheckState");
	RIL_GetUiccLockState = (FRIL_GetUiccLockState)GetProcAddress(hDll, "RIL_GetUiccLockState");
	RIL_GetUiccPRLID = (FRIL_GetUiccPRLID)GetProcAddress(hDll, "RIL_GetUiccPRLID");
	RIL_GetUiccRecordStatus = (FRIL_GetUiccRecordStatus)GetProcAddress(hDll, "RIL_GetUiccRecordStatus");
	RIL_GetUiccServiceLock = (FRIL_GetUiccServiceLock)GetProcAddress(hDll, "RIL_GetUiccServiceLock");
	RIL_GetUiccServiceState = (FRIL_GetUiccServiceState)GetProcAddress(hDll, "RIL_GetUiccServiceState");
	RIL_GetUiccToolkitProfile = (FRIL_GetUiccToolkitProfile)GetProcAddress(hDll, "RIL_GetUiccToolkitProfile");
	RIL_Initialize = (FRIL_Initialize)GetProcAddress(hDll, "RIL_Initialize");
	RIL_ManageCalls = (FRIL_ManageCalls)GetProcAddress(hDll, "RIL_ManageCalls");
	RIL_ManageCalls_V1 = (FRIL_ManageCalls_V1)GetProcAddress(hDll, "RIL_ManageCalls_V1");
	RIL_ManageCalls_V2 = (FRIL_ManageCalls_V2)GetProcAddress(hDll, "RIL_ManageCalls_V2");
	RIL_ManageCalls_V3 = (FRIL_ManageCalls_V3)GetProcAddress(hDll, "RIL_ManageCalls_V3");
	RIL_OpenUiccLogicalChannel = (FRIL_OpenUiccLogicalChannel)GetProcAddress(hDll, "RIL_OpenUiccLogicalChannel");
	RIL_RadioStateGetPasswordRetryCount = (FRIL_RadioStateGetPasswordRetryCount)GetProcAddress(hDll, "RIL_RadioStateGetPasswordRetryCount");
	RIL_RadioStatePasswordCompare = (FRIL_RadioStatePasswordCompare)GetProcAddress(hDll, "RIL_RadioStatePasswordCompare");
	RIL_ReadMsg = (FRIL_ReadMsg)GetProcAddress(hDll, "RIL_ReadMsg");
	RIL_ReadPhonebookEntries = (FRIL_ReadPhonebookEntries)GetProcAddress(hDll, "RIL_ReadPhonebookEntries");
	RIL_RegisterUiccToolkitService = (FRIL_RegisterUiccToolkitService)GetProcAddress(hDll, "RIL_RegisterUiccToolkitService");
	RIL_RemoveCallForwarding = (FRIL_RemoveCallForwarding)GetProcAddress(hDll, "RIL_RemoveCallForwarding");
	RIL_ResetModem = (FRIL_ResetModem)GetProcAddress(hDll, "RIL_ResetModem");
	RIL_SendDTMF = (FRIL_SendDTMF)GetProcAddress(hDll, "RIL_SendDTMF");
	RIL_SendFlash = (FRIL_SendFlash)GetProcAddress(hDll, "RIL_SendFlash");
	RIL_SendMsg = (FRIL_SendMsg)GetProcAddress(hDll, "RIL_SendMsg");
	RIL_SendMsgAck = (FRIL_SendMsgAck)GetProcAddress(hDll, "RIL_SendMsgAck");
	RIL_SendMsgAck_V1 = (FRIL_SendMsgAck_V1)GetProcAddress(hDll, "RIL_SendMsgAck_V1");
	RIL_SendRTT = (FRIL_SendRTT)GetProcAddress(hDll, "RIL_SendRTT");
	RIL_SendRestrictedUiccCmd = (FRIL_SendRestrictedUiccCmd)GetProcAddress(hDll, "RIL_SendRestrictedUiccCmd");
	RIL_SendSupServiceData = (FRIL_SendSupServiceData)GetProcAddress(hDll, "RIL_SendSupServiceData");
	RIL_SendSupServiceDataResponse = (FRIL_SendSupServiceDataResponse)GetProcAddress(hDll, "RIL_SendSupServiceDataResponse");
	RIL_SendUiccToolkitCmdResponse = (FRIL_SendUiccToolkitCmdResponse)GetProcAddress(hDll, "RIL_SendUiccToolkitCmdResponse");
	RIL_SendUiccToolkitEnvelope = (FRIL_SendUiccToolkitEnvelope)GetProcAddress(hDll, "RIL_SendUiccToolkitEnvelope");
	RIL_SetCallBarringStatus = (FRIL_SetCallBarringStatus)GetProcAddress(hDll, "RIL_SetCallBarringStatus");
	RIL_SetCallForwardingStatus = (FRIL_SetCallForwardingStatus)GetProcAddress(hDll, "RIL_SetCallForwardingStatus");
	RIL_SetCallWaitingStatus = (FRIL_SetCallWaitingStatus)GetProcAddress(hDll, "RIL_SetCallWaitingStatus");
	RIL_SetCellBroadcastMsgConfig = (FRIL_SetCellBroadcastMsgConfig)GetProcAddress(hDll, "RIL_SetCellBroadcastMsgConfig");
	RIL_SetDMProfileConfigInfo = (FRIL_SetDMProfileConfigInfo)GetProcAddress(hDll, "RIL_SetDMProfileConfigInfo");
	RIL_SetEquipmentState = (FRIL_SetEquipmentState)GetProcAddress(hDll, "RIL_SetEquipmentState");
	RIL_SetExecutorConfig = (FRIL_SetExecutorConfig)GetProcAddress(hDll, "RIL_SetExecutorConfig");
	RIL_SetExecutorFocus = (FRIL_SetExecutorFocus)GetProcAddress(hDll, "RIL_SetExecutorFocus");
	RIL_SetExecutorRFState = (FRIL_SetExecutorRFState)GetProcAddress(hDll, "RIL_SetExecutorRFState");
	RIL_SetGeolocationData = (FRIL_SetGeolocationData)GetProcAddress(hDll, "RIL_SetGeolocationData");
	RIL_SetMsgInUiccStatus = (FRIL_SetMsgInUiccStatus)GetProcAddress(hDll, "RIL_SetMsgInUiccStatus");
	RIL_SetMsgMemoryStatus = (FRIL_SetMsgMemoryStatus)GetProcAddress(hDll, "RIL_SetMsgMemoryStatus");
	RIL_SetNotificationFilterState = (FRIL_SetNotificationFilterState)GetProcAddress(hDll, "RIL_SetNotificationFilterState");
	RIL_SetPSMediaConfiguration = (FRIL_SetPSMediaConfiguration)GetProcAddress(hDll, "RIL_SetPSMediaConfiguration");
	RIL_SetPreferredOperatorList = (FRIL_SetPreferredOperatorList)GetProcAddress(hDll, "RIL_SetPreferredOperatorList");
	RIL_SetRFState = (FRIL_SetRFState)GetProcAddress(hDll, "RIL_SetRFState");
	RIL_SetRFState_V1 = (FRIL_SetRFState_V1)GetProcAddress(hDll, "RIL_SetRFState_V1");
	RIL_SetRadioConfiguration = (FRIL_SetRadioConfiguration)GetProcAddress(hDll, "RIL_SetRadioConfiguration");
	RIL_SetRadioStateDetails = (FRIL_SetRadioStateDetails)GetProcAddress(hDll, "RIL_SetRadioStateDetails");
	RIL_SetSMSC = (FRIL_SetSMSC)GetProcAddress(hDll, "RIL_SetSMSC");
	RIL_SetSlotPower = (FRIL_SetSlotPower)GetProcAddress(hDll, "RIL_SetSlotPower");
	RIL_SetSystemSelectionPrefs = (FRIL_SetSystemSelectionPrefs)GetProcAddress(hDll, "RIL_SetSystemSelectionPrefs");
	RIL_SetSystemSelectionPrefs_V1 = (FRIL_SetSystemSelectionPrefs_V1)GetProcAddress(hDll, "RIL_SetSystemSelectionPrefs_V1");
	//RIL_SetTerminalCapability = (FRIL_SetTerminalCapability)GetProcAddress(hDll, "RIL_SetTerminalCapability");
	RIL_SetUiccLockEnabled = (FRIL_SetUiccLockEnabled)GetProcAddress(hDll, "RIL_SetUiccLockEnabled");
	RIL_SetUiccServiceState = (FRIL_SetUiccServiceState)GetProcAddress(hDll, "RIL_SetUiccServiceState");
	RIL_SetUiccToolkitProfile = (FRIL_SetUiccToolkitProfile)GetProcAddress(hDll, "RIL_SetUiccToolkitProfile");
	RIL_StartDTMF = (FRIL_StartDTMF)GetProcAddress(hDll, "RIL_StartDTMF");
	RIL_StartModemLogs = (FRIL_StartModemLogs)GetProcAddress(hDll, "RIL_StartModemLogs");
	RIL_StopDTMF = (FRIL_StopDTMF)GetProcAddress(hDll, "RIL_StopDTMF");
	RIL_StopModemLogs = (FRIL_StopModemLogs)GetProcAddress(hDll, "RIL_StopModemLogs");
	RIL_UnblockUiccLock = (FRIL_UnblockUiccLock)GetProcAddress(hDll, "RIL_UnblockUiccLock");
	RIL_VerifyUiccLock = (FRIL_VerifyUiccLock)GetProcAddress(hDll, "RIL_VerifyUiccLock");
	RIL_WatchUiccFileChange = (FRIL_WatchUiccFileChange)GetProcAddress(hDll, "RIL_WatchUiccFileChange");
	RIL_WriteAdditionalNumberString = (FRIL_WriteAdditionalNumberString)GetProcAddress(hDll, "RIL_WriteAdditionalNumberString");
	RIL_WriteMsg = (FRIL_WriteMsg)GetProcAddress(hDll, "RIL_WriteMsg");
	RIL_WritePhonebookEntry = (FRIL_WritePhonebookEntry)GetProcAddress(hDll, "RIL_WritePhonebookEntry");
}

#pragma endregion

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

/*typedef struct _WNF_STATE_NAME
{
	ULONG Data[2];
} WNF_STATE_NAME, * PWNF_STATE_NAME;

typedef struct _WNF_TYPE_ID
{
	GUID TypeId;
} WNF_TYPE_ID, * PWNF_TYPE_ID;
typedef const WNF_TYPE_ID* PCWNF_TYPE_ID;

typedef ULONG WNF_CHANGE_STAMP, * PWNF_CHANGE_STAMP;*/

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

//
// These functions do not work when called from a service
//

/*extern "C"
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
	_In_ ULONG CheckStamp);*/

bool IsBuild18912OrGreater()
{
	typedef NTSTATUS(WINAPI* FRtlGetVersion)(LPOSVERSIONINFOEXW);

	HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
	FRtlGetVersion RtlGetVersion = (FRtlGetVersion)GetProcAddress(hNtdll, "RtlGetVersion");

	OSVERSIONINFOEXW v = { 0 };
	v.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	RtlGetVersion(&v);

	return v.dwBuildNumber >= 18912;
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

	//
	// Initialize the event that we use to get notified of the results of every call
	//
	ResultEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"ResultEvent");

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

				if (i == 0)
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
					HRESULT nError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Notifications\\Data", NULL, KEY_ALL_ACCESS, &hKey);
					if (nError == ERROR_SUCCESS)
					{
						nError = RegSetValueEx(hKey, L"0D8A0B2EA3BDF475", NULL, REG_BINARY, (LPBYTE)&callregdata, 0x78);
						if (nError == ERROR_SUCCESS)
						{
							nError = RegCloseKey(hKey);

							if (nError == ERROR_SUCCESS)
							{
								std::cout << "Successfully wrote WNF registration information for phone service." << std::endl;
							}
						}
					}

					//
					// These functions do not work when called from a service
					//

					/*WNF_STATE_NAME wnf_machine_store{ 0xA3BDF475, 0x0D8A0B2E };
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
					}*/
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

	RILSIGNALQUALITY* signalquality = nullptr;
	DWORD length = 0;
	result = GetSignalQuality(executor, &signalquality, &length);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	RILIMSSTATUS imsstatus = { 0 };
	result = GetIMSStatus(executor, &imsstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	//RIL_COMMAND_GETPSMEDIACONFIGURATION 0 (not implemented in wmril, normal)

	RILEXECUTORRFSTATE executorrfstate;
	result = GetExecutorRFState(executor, &executorrfstate);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

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

	RILUICCATRINFO uiccatrinfo = { 0 };
	result = GetUiccATR(executor, &uiccatrinfo);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

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

	imsstatus = { 0 };
	result = GetIMSStatus(executor, &imsstatus);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

	BYTE buffer2[] = {
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

	RILSETCELLBROADCASTMSGCONFIGPARAMS params2 = *(RILSETCELLBROADCASTMSGCONFIGPARAMS*)buffer2;

	result = SetCellBroadcastMsgConfig(params2.hUiccApp, &params2.rmCBConfig);
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

	//
	// Turn on power for the SIM slot
	//
	result = SetSlotPower(executor, TRUE);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

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

	//
	// Turn on power for the SIM slot
	//
	result = SetSlotPower(executor, TRUE);
	if (result)
	{
		std::cout << "Ok." << std::endl;
	}
	std::cout << std::endl;

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

	//
	// Restart service
	// WwanSvc won't pick up new changes from the MBB driver
	// if we don't force it to reload, so do that
	// (otherwise the data connection won't be established)
	//

	//
	// Give us a bit of time to fully bring up the network before starting
	// to restart WwanSvc
	//
	Sleep(7500);

	//
	// The actual service restart portion
	//
	SERVICE_STATUS Status;

	SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE SHandle = OpenService(SCManager, L"WwanSvc", SC_MANAGER_ALL_ACCESS);

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

	CloseServiceHandle(SCManager);
	CloseServiceHandle(SHandle);

	//
	// We're done
	//
}

int main()
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
	// Initialize imports
	//
	InitializeRILFunctions();

	//
	// Loop until RILAdaptation is loaded and ready
	// In case we get told to stop by the service control, stop.
	//
	while (true)
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