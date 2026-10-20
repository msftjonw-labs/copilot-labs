#include "workbench_service.h"

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shobjidl.h>

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

namespace {

using workbench::AuditEvent;
using workbench::DemoWorkbenchService;
using workbench::Invoice;
using workbench::Order;
using workbench::Role;

constexpr wchar_t kWindowClass[] = L"EnterpriseOperationsWorkbenchWindow";
constexpr wchar_t kPreferencesKey[] =
    L"Software\\CopilotLabs\\EnterpriseOperationsWorkbench";
constexpr int kNavOrders = 1001;
constexpr int kNavFinance = 1002;
constexpr int kNavAudit = 1003;
constexpr int kRole = 1004;
constexpr int kSearch = 1005;
constexpr int kCreateOrder = 1006;
constexpr int kSubmitOrder = 1007;
constexpr int kApproveInvoice = 1008;
constexpr int kAttachDocument = 1009;
constexpr int kOpenDocuments = 1010;
constexpr int kRecords = 1011;
constexpr int kDetails = 1012;
constexpr int kStatus = 1013;
constexpr int kTitle = 1014;
constexpr int kDemoLabel = 1015;
constexpr int kDetailsLabel = 1016;

enum class Page {
    Orders,
    Finance,
    Audit
};

struct App {
    HWND window = nullptr;
    HWND title = nullptr;
    HWND demoLabel = nullptr;
    HWND ordersButton = nullptr;
    HWND financeButton = nullptr;
    HWND auditButton = nullptr;
    HWND roleCombo = nullptr;
    HWND search = nullptr;
    HWND createButton = nullptr;
    HWND submitButton = nullptr;
    HWND approveButton = nullptr;
    HWND attachButton = nullptr;
    HWND documentsButton = nullptr;
    HWND records = nullptr;
    HWND detailsLabel = nullptr;
    HWND details = nullptr;
    HWND status = nullptr;
    DemoWorkbenchService service;
    Page page = Page::Orders;
    Role role = Role::Viewer;
    std::vector<std::wstring> visibleIds;
};

App* g_app = nullptr;

std::wstring controlText(HWND control) {
    const int length = GetWindowTextLengthW(control);
    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(control, text.data(), length + 1);
    text.resize(static_cast<size_t>(length));
    return text;
}

std::wstring lower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return value;
}

bool containsText(const std::wstring& value, const std::wstring& query) {
    return lower(value).find(lower(query)) != std::wstring::npos;
}

HWND createControl(HWND parent, const wchar_t* className, const wchar_t* text,
                   DWORD style, int id, DWORD exStyle = 0) {
    return CreateWindowExW(exStyle, className, text, WS_CHILD | WS_VISIBLE | style,
                           0, 0, 0, 0, parent,
                           reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                           GetModuleHandleW(nullptr), nullptr);
}

void setStatus(const std::wstring& message) {
    if (g_app && g_app->status) {
        SetWindowTextW(g_app->status, message.c_str());
    }
}

void setDetails(const std::wstring& message) {
    SetWindowTextW(g_app->details, message.c_str());
}

void addColumn(HWND list, int index, const wchar_t* title, int width) {
    LVCOLUMNW column{};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<wchar_t*>(title);
    column.cx = width;
    column.iSubItem = index;
    ListView_InsertColumn(list, index, &column);
}

void clearColumns(HWND list) {
    while (ListView_DeleteColumn(list, 0)) {
    }
}

void addRow(HWND list, int row, const std::vector<std::wstring>& values) {
    LVITEMW item{};
    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.pszText = const_cast<wchar_t*>(values.front().c_str());
    ListView_InsertItem(list, &item);
    for (size_t column = 1; column < values.size(); ++column) {
        ListView_SetItemText(list, row, static_cast<int>(column),
                             const_cast<wchar_t*>(values[column].c_str()));
    }
}

bool matchesOrder(const Order& order, const std::wstring& query) {
    return query.empty() || containsText(order.id + L" " + order.customer + L" " +
                                              order.status + L" " + order.total + L" " +
                                              order.availability + L" " + order.lineItems +
                                              L" " + order.document,
                                          query);
}

bool matchesInvoice(const Invoice& invoice, const std::wstring& query) {
    return query.empty() || containsText(invoice.id + L" " + invoice.account + L" " +
                                              invoice.status + L" " + invoice.amount +
                                              L" " + invoice.reconciliation + L" " +
                                              invoice.accountBalance,
                                          query);
}

