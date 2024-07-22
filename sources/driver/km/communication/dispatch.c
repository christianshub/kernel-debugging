#include "dispatch.h"

NTSTATUS on_message(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	PIO_STACK_LOCATION io_stack = IoGetCurrentIrpStackLocation(irp);
	ULONG control_code = io_stack->Parameters.DeviceIoControl.IoControlCode;

	if (control_code == MOUSE_REQUEST) {
		message("Mouse request\n");
		PKMOUSE_REQUEST mouse_request = (PKMOUSE_REQUEST)irp->AssociatedIrp.SystemBuffer;

		mouse_move(mouse_request->x, mouse_request->y, mouse_request->button_flags);

		// Perform the double click
		mouse_down();
		mouse_up();
		mouse_down();
		mouse_up();

		irp->IoStatus.Status = STATUS_SUCCESS;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
	}
	else if (control_code == PROCESSID_REQUEST) {
		// Handle other requests (if needed)
	}
	else if (control_code == MODULEBASE_REQUEST) {
		// Handle other requests (if needed)
	}
	else if (control_code == READ_REQUEST) {
		// Handle other requests (if needed)
	}
	else {
		message("Unknown request\n");
		irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return STATUS_ABANDONED;
	}
}

NTSTATUS unsupported_opperation(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);
	UNREFERENCED_PARAMETER(irp);

	message("TODO\n");
	return STATUS_NOT_SUPPORTED;
}

NTSTATUS on_create(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	message("Creation called\n");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS on_close(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	message("Close called\n");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
