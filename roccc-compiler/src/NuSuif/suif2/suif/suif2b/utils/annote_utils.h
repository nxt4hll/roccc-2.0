#ifndef __UTILS__ANNOTE_UTILS_H__
#define __UTILS__ANNOTE_UTILS_H__ 1

#include <suifkernel/suifkernel_forwarders.h>
#include <common/suif_list.h>

/**	@file
 *	Utilities for Annotes.
 *
 */


/**
 * trash_named_annotes()
 * 
 * Starting at root, look for Annotes with given name(s), and put them in
 * the trash for suif_env.
 */

void trash_named_annotes(SuifEnv *suif_env,
			 list<LString> &annote_names,
			 SuifObject *root);

void trash_named_annotes(SuifEnv *suif_env,
			 LString annote_name,
			 SuifObject *root);


#endif