void updateActions() {
    if (!g_app) {
        return;
    }
    const bool hasSelection = ListView_GetNextItem(g_app->records, -1, LVNI_SELECTED) >= 0;
    const bool isOrders = g_app->page == Page::Orders;
    const bool isFinance = g_app->page == Page::Finance;
    EnableWindow(g_app->createButton, isOrders && g_app->role == Role::Operations);
    EnableWindow(g_app->submitButton,
                 isOrders && hasSelection && g_app->role == Role::Operations);
    EnableWindow(g_app->attachButton,
                 isOrders && hasSelection && g_app->role == Role::Operations);
    EnableWindow(g_app->approveButton,
                 isFinance && hasSelection && g_app->role == Role::FinanceApprover);
}

void renderPage() {
    if (!g_app) {
        return;
    }
    const auto query = controlText(g_app->search);
    HWND list = g_app->records;
    ListView_DeleteAllItems(list);
    clearColumns(list);
    g_app->visibleIds.clear();
    setDetails(L"Select a row to see its details.\r\n\r\n"
               L"All records and actions in this demonstrator are local sample data.");

    if (g_app->page == Page::Orders) {
        SetWindowTextW(g_app->title, L"Order processing");
        addColumn(list, 0, L"Order", 110);
        addColumn(list, 1, L"Customer", 210);
        addColumn(list, 2, L"Status", 160);
        addColumn(list, 3, L"Total", 115);
        addColumn(list, 4, L"Availability", 145);
        addColumn(list, 5, L"Line items", 240);
        addColumn(list, 6, L"Attachment", 180);
        int row = 0;
        for (const auto& order : g_app->service.orders()) {
            if (!matchesOrder(order, query)) {
                continue;
            }
            g_app->visibleIds.push_back(order.id);
            addRow(list, row++, {order.id, order.customer, order.status, order.total,
                                 order.availability,
                                 order.lineItems,
                                 order.document.empty() ? L"—" : order.document});
        }
    } else if (g_app->page == Page::Finance) {
        SetWindowTextW(g_app->title, L"Finance");
        addColumn(list, 0, L"Invoice", 120);
        addColumn(list, 1, L"Account", 220);
        addColumn(list, 2, L"Status", 175);
        addColumn(list, 3, L"Amount", 120);
        addColumn(list, 4, L"Reconciliation", 180);
        addColumn(list, 5, L"Account open balance", 175);
        int row = 0;
        for (const auto& invoice : g_app->service.invoices()) {
            if (!matchesInvoice(invoice, query)) {
                continue;
            }
            g_app->visibleIds.push_back(invoice.id);
            addRow(list, row++, {invoice.id, invoice.account, invoice.status,
                                 invoice.amount, invoice.reconciliation,
                                 invoice.accountBalance});
        }
    } else {
        SetWindowTextW(g_app->title, L"Audit history");
        addColumn(list, 0, L"Timestamp", 170);
        addColumn(list, 1, L"Demo role", 145);
        addColumn(list, 2, L"Action", 310);
        addColumn(list, 3, L"Target", 170);
        int row = 0;
        const auto events = g_app->service.auditEvents();
        for (size_t index = 0; index < events.size(); ++index) {
            const auto& event = events[index];
            const auto searchable = event.timestamp + L" " + event.actor + L" " +
                                    event.action + L" " + event.target;
            if (!query.empty() && !containsText(searchable, query)) {
                continue;
            }
            g_app->visibleIds.push_back(std::to_wstring(index));
            addRow(list, row++, {event.timestamp, event.actor, event.action, event.target});
        }
    }
    updateActions();
}

std::wstring selectedId() {
    if (!g_app) {
        return {};
    }
    const int row = ListView_GetNextItem(g_app->records, -1, LVNI_SELECTED);
    if (row < 0 || static_cast<size_t>(row) >= g_app->visibleIds.size()) {
        return {};
    }
    return g_app->visibleIds[static_cast<size_t>(row)];
}

