#include <ntddk.h>

void DriverUnload(PDRIVER_OBJECT dob)
{
	UNREFERENCED_PARAMETER(dob);
	DbgPrint("Driver unloaded");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = DriverUnload;
	DbgPrint("Driver loaded");

	return STATUS_SUCCESS;
}