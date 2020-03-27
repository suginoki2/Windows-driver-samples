#include "driver.h"


NTSTATUS
EchoDeviceCreate(PWDFDEVICE_INIT DeviceInit)
    WDF_OBJECT_ATTRIBUTES attributes;
    PDEVICE_CONTEXT deviceContext;
    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

   
    pnpPowerCallbacks.EvtDeviceSelfManagedIoInit    = EchoEvtDeviceSelfManagedIoStart;
    pnpPowerCallbacks.EvtDeviceSelfManagedIoSuspend = EchoEvtDeviceSelfManagedIoSuspend;

    #pragma warning(suppress: 28024)
    pnpPowerCallbacks.EvtDeviceSelfManagedIoRestart = EchoEvtDeviceSelfManagedIoStart;


    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, REQUEST_CONTEXT);
    WdfDeviceInitSetRequestAttributes(DeviceInit, &attributes);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);

    if (NT_SUCCESS(status)) {
        deviceContext = WdfObjectGet_DEVICE_CONTEXT(device);
        deviceContext->PrivateDeviceData = 0;

        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_ECHO,
            NULL // ReferenceString
            );

        if (NT_SUCCESS(status)) {
            //
            // Initialize the I/O Package and any Queues
            //
            status = EchoQueueInitialize(device);
        }
    }

    return status;
}


NTSTATUS
EchoEvtDeviceSelfManagedIoStart(
    IN  WDFDEVICE Device
    )
{
    PQUEUE_CONTEXT queueContext = QueueGetContext(WdfDeviceGetDefaultQueue(Device));
    LARGE_INTEGER DueTime;

    KdPrint(("--> EchoEvtDeviceSelfManagedIoInit\n"));

    WdfIoQueueStart(WdfDeviceGetDefaultQueue(Device));

    DueTime.QuadPart = WDF_REL_TIMEOUT_IN_MS(100);

    WdfTimerStart(queueContext->Timer,  DueTime.QuadPart);

    KdPrint(( "<-- EchoEvtDeviceSelfManagedIoInit\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
EchoEvtDeviceSelfManagedIoSuspend(
    IN  WDFDEVICE Device
    )
{
    PQUEUE_CONTEXT queueContext = QueueGetContext(WdfDeviceGetDefaultQueue(Device));

    PAGED_CODE();

    KdPrint(("--> EchoEvtDeviceSelfManagedIoSuspend\n"));

    WdfIoQueueStopSynchronously(WdfDeviceGetDefaultQueue(Device));

    WdfTimerStop(queueContext->Timer, TRUE);

    KdPrint(( "<-- EchoEvtDeviceSelfManagedIoSuspend\n"));

    return STATUS_SUCCESS;
}