void showSelectionDetails() {
    if (!g_app) {
        return;
    }
    const auto id = selectedId();
    if (id.empty()) {
        updateActions();
        return;
    }

    if (g_app->page == Page::Orders) {
        for (const auto& order : g_app->service.orders()) {
            if (order.id == id) {
                setDetails(L"Order: " + order.id + L"\r\nCustomer: " + order.customer +
                           L"\r\nStatus: " + order.status + L"\r\nTotal: " +
                           order.total + L"\r\nAvailability: " + order.availability +
                           L"\r\nLine items: " + order.lineItems +
                           L"\r\nAttachment: " +
                           (order.document.empty() ? L"None" : order.document) +
                           L"\r\n\r\nApproval and fulfillment are simulated.");
                break;
            }
        }
    } else if (g_app->page == Page::Finance) {
        for (const auto& invoice : g_app->service.invoices()) {
            if (invoice.id == id) {
                setDetails(L"Invoice: " + invoice.id + L"\r\nAccount: " +
                           invoice.account + L"\r\nStatus: " + invoice.status +
                           L"\r\nAmount: " + invoice.amount + L"\r\nReconciliation: " +
                           invoice.reconciliation + L"\r\nAccount open balance: " +
                           invoice.accountBalance +
                           L"\r\n\r\nThis demo does not execute payments or post to an accounting system.");
                break;
            }
        }
    } else {
        const auto events = g_app->service.auditEvents();
        const auto eventIndex = static_cast<size_t>(_wtoi(id.c_str()));
        if (eventIndex < events.size()) {
            const auto& event = events[eventIndex];
            setDetails(L"Timestamp: " + event.timestamp + L"\r\nActor: " + event.actor +
                       L"\r\nAction: " + event.action + L"\r\nTarget: " + event.target);
        }
    }
    updateActions();
}

void layoutControls(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    const int margin = 16;
    const int top = 14;

    MoveWindow(g_app->title, margin, top, 350, 28, TRUE);
    MoveWindow(g_app->demoLabel, width - 370, top + 2, 354, 24, TRUE);
    MoveWindow(g_app->ordersButton, margin, 48, 92, 30, TRUE);
    MoveWindow(g_app->financeButton, margin + 98, 48, 92, 30, TRUE);
    MoveWindow(g_app->auditButton, margin + 196, 48, 92, 30, TRUE);
    MoveWindow(g_app->roleCombo, width - 270, 48, 254, 250, TRUE);
    MoveWindow(g_app->search, margin, 88, width - 2 * margin, 28, TRUE);
    MoveWindow(g_app->createButton, margin, 124, 132, 30, TRUE);
    MoveWindow(g_app->submitButton, margin + 140, 124, 155, 30, TRUE);
    MoveWindow(g_app->approveButton, margin + 303, 124, 150, 30, TRUE);
    MoveWindow(g_app->attachButton, margin + 461, 124, 150, 30, TRUE);
    MoveWindow(g_app->documentsButton, width - 190, 124, 174, 30, TRUE);

    const int listY = 166;
    const int listHeight = std::max(150, (height - listY - 190));
    MoveWindow(g_app->records, margin, listY, width - 2 * margin, listHeight, TRUE);
    MoveWindow(g_app->detailsLabel, margin, listY + listHeight + 10, 100, 20, TRUE);
    MoveWindow(g_app->details, margin, listY + listHeight + 32,
               width - 2 * margin, 116, TRUE);
    MoveWindow(g_app->status, margin, height - 28, width - 2 * margin, 20, TRUE);
}

void loadWindowSize(int& width, int& height) {
    DWORD storedWidth = 0;
    DWORD storedHeight = 0;
    DWORD bytes = sizeof(DWORD);
    if (RegGetValueW(HKEY_CURRENT_USER, kPreferencesKey, L"WindowWidth",
                     RRF_RT_REG_DWORD, nullptr, &storedWidth, &bytes) == ERROR_SUCCESS &&
        storedWidth >= 760 && storedWidth <= 4000) {
        width = static_cast<int>(storedWidth);
    }
    bytes = sizeof(DWORD);
    if (RegGetValueW(HKEY_CURRENT_USER, kPreferencesKey, L"WindowHeight",
                     RRF_RT_REG_DWORD, nullptr, &storedHeight, &bytes) == ERROR_SUCCESS &&
        storedHeight >= 560 && storedHeight <= 3000) {
        height = static_cast<int>(storedHeight);
    }
}

void saveWindowSize(HWND window) {
    WINDOWPLACEMENT placement{};
    placement.length = sizeof(placement);
    if (!GetWindowPlacement(window, &placement)) {
        return;
    }
    const RECT& rect = placement.rcNormalPosition;
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kPreferencesKey, 0, nullptr, 0,
                        KEY_SET_VALUE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return;
    }
    const DWORD width = static_cast<DWORD>(rect.right - rect.left);
    const DWORD height = static_cast<DWORD>(rect.bottom - rect.top);
    RegSetValueExW(key, L"WindowWidth", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&width), sizeof(width));
    RegSetValueExW(key, L"WindowHeight", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&height), sizeof(height));
    RegCloseKey(key);
}

