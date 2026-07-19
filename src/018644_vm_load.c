/* @unit VmLoad */
#include <shinobi.h>
#include "018644_vm_load.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "sectionB.h"

/* ====================
 * Functions
 * ====================
 */

/*
 * Multi-frame task that loads a null-terminated list of save files (task->0x18)
 * from the selected VMU. State 0 requests the next missing file via BupLoad and
 * yields; state 1 waits for the drive, analyzes the loaded image, and appends its
 * header to the growing buffer. var_8c226010 reports the outcome: 1 = all loaded,
 * 2 = error.
 */
void VmLoadTask_8c018644(Task *task)
{
    char **names;

    if (task->field_0x08 == 0) {
        for (names = (char **)task->queuedItem_0x18; **names != '\0'; names++) {
            int err = buIsExistFile(var_selectedVm_8c1ba34c, *names);
            if (err == 0) {
                BupLoad_8c014bc6(var_selectedVm_8c1ba34c, *names, var_8c225fe0);
                var_8c225fe4[var_8c22600c] = (int)task->field_0x0c;
                var_8c22600c++;
                task->queuedItem_0x18 = names + 1;
                task->field_0x08 = 1;
                task->field_0x0c = (void *)((int)task->field_0x0c + 1);
                return;
            }
            if (err != -0xfb) {
                TaskFree_8c014b66(task);
                var_8c226010 = 2;
                return;
            }
            task->field_0x0c = (void *)((int)task->field_0x0c + 1);
        }
        TaskFree_8c014b66(task);
        var_8c226010 = 1;
    } else if (task->field_0x08 == 1 && buStat(var_selectedVm_8c1ba34c) == 0) {
        if (buGetLastError(var_selectedVm_8c1ba34c) != 0) {
            TaskFree_8c014b66(task);
            var_8c226010 = 2;
            return;
        }
        var_8c1ba348 = syMalloc(0xe8);
        buAnalyzeBackupFileImage(&var_8c1ba2e4, var_8c225fe0);
        njMemCopy(var_8c1ba348, var_8c1ba33c, 0xe8);
        njMemCopy(var_8c225fe0, var_8c1ba348, 0xe8);
        syFree(var_8c1ba348);
        var_8c1ba348 = (void *)-1;
        var_8c225fe0 = (char *)var_8c225fe0 + 0x600;
        task->field_0x08 = 0;
    }
}
