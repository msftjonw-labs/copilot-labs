#pragma once

#include <string>
#include <vector>

namespace workbench {

enum class Role {
    Viewer,
    Operations,
    FinanceApprover
};

struct Order {
    std::wstring id;
    std::wstring customer;
    std::wstring status;
    std::wstring total;
    std::wstring availability;
    std::wstring lineItems;
    std::wstring document;
};

struct Invoice {
    std::wstring id;
    std::wstring account;
    std::wstring status;
    std::wstring amount;
    std::wstring reconciliation;
    std::wstring accountBalance;
};

struct AuditEvent {
    std::wstring timestamp;
    std::wstring actor;
    std::wstring action;
    std::wstring target;
};

struct Result {
    bool succeeded;
    std::wstring message;
};

class IWorkbenchService {
public:
    virtual ~IWorkbenchService() = default;

    virtual std::vector<Order> orders() const = 0;
    virtual std::vector<Invoice> invoices() const = 0;
    virtual std::vector<AuditEvent> auditEvents() const = 0;
    virtual Result createOrder(Role role) = 0;
    virtual Result submitOrder(Role role, const std::wstring& orderId) = 0;
    virtual Result approveInvoice(Role role, const std::wstring& invoiceId) = 0;
    virtual Result attachDocument(Role role, const std::wstring& orderId,
                                  const std::wstring& fileName) = 0;
};

class DemoWorkbenchService final : public IWorkbenchService {
public:
    DemoWorkbenchService();

    std::vector<Order> orders() const override;
    std::vector<Invoice> invoices() const override;
    std::vector<AuditEvent> auditEvents() const override;
    Result createOrder(Role role) override;
    Result submitOrder(Role role, const std::wstring& orderId) override;
    Result approveInvoice(Role role, const std::wstring& invoiceId) override;
    Result attachDocument(Role role, const std::wstring& orderId,
                          const std::wstring& fileName) override;

private:
    void record(Role role, const std::wstring& action, const std::wstring& target);

    std::vector<Order> orders_;
    std::vector<Invoice> invoices_;
    std::vector<AuditEvent> auditEvents_;
    unsigned int nextOrderNumber_;
};

std::wstring roleName(Role role);

} // namespace workbench
