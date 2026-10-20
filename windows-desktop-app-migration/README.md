# Enterprise Operations Workbench

A native Win32 desktop demonstrator for common order and finance workflows. The
application uses C++17 and the Windows SDK; its in-memory service, sample data,
and explicitly simulated actions are separate from the Win32 UI.

## Features

- **Orders:** Search sample orders, create a draft, validate availability,
  submit an order for simulated approval, and associate a user-selected
  document's file name with an order.
- **Finance:** Review invoice/payment statuses, sample account open balances, and reconciliation exceptions;
  simulate approval of a matched pending invoice. No payment is executed.
- **Audit:** Inspect actions recorded by the local demo service.
- **Windows integration:** Win32 controls and keyboard-tab navigation, COM
  `IFileOpenDialog` for user-selected attachments, Windows Shell integration to
  open the Documents folder, and per-user Registry storage of window dimensions.

The demo role selector is only a local presentation aid. It is not identity,
authorization, or a security boundary. No business records or credentials are
written to the Registry or disk. Attachments are not copied or uploaded; only
the selected file name is shown in volatile sample data. No enterprise service
is connected.

## Build on Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload
and CMake. From this directory, run:

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Run `build\Release\enterprise_operations_workbench.exe` (or the executable in
the selected configuration directory).

The portable sample-service tests can also be built on non-Windows systems;
the native UI target is generated only on Windows:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Production integration boundary

`IWorkbenchService` separates the UI from demo business operations. Replace
`DemoWorkbenchService` with an authenticated service-backed implementation only
after API contracts, identity, server-enforced role permissions, audit
requirements, and error/loading behavior are specified. The service must remain
authoritative for order availability, approvals, reconciliation, and all
accounting/payment actions; client-side role checks are not security controls.