void applyResult(const workbench::Result& result) {
    setStatus(result.message);
    if (!result.succeeded) {
        MessageBeep(MB_ICONWARNING);
    }
    renderPage();
}

void chooseAttachment() {
    if (!g_app) {
        return;
    }
    const auto id = selectedId();
    if (id.empty()) {
        setStatus(L"Select an order before choosing an attachment.");
        return;
    }

    IFileOpenDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&dialog));
    if (FAILED(hr)) {
        setStatus(L"Could not open the Windows file picker.");
        return;
    }
    DWORD options = 0;
    if (SUCCEEDED(dialog->GetOptions(&options))) {
        dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
    }
    hr = dialog->Show(g_app->window);
    if (SUCCEEDED(hr)) {
        IShellItem* item = nullptr;
        hr = dialog->GetResult(&item);
        if (SUCCEEDED(hr)) {
            PWSTR path = nullptr;
            hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
            if (SUCCEEDED(hr) && path) {
                const std::wstring fullPath(path);
                const auto separator = fullPath.find_last_of(L"\\/");
                const auto fileName = separator == std::wstring::npos
                                          ? fullPath
                                          : fullPath.substr(separator + 1);
                applyResult(g_app->service.attachDocument(g_app->role, id, fileName));
                CoTaskMemFree(path);
            }
            item->Release();
        }
    } else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        setStatus(L"Attachment selection cancelled.");
    } else {
        setStatus(L"Could not read the selected file.");
    }
    dialog->Release();
}

void openDocumentsFolder() {
    PWSTR path = nullptr;
    const HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr,
                                            &path);
    if (FAILED(hr) || !path) {
        setStatus(L"Windows could not resolve the Documents folder.");
        return;
    }
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(g_app->window, L"open", path, nullptr, nullptr, SW_SHOWNORMAL));
    CoTaskMemFree(path);
    if (result <= 32) {
        setStatus(L"Windows could not open the Documents folder.");
    } else {
        setStatus(L"Opened your Documents folder in File Explorer.");
    }
}

