#ifndef TRASH_UTILS_H
#define TRASH_UTILS_H

#include <suifkernel/suifkernel_forwarders.h>

/**	@file
 *	trash management functions for suif objects
 *	It is not always a good idea to delete suif objects
 *	immediately when you replace them because there may still be
 *	other references which will need to access them. This
 * 	provides an alternative mechanism
 */

/**
 * trash_it() is called on an unowned suif object.
 * a reference to is will be placed into an annotation
 * called "trash" in on the FileSetBlock.
 */
void trash_it(SuifEnv *s, SuifObject *obj);
/**
 * This variant of trash_it will trash the 
 * suifobject in it's creator's SuifEnv
 */
void trash_it(SuifObject *obj);

/**
 * take_out_trash()
 *
 * will walk through the entire file set block and
 * look for references to the "trash" not in trash.
 *
 * If there are not any references to an object in the
 * trash, the object will be removed.
 */
void take_out_trash(SuifEnv *s);

/**
 * validate_file_set_block_ownership
 * 
 * Check every owned object in the fileset block to verify that
 * it's ultimate ancestor is the fileset block.
 *
 * Then check every referenced object in the fileset block
 * to verify that its ultimate ancestor is the fileset block.
 *
 */
size_t validate_file_set_block_ownership(FileSetBlock *fsb);

#endif /* TRASH_UTILS_H */
