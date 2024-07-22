#pragma once
#include "../utils/defs.h"

#define KEYBOARD_KEY_DOWN 0x0000
#define KEYBOARD_KEY_UP   0x0001

//typedef struct _KEYBOARD_OBJECT {
//    PDEVICE_OBJECT keyboard_device;
//    PVOID service_callback;
//    ULONG use_keyboard;
//} KEYBOARD_OBJECT, * PKEYBOARD_OBJECT;
//
//typedef NTSTATUS(*KeyboardClassServiceCallbackFn)(PVOID, PVOID, PVOID, PVOID);
//
//KEYBOARD_OBJECT gKeyboardObject = { 0 };

//KEYBOARD_OBJECT gKeyboardObject = { 0 };


inline NTSTATUS InitializeKeyboard(PKEYBOARD_OBJECT keyboard_obj)
{
    UNICODE_STRING class_string;
    RtlInitUnicodeString(&class_string, L"\\Driver\\KbdClass");

    PDRIVER_OBJECT class_driver_object = NULL;
    NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Keyboard] Failed Initializing Keyboard 0x1, Code: %08X\n", status);
        return status;
    }

    UNICODE_STRING hid_string;
    RtlInitUnicodeString(&hid_string, L"\\Driver\\KbdHID");

    PDRIVER_OBJECT hid_driver_object = NULL;
    status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Keyboard] Failed Initializing Keyboard 0x2, Code: %08X\n", status);
        if (class_driver_object) { ObDereferenceObject(class_driver_object); }
        return status;
    }

    PVOID class_driver_base = NULL;
    PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
    while (hid_device_object && !keyboard_obj->service_callback)
    {
        PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
        while (class_device_object && !keyboard_obj->service_callback)
        {
            if (!class_device_object->NextDevice && !keyboard_obj->keyboard_device)
            {
                keyboard_obj->keyboard_device = class_device_object;
            }

            PULONG_PTR device_extension = (PULONG_PTR)hid_device_object->DeviceExtension;
            ULONG_PTR device_ext_size = ((ULONG_PTR)hid_device_object->DeviceObjectExtension - (ULONG_PTR)hid_device_object->DeviceExtension) / 4;
            class_driver_base = class_driver_object->DriverStart;
            for (ULONG_PTR i = 0; i < device_ext_size; i++)
            {
                if (device_extension[i] == (ULONG_PTR)class_device_object && device_extension[i + 1] > (ULONG_PTR)class_driver_object)
                {
                    keyboard_obj->service_callback = (KeyboardClassServiceCallbackFn)(device_extension[i + 1]);
                    break;
                }
            }
            class_device_object = class_device_object->NextDevice;
        }
        hid_device_object = hid_device_object->AttachedDevice;
    }

    if (!keyboard_obj->keyboard_device)
    {
        PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
        while (target_device_object)
        {
            if (!target_device_object->NextDevice)
            {
                keyboard_obj->keyboard_device = target_device_object;
                break;
            }
            target_device_object = target_device_object->NextDevice;
        }
    }

    ObDereferenceObject(class_driver_object);
    ObDereferenceObject(hid_driver_object);

    return STATUS_SUCCESS;
}

inline BOOL keyboard_open() {
    message("Opening keyboard...\n");

    if (gKeyboardObject.use_keyboard == 0) {
        NTSTATUS status = InitializeKeyboard(&gKeyboardObject);
        if (!NT_SUCCESS(status)) {
            message("Failed to initialize keyboard: %x\n", status);
            gKeyboardObject.use_keyboard = 0;
            return 0;
        }

        if (gKeyboardObject.keyboard_device && gKeyboardObject.service_callback)
            gKeyboardObject.use_keyboard = 1;
    }

    message("Keyboard device: %p, callback: %p\n", gKeyboardObject.keyboard_device, gKeyboardObject.service_callback);

    return gKeyboardObject.keyboard_device && gKeyboardObject.service_callback;
}

inline VOID keyboard_send_input(USHORT make_code, USHORT flags) {
    KIRQL irql;
    ULONG input_data;
    KEYBOARD_INPUT_DATA kid = { 0 };
    kid.MakeCode = make_code;
    kid.Flags = flags;

    if (!keyboard_open())
        return;

    kid.UnitId = 1;
    message("Sending input: make_code: %x, flags: %x\n", make_code, flags);
    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    gKeyboardObject.service_callback(gKeyboardObject.keyboard_device, &kid, (PKEYBOARD_INPUT_DATA)&kid + 1, &input_data);
    KeLowerIrql(irql);
}

inline VOID keyboard_key_down(USHORT make_code) {
    message("Key down: %x\n", make_code);
    keyboard_send_input(make_code, KEYBOARD_KEY_DOWN);
}

inline VOID keyboard_key_up(USHORT make_code) {
    message("Key up: %x\n", make_code);
    keyboard_send_input(make_code, KEYBOARD_KEY_UP);
}