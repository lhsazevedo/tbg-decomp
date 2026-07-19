#include <shinobi.h>

#ifndef _02F320_H_
#define _02F320_H_

/* referenced by 01614c_debug_menu's replaySaveTask_8c0167ca: size (bytes) of the recorded replay data */
extern Uint32 var_8c228ba4;

/* referenced by 01614c_debug_menu's startReplaySave_8c016924: resets serializer state before FUN_8c02f934 */
void FUN_8c02f320(void);

/* referenced by 01614c_debug_menu's startReplaySave_8c016924: packs src into **dest (advancing it), size bytes */
void FUN_8c02f934(void *src, void **dest, Uint32 size);

/* referenced by 01614c_debug_menu's replayLoadTask_8c0169bc: unpacks src into **dest (advancing it), size bytes */
void FUN_readDemo_8c02fa14(void *src, void **dest, Uint32 size);

#endif /* _02F320_H_ */
