#include "communication/dispatch.h"
#include "utils/message.h"
#include "memory/memory.h"
#include "memory/process.h"

// must change
UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\infestation");
UNICODE_STRING device_link = RTL_CONSTANT_STRING(L"\\DosDevices\\infestation");
PDEVICE_OBJECT device_object;


NTSTATUS driver_unload(PDRIVER_OBJECT driver_object) {

	IoDeleteDevice(device_object);
	IoDeleteSymbolicLink(&device_link);
	message("Goodbye, world!\n");
	return STATUS_SUCCESS;
}

NTSTATUS driver_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	
	UNREFERENCED_PARAMETER(registry_path);

	message("Hello, world!\n");
	driver_object->DriverUnload = driver_unload;

	message("mouhid.sys %p\n", get_kernel_module("mouhid.sys"));


	IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
	IoCreateSymbolicLink(&device_link, &device_name);

	driver_object->MajorFunction[IRP_MJ_CREATE] = on_create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = on_close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = on_message;

	device_object->Flags |= DO_DIRECT_IO;
	device_object->Flags &= ~DO_DEVICE_INITIALIZING;
	
	// Sample example, have fun!
	int pid = get_process_id("cs2.exe");
	UNICODE_STRING module_name;
	RtlInitUnicodeString(&module_name, L"client.dll");
	uintptr_t client = get_module_base(pid, module_name);
	int local_health = 0;
	uintptr_t local_player = 0;
	read_virtual_memory(pid, client + 0x16C2B18, &local_player, sizeof(uintptr_t));

	read_virtual_memory(pid, local_player + 0x32C,&local_health, sizeof(int));

	message("local_player %p , health %d",local_player, local_health);
	
	return STATUS_SUCCESS;
}

// NOTE: to use this driver, you need to change the current driver_entry to driver_initialize
//NTSTATUS driver_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
//	UNREFERENCED_PARAMETER(driver_object);
//	UNREFERENCED_PARAMETER(registry_path);
//
//	message("system range start is %p driver_entry at %p\n", MmSystemRangeStart, driver_entry);
//
//	NTSTATUS status = STATUS_SUCCESS;
//	__try {
		// must change
//		UNICODE_STRING driver_name = RTL_CONSTANT_STRING(L"\\Driver\\infestation");
//
//		status = IoCreateDriver(&driver_name, &driver_initialize);
//	}
//	__except (EXCEPTION_EXECUTE_HANDLER) {
//		status = GetExceptionCode();
//	}
//
//	if (!NT_SUCCESS(status)) {
//		message("driver_initialize failed with status 0x%08X\n", status);
//		driver_unload(driver_object);
//	}
//
//	return status;
//}
