# Radio Interface Layer (RIL) Service Initiliazer
This (Work In Progress/WIP) utility handles the initialization phase of cellular modems requiring a RIL in order to work in Windows 10. It depends on the Microsoft EMB infrastructure (and notably MSRil.sys, infrastructure not present by default in Windows 10 Client editions) to work as well as the RilProxy.dll file to perform communications with the EMB infrastructure.

A series of tasks are performed such as reading the modem information, card information, UICC  application states and sending restricted commands to these UICC apps.

## But Why?

This utility was created following Microsoft's code refactoring in the Windows 10 20H1 (Vibranium) release, making our QUALCOMM phone modems unable to initialize. Most of this work as been done by dumping the communication between our RIL and MSRil.sys on a 19H1 system.

## Ok, but what's currently working as of now?

Basic bring up functionality is implemented, it is not "smart" as in we're discarding many information returned by the modem and sending hardcoded commands. As a result it may not work at all in most cases.
One of these cases is SIM cards with a PIN set.

The second issue is that we currently handle only one SIM slot. In the future we want to handle both slots.
The third issue is that APPerso check and states are not handled, so SIM Locked systems won't activate even with the SUBSCRIPTION_OK_V1 message being sent via QMI to AMSS.

And last but not least, we do not report the signal bar status via WNF for the system, something we may want to address for 20H1.