void createControls(HWND window) {
    g_app->title = createControl(window, L"STATIC", L"Order processing",
                                 SS_LEFT | SS_CENTERIMAGE, kTitle);
    g_app->demoLabel = createControl(
        window, L"STATIC", L"DEMO DATA  |  SIMULATED ACTIONS  |  NO BACKEND",
        SS_RIGHT | SS_CENTERIMAGE, kDemoLabel);
    g_app->ordersButton = createControl(window, L"BUTTON", L"Orders",
                                         BS_PUSHBUTTON | WS_TABSTOP, kNavOrders);
    g_app->financeButton = createControl(window, L"BUTTON", L"Finance",
                                          BS_PUSHBUTTON | WS_TABSTOP, kNavFinance);
    g_app->auditButton = createControl(window, L"BUTTON", L"Audit",
                                        BS_PUSHBUTTON | WS_TABSTOP, kNavAudit);
    g_app->roleCombo = createControl(window, L"COMBOBOX", L"",
                                      CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, kRole);
    SendMessageW(g_app->roleCombo, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(L"Demo role: Viewer"));
    SendMessageW(g_app->roleCombo, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(L"Demo role: Operations"));
    SendMessageW(g_app->roleCombo, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(L"Demo role: Finance Approver"));
    SendMessageW(g_app->roleCombo, CB_SETCURSEL, 0, 0);
    g_app->search = createControl(window, L"EDIT", L"",
                                   ES_AUTOHSCROLL | WS_TABSTOP, kSearch, WS_EX_CLIENTEDGE);
    g_app->createButton = createControl(window, L"BUTTON", L"New sample order",
                                         BS_PUSHBUTTON | WS_TABSTOP, kCreateOrder);
    g_app->submitButton = createControl(window, L"BUTTON", L"Submit for approval",
                                         BS_PUSHBUTTON | WS_TABSTOP, kSubmitOrder);
    g_app->approveButton = createControl(window, L"BUTTON", L"Approve invoice",
                                          BS_PUSHBUTTON | WS_TABSTOP, kApproveInvoice);
    g_app->attachButton = createControl(window, L"BUTTON", L"Attach document...",
                                         BS_PUSHBUTTON | WS_TABSTOP, kAttachDocument);
    g_app->documentsButton = createControl(window, L"BUTTON", L"Open Documents folder",
                                            BS_PUSHBUTTON | WS_TABSTOP, kOpenDocuments);
    g_app->records = createControl(window, WC_LISTVIEWW, L"",
                                    LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                                        WS_TABSTOP,
                                    kRecords, WS_EX_CLIENTEDGE);
    ListView_SetExtendedListViewStyle(g_app->records,
                                      LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    g_app->detailsLabel = createControl(window, L"STATIC", L"Details",
                                         SS_LEFT | SS_CENTERIMAGE, kDetailsLabel);
    g_app->details = createControl(window, L"EDIT", L"",
                                    ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY |
                                        WS_VSCROLL | WS_TABSTOP,
                                    kDetails, WS_EX_CLIENTEDGE);
    g_app->status = createControl(window, L"STATIC",
                                   L"Ready. Demo actions only; no service is connected.",
                                   SS_LEFT | SS_CENTERIMAGE, kStatus);

    HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    for (HWND control : {g_app->title, g_app->demoLabel, g_app->ordersButton,
                         g_app->financeButton, g_app->auditButton, g_app->roleCombo,
                         g_app->search, g_app->createButton, g_app->submitButton,
                         g_app->approveButton, g_app->attachButton,
                         g_app->documentsButton, g_app->records,
                         g_app->detailsLabel, g_app->details, g_app->status}) {
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    }
    SendMessageW(g_app->title, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    layoutControls(window);
    renderPage();
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam,
                                 LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        createControls(window);
        return 0;
    case WM_SIZE:
        if (g_app && g_app->records) {
            layoutControls(window);
        }
        return 0;
    case WM_COMMAND:
        if (!g_app) {
            break;
        }
        switch (LOWORD(wParam)) {
        case kNavOrders:
            g_app->page = Page::Orders;
            renderPage();
            return 0;
        case kNavFinance:
            g_app->page = Page::Finance;
            renderPage();
            return 0;
        case kNavAudit:
            g_app->page = Page::Audit;
            renderPage();
            return 0;
        case kRole:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                switch (SendMessageW(g_app->roleCombo, CB_GETCURSEL, 0, 0)) {
                case 1:
                    g_app->role = Role::Operations;
                    break;
                case 2:
                    g_app->role = Role::FinanceApprover;
                    break;
                default:
                    g_app->role = Role::Viewer;
                    break;
                }
                setStatus(L"Demo role changed. This is not authenticated authorization.");
                updateActions();
            }
            return 0;
        case kSearch:
            if (HIWORD(wParam) == EN_CHANGE) {
                renderPage();
            }
            return 0;
        case kCreateOrder:
            applyResult(g_app->service.createOrder(g_app->role));
            return 0;
        case kSubmitOrder:
            applyResult(g_app->service.submitOrder(g_app->role, selectedId()));
            return 0;
        case kApproveInvoice:
            applyResult(g_app->service.approveInvoice(g_app->role, selectedId()));
            return 0;
        case kAttachDocument:
            chooseAttachment();
            return 0;
        case kOpenDocuments:
            openDocumentsFolder();
            return 0;
        }
        break;
    case WM_NOTIFY:
        if (g_app && reinterpret_cast<NMHDR*>(lParam)->idFrom == kRecords &&
            reinterpret_cast<NMHDR*>(lParam)->code == LVN_ITEMCHANGED) {
            showSelectionDetails();
        }
        break;
    case WM_DESTROY:
        if (g_app) {
            saveWindowSize(window);
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
        MessageBoxW(nullptr, L"COM initialization failed.", L"Enterprise Operations Workbench",
                    MB_OK | MB_ICONERROR);
        return 1;
    }
    const bool uninitializeCom = SUCCEEDED(comResult);

    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&controls);

    App app;
    g_app = &app;
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&windowClass)) {
        if (uninitializeCom) {
            CoUninitialize();
        }
        return 1;
    }

    int width = 1120;
    int height = 780;
    loadWindowSize(width, height);
    HWND window = CreateWindowExW(
        0, kWindowClass, L"Enterprise Operations Workbench",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, instance, nullptr);
    if (!window) {
        if (uninitializeCom) {
            CoUninitialize();
        }
        return 1;
    }
    app.window = window;
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    g_app = nullptr;
    if (uninitializeCom) {
        CoUninitialize();
    }
    return static_cast<int>(message.wParam);
}
