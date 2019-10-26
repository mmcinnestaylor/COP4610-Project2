#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

long (*STUB_issue_request)(int, int, int) = NULL;
EXPORT_SYMBOL(STUB_issue_request);

SYSCALL_DEFINEn(issue_request, int, p_type, int, s_floor, int, d_floor)
{
    if (STUB_issue_request != NULL)
        return STUB_issue_request(p_type, s_floor, d_floor);
    else
        return -ENOSYS;
}
