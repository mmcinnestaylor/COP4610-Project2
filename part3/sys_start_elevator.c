#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>

long (*STUB_start_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_start_elevator);

SYSCALL_DEFINEn(start_elevator) 
{
    if (STUB_start_elevator != NULL)
        return STUB_start_elevator();
    else
        return -ENOSYS;
}
