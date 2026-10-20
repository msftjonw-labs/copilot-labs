#include "workbench_service.h"

#include <cassert>

int main() {
    using namespace workbench;
    DemoWorkbenchService service;

    assert(service.orders().size() == 3);
    assert(service.invoices().size() == 3);

    const auto deniedCreate = service.createOrder(Role::Viewer);
    assert(!deniedCreate.succeeded);
    assert(service.orders().size() == 3);

    const auto created = service.createOrder(Role::Operations);
    assert(created.succeeded);
    const auto orders = service.orders();
    assert(orders.size() == 4);
    assert(orders.front().status == L"Draft");

    assert(!service.submitOrder(Role::Operations, L"ORD-1040").succeeded);
    assert(service.submitOrder(Role::Operations, orders.front().id).succeeded);
    assert(service.orders().front().status == L"Pending Approval");

    assert(!service.approveInvoice(Role::Viewer, L"INV-2048").succeeded);
    assert(service.approveInvoice(Role::FinanceApprover, L"INV-2048").succeeded);
    assert(service.invoices().front().status == L"Approved (demo)");
    assert(!service.approveInvoice(Role::FinanceApprover, L"INV-2046").succeeded);

    assert(service.attachDocument(Role::Operations, orders.front().id,
                                  L"quote.pdf").succeeded);
    assert(service.orders().front().document == L"quote.pdf");
    assert(service.auditEvents().size() >= 4);
    return 0;
}
