/*-
 * Copyright (c) 2008-2014 WiredTiger, Inc.
 *	All rights reserved.
 *
 * See the file LICENSE for redistribution information.
 */

#include "wt_internal.h"

#if defined(HAVE_FALLOCATE)
#include <linux/falloc.h>
#endif

/*
 * __wt_fallocate --
 *	Allocate space for a file handle.
 */
int
__wt_fallocate(WT_SESSION_IMPL *session, WT_FH *fh, wt_off_t offset, wt_off_t len)
{
	WT_DECL_RET;

	WT_RET(__wt_verbose(
	    session, WT_VERB_FILEOPS, "%s: fallocate", fh->name));

	/*
	 * Prefer the non-portable Linux fallocate call if it's available,
	 * it's the only one that doesn't require locking by our caller.
	 * See the __block_extend function for details.
	 */
#if defined(HAVE_FALLOCATE)
	WT_SYSCALL_RETRY(
	    fallocate(fh->fd, FALLOC_FL_KEEP_SIZE, offset, len), ret);
#elif defined(HAVE_POSIX_FALLOCATE)
	WT_SYSCALL_RETRY(posix_fallocate(fh->fd, offset, len), ret);
	if (ret != 0)
		WT_RET_MSG(session, ret, "%s: posix_fallocate", fh->name);
#elif defined(HAVE_FTRUNCATE)
	WT_SYSCALL_RETRY(ftruncate(fh->fd, offset + len), ret);
	if (ret != 0)
		WT_RET_MSG(session, ret, "%s: ftruncate", fh->name);
#else
	WT_UNUSED(ret);
	WT_UNUSED(offset);
	WT_UNUSED(len);
#endif
	return (0);
}
