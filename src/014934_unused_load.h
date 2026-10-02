/* 8c014934: the unused twin of RouteSpawnLoadTask_8c0144fc -- same loading
 * screen, but it spawns RouteUnusedTask_8c014784, which ends by binding
 * the interior texture and handing off to the input task. Nothing in the image
 * calls it, and 014934 is that task's only referencer, so the pair is dead. */
#ifndef _014934_UNUSED_LOAD_H
#define _014934_UNUSED_LOAD_H

void UnusedLoadSpawnTask_8c014934();

#endif // _014934_UNUSED_LOAD_H
