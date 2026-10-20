#include "workbench_service.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace workbench {
namespace {

std::wstring currentTimestamp() {
    const auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    std::wostringstream result;
    result << std::put_time(&localTime, L"%Y-%m-%d %H:%M:%S");
    return result.str();
}

} // namespace

std::wstring roleName(Role role) {
    switch (role) {
    case Role::Operations:
        return L"Operations";
    case Role::FinanceApprover:
        return L"Finance Approver";
    case Role::Viewer:
    default:
        return L"Viewer";
    }
}

DemoWorkbenchService::DemoWorkbenchService() : nextOrderNumber_(1043) {
    orders_ = {
        {L"ORD-1042", L"Contoso Industrial", L"Pending Approval", L"$12,480.00",
         L"Available", L"Industrial pump × 2; service kit × 4", L""},
        {L"ORD-1041", L"Northwind Traders", L"Processing", L"$3,275.50",
         L"Available", L"Network switch × 5; installation × 1", L"packing-list.pdf"},
        {L"ORD-1040", L"Fabrikam Services", L"Draft", L"$890.00",
         L"Review required", L"Replacement motor × 1", L""}
    };
    invoices_ = {
        {L"INV-2048", L"Contoso Industrial", L"Pending Approval", L"$12,480.00",
         L"Matched", L"$12,480.00 open"},
        {L"INV-2047", L"Northwind Traders", L"Paid", L"$3,275.50",
         L"Matched", L"$0.00 open"},
        {L"INV-2046", L"Fabrikam Services", L"Exception", L"$890.00",
         L"Amount mismatch", L"$890.00 open"}
    };
    auditEvents_.push_back(
        {currentTimestamp(), L"Demo", L"Opened sample workspace", L"Session"});
}

std::vector<Order> DemoWorkbenchService::orders() const {
    return orders_;
}

std::vector<Invoice> DemoWorkbenchService::invoices() const {
    return invoices_;
}

std::vector<AuditEvent> DemoWorkbenchService::auditEvents() const {
    return auditEvents_;
}

Result DemoWorkbenchService::createOrder(Role role) {
    if (role != Role::Operations) {
        return {false, L"Only the Operations demo role can create an order."};
    }

    std::wostringstream id;
    id << L"ORD-" << nextOrderNumber_++;
    orders_.insert(orders_.begin(),
                   {id.str(), L"New sample customer", L"Draft", L"$1,280.00",
                    L"Available", L"Sample component × 2", L""});
    record(role, L"Created simulated draft order", id.str());
    return {true, L"Draft order created in local sample data."};
}

Result DemoWorkbenchService::submitOrder(Role role, const std::wstring& orderId) {
    if (role != Role::Operations) {
        return {false, L"Only the Operations demo role can submit orders."};
    }

    for (auto& order : orders_) {
        if (order.id != orderId) {
            continue;
        }
        if (order.status != L"Draft") {
            return {false, L"Only draft orders can be submitted."};
        }
        if (order.availability != L"Available") {
            return {false, L"Resolve availability before submitting this order."};
        }
        order.status = L"Pending Approval";
        record(role, L"Submitted simulated order for approval", order.id);
        return {true, L"Order submitted in the local demo; no request was sent."};
    }
    return {false, L"Select an order that still exists."};
}

Result DemoWorkbenchService::approveInvoice(Role role, const std::wstring& invoiceId) {
    if (role != Role::FinanceApprover) {
        return {false, L"Only the Finance Approver demo role can approve invoices."};
    }

    for (auto& invoice : invoices_) {
        if (invoice.id != invoiceId) {
            continue;
        }
        if (invoice.status != L"Pending Approval") {
            return {false, L"Only invoices pending approval can be approved."};
        }
        if (invoice.reconciliation != L"Matched") {
            return {false, L"Resolve reconciliation exceptions before approval."};
        }
        invoice.status = L"Approved (demo)";
        record(role, L"Approved simulated invoice", invoice.id);
        return {true, L"Invoice approval simulated locally; no payment was made."};
    }
    return {false, L"Select an invoice that still exists."};
}

Result DemoWorkbenchService::attachDocument(Role role, const std::wstring& orderId,
                                            const std::wstring& fileName) {
    if (role != Role::Operations) {
        return {false, L"Only the Operations demo role can attach documents."};
    }
    if (fileName.empty()) {
        return {false, L"Choose a file before attaching a document."};
    }

    for (auto& order : orders_) {
        if (order.id == orderId) {
            order.document = fileName;
            record(role, L"Selected simulated order attachment", order.id);
            return {true, L"File name added to local sample data; the file was not uploaded."};
        }
    }
    return {false, L"Select an order that still exists."};
}

void DemoWorkbenchService::record(Role role, const std::wstring& action,
                                  const std::wstring& target) {
    auditEvents_.insert(auditEvents_.begin(),
                        {currentTimestamp(), roleName(role), action, target});
}

} // namespace workbench
