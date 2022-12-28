#ifndef LCF_POSIX
#define LCF_POSIX "1.0.0"

#define LCF_MEMORY_PROVIDE_MEMORY "posix"
#define LCF_MEMORY_reserve posix_Reserve
#define LCF_MEMORY_commit posix_Commit
#define LCF_MEMORY_decommit posix_Decommit
#define LCF_MEMORY_free posix_Free
#define LCF_MEMORY_RESERVE_SIZE GB(1)
#define LCF_MEMORY_COMMIT_SIZE (posix_GetPageSize())

#include "../base/lcf_base.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>

/* Memory */
u64 posix_GetPageSize();
LCF_MEMORY_RESERVE_MEMORY(posix_Reserve);
LCF_MEMORY_COMMIT_MEMORY(posix_Commit);
LCF_MEMORY_DECOMMIT_MEMORY(posix_Decommit);
LCF_MEMORY_FREE_MEMORY(posix_Free);

/* Files */

#endif /* LCF_POSIX */
