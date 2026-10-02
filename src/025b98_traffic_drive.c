/* @unit TrafficDrive */
#include <shinobi.h>

#include "025b98_traffic_drive.h"
#include "014a9c_tasks.h"       /* Task, TaskKill_8c014b66 */
#include "026710_traffic.h"     /* TrafficEntry, TrafficUpdateHeading_8c026bc4 */
#include "027958_bus_draw.h"    /* BusDrawPlaceEntity_8c027c3c, BusDrawFadeLights_8c028022 */
#include "02e400_collision.h"   /* CollisionFindTaskHit_8c02e400 */
#include "02e51c_attr_query.h"             /* AttrQueryFindConvexPolygon_8c02e51c, AttrQueryRegionOccupied_8c02f08a */
#include "02df3c_traffic_lookahead.h"             /* TrafficLookaheadInit_8c02df3c, TrafficLookaheadScan_8c02dfca */
#include "02f0c8_traffic_path_scan.h"             /* TrafficPathScanBuild_8c02f0c8, TrafficPathScanJunctionOccupied_8c02f28a */
#include "0207d4_vec_xz.h" /* VecXZDot_8c0207d4, PointXZ */
#include "02081c_geom.h" /* GeomDistanceXZ_8c02081c */
#include "028258_signal.h" /* SignalGetFrame_8c028900, SignalMarkCrossingOccupied_8c028984, SignalIsPedCrossing_8c028998 */
#include "013ae8_route.h"  /* var_timeOfDay_8c18ad20 */
#include "020214_drive_cue.h" /* var_driveCueState_8c2264b8 */
#include "1ba1c8_globals.h"

/* ====================
 * Functions
 * ====================
 */

/* TaskAction for a fixed-position decoration (traffic light, sign, ...).
 * While driveState_0x2b4 == 1 the entity is being pushed along
 * dirX_0x29c/dirZ_0x2a0:
 *   - clear path (all 3 ground-probe .attr_0x00 words nonzero) and no live
 *     collision: applies one step using the *current* speed_0x27c, then
 *     ramps it down by 0.1; the push keeps running next frame unless that
 *     brings it to 0 or below.
 *   - blocked (either gate fails): undoes one step using speed_0x27c+0.1,
 *     then unconditionally ends the push this frame -- real asm behavior:
 *     the "still running" check below always reads false on this path, so
 *     a single blocked frame cancels the whole push rather than ramping it
 *     back down over several frames.
 * Ending the push (either way) clears driveState_0x2b4 and the 3
 * probes' .count_0x0c (forcing a re-probe) and pins speed_0x27c to 0.
 * TrafficUpdateHeading_8c026bc4's first argument is a real but functionally
 * unused float (see its own header comment) -- the value here is whatever
 * the position-update multiply above left in the register (dirX_0x29c or
 * dirZ_0x2a0 times the step's speed), preserved for a bit-exact test.
 *
 * Finally refreshes busDistance_0x490 (the distance to the player's bus); if
 * that distance is over 200 and the
 * entity's preset no longer matches var_activeTrafficPreset_8c227e14, frees
 * the task, else tail-calls BusDrawPlaceEntity_8c027c3c to register/draw it for
 * this frame. */
void TrafficDriveDecoration_8c02656a(Task *task, TrafficEntry *e)
{
    float dx, dz;

    if (e->driveState_0x2b4 == 1) {
        float speed = e->speed_0x27c;
        float newSpeed;
        float leftover;

        if (e->groundProbe_0x190[0].attr_0x00 != 0 &&
            e->groundProbe_0x190[1].attr_0x00 != 0 &&
            e->groundProbe_0x190[2].attr_0x00 != 0 &&
            CollisionFindTaskHit_8c02e400(task, e) == 0) {
            dx = e->dirX_0x29c * speed;
            dz = e->dirZ_0x2a0 * speed;
            leftover = dx;
            e->posX_0xf4 += dx;
            e->posZ_0xfc += dz;
            e->frontPointX_0x100 += dx;
            e->frontPointZ_0x108 += dz;
            newSpeed = speed - 0.1f;
        } else {
            speed += 0.1f;
            dx = e->dirX_0x29c * speed;
            dz = e->dirZ_0x2a0 * speed;
            leftover = dz;
            e->posX_0xf4 -= dx;
            e->posZ_0xfc -= dz;
            e->frontPointX_0x100 -= dx;
            e->frontPointZ_0x108 -= dz;
            newSpeed = 0.0f;
        }

        if (newSpeed <= 0.0f) {
            e->driveState_0x2b4 = 0;
            e->groundProbe_0x190[0].count_0x0c = 0;
            e->groundProbe_0x190[1].count_0x0c = 0;
            e->groundProbe_0x190[2].count_0x0c = 0;
            newSpeed = 0.0f;
        }
        e->speed_0x27c = newSpeed;
        TrafficUpdateHeading_8c026bc4(leftover, e);
    }

    dx = var_busState_8c1bb9d0.posX_0x0f4 - e->posX_0xf4;
    dz = var_busState_8c1bb9d0.posZ_0x0fc - e->posZ_0xfc;
    e->busDistance_0x490 = njSqrt(dx * dx + dz * dz);

    if (e->busDistance_0x490 > 200.0f && e->spawnPresetId_0x2f4 != var_activeTrafficPreset_8c227e14) {
        TaskKill_8c014b66(task);
        return;
    }

    BusDrawPlaceEntity_8c027c3c(e, 0.0f);
}

/* TaskAction for a moving CPU vehicle, dispatched on driveState_0x2b4:
 *
 *   0/2 - normal driving (below). 2 additionally eases
 *         projectDistance_0x2c4 (how far off its path point the entity
 *         sits) back down to 2.0 while that point is behind it
 *         (VecXZDot_8c0207d4 < 0.0), reverting to driveState 0 once it
 *         gets there.
 *   1   - being pushed by a collision (speed_0x27c/dirX_0x29c/dirZ_0x2a0),
 *         same shape as TrafficDriveDecoration_8c02656a's push, gated on
 *         the same 3 ground probes plus CollisionFindTaskHit_8c02e400. Ends
 *         by refreshing projectDistance_0x2c4 and reverting to driveState 0.
 *   3   - waiting for the entity's own spawn box (resolvedArgs_0x304[0], a
 *         fixed 0..8 window) to clear after a script reload before running
 *         the script for the first time.
 *   else - none of the above; falls straight into the shared tail below
 *          with speed_0x27c/the ring-buffer sum untouched (unreachable with
 *          well-formed scripts, since driveState_0x2b4 only ever holds
 *          0-3).
 *
 * Normal driving (0/2) first resolves a junction under the entity's own
 * position (entry->junctionQueryFn_0x2cc, one of AttrQueryFindConvexPolygon_8c02e51c/eab4) to get a signal
 * id (signalId_0x410) and refreshes an obstacle-braking distance (brakeDist) by
 * scanning ahead with TrafficLookaheadScan_8c02dfca over a lookahead of
 * speed_0x27c*36.0 + lookaheadMargin_0x41c (+5.0 once already braking, to stop the
 * candidate flapping in and out of range every frame). It then walks a
 * handful of independent little state machines keyed to that signal id and
 * to resolvedArgs_0x304[0]/blockIndex_0x300 (which path block the entity is
 * currently in) -- a junction yield sequence (yieldState_0x42c/0x430/0x434/
 * 0x438), and three "decoration" waits for traffic-light frame/attachment
 * changes (signalWaitState_0x448/0x458+0x498/0x468), each armed by
 * TrafficRunEntryScript_8c027012's opcodes 5/6/7 and torn down once the
 * entity leaves the block that armed it -- before picking the frame's
 * actual speed limit as the smallest of: the obstacle distance (brakeDist,
 * converted to a speed via /108000*3000), a curve/lane-offset limit
 * (min of laneOffsetRatio_0x414/0x418), and a signal-stop limit (only computed once
 * flagged by the light logic above, via TrafficRemainingPathDistance_8c026fb0).
 * Whichever bound wins gets its own "still constrained" flag
 * (obstacleLimitActive_0x424/0x428) set for next frame's lookahead widening.
 *
 * If the resulting speed is nonzero, advances position/pathDistance by it,
 * decrements lookaheadCacheLen_0x4ec, runs the driveState==2 ground-blend above, then
 * advances along the path (TrafficAdvanceOnPath_8c026ca2) and the entry's
 * script (TrafficRunEntryScript_8c027012). Running out of script instantly
 * frees the task and returns -- skipping the shared tail's ring-buffer/
 * blinker/render bookkeeping entirely, real asm behavior -- unless this is
 * the single demo-entry/opcode-9 case that instead reloads the script from
 * its own base and re-enters at driveState 3.
 *
 * Shared tail (all driveStates): shifts a 4-slot ring buffer of recent
 * speed deltas down by one slot (dropping the oldest), sums the 3 dropped
 * samples plus this frame's own speed delta into the acceleration
 * BusDrawPlaceEntity_8c027c3c leans the body with, sets a couple of
 * lamp bits, stores the new speed_0x27c, and finally either tail-calls
 * BusDrawPlaceEntity_8c027c3c to register/draw the entity this frame (within
 * 200 units of the player, or its preset still matches
 * var_activeTrafficPreset_8c227e14) or frees the task. */
void TrafficDriveVehicle_8c025b98(Task *task, TrafficEntry *e)
{
    float speed = e->speed_0x27c;
    Sint32 signalId;
    /* Recent speed deltas, oldest first; slot 3 is written by whichever
     * driveState computed a heading this frame before falling into the
     * shared tail (or left untouched on the "else" -- unreachable -- path,
     * a genuine uninitialized-read quirk of the original asm). */
    float heading = 0.0f;

    e->blinker_0x080 = 0;

    if (e->driveState_0x2b4 == 0 || e->driveState_0x2b4 == 2) {
        /* ---- normal driving: junction lookup + obstacle scan ---- */
        float brakeDist = 9999.0f;

        if (speed != 0.0f) {
            void *hit = e->junctionQueryFn_0x2cc(
                e->posX_0xf4, e->posY_0xf8, e->posZ_0xfc, &e->junctionSlot_0x404);
            if (hit == NULL) {
                signalId = -1;
                e->lightFadeTrigger_0x2dc = 0;
            } else {
                signalId = *(Sint32 *)hit;
                e->lightFadeTrigger_0x2dc = *((Sint32 *)hit + 1);
            }
        }
        /* speed == 0.0: signalId is genuinely left uninitialized here in
         * the original asm (Ghidra flags it "unaff_r8") -- whatever
         * garbage was last in that register gets stored to signalId_0x410
         * below. A real original-game quirk, not bit-reproducible in C,
         * so left as an uninitialized read. */
        e->signalId_0x410 = signalId;
        e->atGroundJunction_0x50c = 0;

        {
            /* lookahead widens by 5.0 once already braking for an
             * obstacle, so the candidate doesn't flap in and out of range
             * every frame. */
            float lookahead = speed * 36.0f + e->lookaheadMargin_0x41c;
            TrafficEntry *ahead;
            if (e->obstacleLimitActive_0x424 != 0) {
                lookahead += 5.0f;
            }
            ahead = TrafficLookaheadScan_8c02dfca(task, e, lookahead);
            if (ahead == NULL) {
                e->obstacleLimitActive_0x424 = 0;
                e->busAheadFlag_0x2d4 = 0;
            } else {
                brakeDist = GeomDistanceXZ_8c02081c(&ahead->rearPointX_0x10c, &e->pathPointX_0x0ec);
                if (ahead == (TrafficEntry *)var_playerBus_8c1bbd9c) {
                    float busDist = GeomDistanceXZ_8c02081c(&ahead->posX_0xf4, &e->pathPointX_0x0ec);
                    if (busDist < brakeDist) {
                        brakeDist = busDist;
                    }
                    brakeDist -= 5.0f;
                }
                brakeDist -= e->lookaheadMargin_0x41c;
                if (brakeDist < 0.0f) {
                    brakeDist = 0.0f;
                }
                e->busAheadFlag_0x2d4 = ahead->busAheadFlag_0x2d4;
                if (e->busAheadFlag_0x2d4 != 0) {
                    /* Junction/collision query at the candidate's own
                     * position, swapped onto the bus attribute grid; why
                     * that grid is not established. */
                    var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrBus_0x10;
                    {
                        void *junction = AttrQueryFindConvexPolygon_8c02e51c(
                            ahead->posX_0xf4,
                            ahead->posY_0xf8,
                            ahead->posZ_0xfc,
                            &e->junctionSlot2_0x500);
                        if (junction != NULL &&
                            (*(Uint32 *)((Uint8 *)junction + 0xc) & 0xf000000) == 0) {
                            e->atGroundJunction_0x50c = 1;
                            if (speed == 0.0f && e->mergeWaitState_0x468 != 2) {
                                var_driveCueState_8c2264b8.trafficChimeArmed_0x18 = 1;
                            }
                        }
                    }
                    var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrCpu_0x20;
                }
            }
        }

        /* ---- junction yield sequence ---- */
        if (e->yieldState_0x42c == 1) {
            if (signalId == (Sint32)e->yieldEnterSignalId_0x434) {
                e->yieldState_0x42c = 2;
                e->blinkCounter_0x260 = 0;
            }
        } else if (e->yieldState_0x42c == 2) {
            if (signalId == (Sint32)e->yieldExitSignalId_0x438) {
                e->yieldState_0x42c = 0;
            } else {
                Uint32 waitCount = e->blinkCounter_0x260++;
                if ((waitCount & 0x10) == 0) {
                    if (e->yieldPriority_0x430 == 0) {
                        e->blinker_0x080 |= 2;
                    } else {
                        e->blinker_0x080 |= 4;
                    }
                }
            }
        }

        /* ---- traffic-light frame/attachment waits ---- */
        {
            Sint32 stopFlag = 0;

            if (e->signalWaitState_0x448 == 2) {
                if (e->signalWaitArmedBlock_0x454 == e->blockIndex_0x300) {
                    Sint32 frame = SignalGetFrame_8c028900(e->signalWaitFrameId_0x450);
                    if (frame != 1 || TrafficPathScanJunctionOccupied_8c02f28a(e->signalWaitFrameId_0x450) != 0) {
                        stopFlag = 1;
                    }
                } else {
                    e->signalWaitState_0x448 = 0;
                }
            }

            if (e->attachmentWaitState_0x458 == 2 && e->attachmentArmedBlock_0x464 != e->blockIndex_0x300) {
                e->attachmentWaitState_0x458 = 3;
            }

            if (e->attachmentWaitState_0x458 == 2) {
                if (SignalIsPedCrossing_8c028998(e->attachmentId_0x45c) != 0) {
                    stopFlag = 1;
                }
            } else if (e->attachmentWaitState_0x458 == 3) {
                if (signalId == (Sint32)e->attachmentExitSignalId_0x460) {
                    e->attachmentWaitState_0x458 = 0;
                } else {
                    SignalMarkCrossingOccupied_8c028984(e->attachmentId_0x45c);
                }
            }

            if (e->pendingAttachmentRelease_0x498 != 0) {
                if (e->attachmentArmedBlock_0x464 == e->blockIndex_0x300) {
                    SignalMarkCrossingOccupied_8c028984(e->pendingAttachmentRelease_0x498);
                } else {
                    e->pendingAttachmentRelease_0x498 = 0;
                }
            }

            if (e->mergeWaitState_0x468 == 2) {
                if (e->mergeWaitArmedBlock_0x470 == e->blockIndex_0x300) {
                    if (AttrQueryRegionOccupied_8c02f08a(task, e->mergeWaitSignalId_0x46c) != 0) {
                        stopFlag = 1;
                    }
                } else {
                    e->mergeWaitState_0x468 = 0;
                }
            }

            /* ---- junction-wait sub-state (junctionWaitState_0x474, 1-4; unrelated
             * to driveState_0x2b4) ---- */
            {
                float waitAdvance = speed * 30.0f;
                Sint32 waitState = e->junctionWaitState_0x474;

                if (waitState == 1) {
                    if (signalId == (Sint32)e->junctionWaitSignalId_0x478) {
                        e->junctionWaitState_0x474 = 2;
                        e->junctionWaitTimer_0x488 = 30;
                        e->blinkCounter_0x260 = 30;
                    }
                } else if (waitState == 2) {
                    if (--e->junctionWaitTimer_0x488 < 1) {
                        e->junctionWaitState_0x474 = 3;
                        e->lookaheadMargin_0x41c = speed / 30.0f;
                    }
                    TrafficUpdateFrameFlags_8c026f7e(e);
                } else if (waitState == 3) {
                    if (e->junctionWaitArmedBlock_0x480 == e->blockIndex_0x300) {
                        float halfWindow = e->width_0x23c + 8.0f;
                        void *hitBox = TrafficPathScanBuild_8c02f0c8(task, e, e->junctionPath_0x484,
                                                     e->blockIndex_0x300,
                                                     e->pathDistanceCopy_0x2c0 - halfWindow,
                                                     halfWindow + waitAdvance);
                        if (hitBox == NULL) {
                            e->junctionWaitState_0x474 = 4;
                            TrafficSeekPathRecord_8c026fcc(e, e->junctionPath_0x484);
                            e->pathDistance_0x2bc += waitAdvance;
                            e->pathDistanceCopy_0x2c0 += waitAdvance;
                            var_groundQueryPoint_8c1bc460.x =
                                e->pathRecord_0x2b8->dirX_0x0c * e->pathDistance_0x2bc +
                                e->pathRecord_0x2b8->x_0x04;
                            var_groundQueryPoint_8c1bc460.z =
                                e->pathRecord_0x2b8->dirZ_0x10 * e->pathDistance_0x2bc +
                                e->pathRecord_0x2b8->z_0x08;
                            e->projectDistance_0x2c4 = GeomDistanceXZ_8c02081c(
                                &var_groundQueryPoint_8c1bc460, &e->pathPointX_0x0ec);
                        } else {
                            brakeDist = TrafficComputeBlockedSpeed_8c026eaa(e, (TrafficEntry *)hitBox);
                        }
                    } else {
                        /* Not at the target block yet: same path-offset
                         * refresh as the state==4 case above, but against
                         * the entity's *current* segment/distance rather
                         * than the target one -- Ghidra badly mishandles
                         * this branch (reads an uninitialized stack slot
                         * and the PR register as a float); re-derived from
                         * the .src directly. */
                        var_groundQueryPoint_8c1bc460.x =
                            e->pathRecord_0x2b8->dirX_0x0c * e->pathDistance_0x2bc +
                            e->pathRecord_0x2b8->x_0x04;
                        var_groundQueryPoint_8c1bc460.z =
                            e->pathRecord_0x2b8->dirZ_0x10 * e->pathDistance_0x2bc +
                            e->pathRecord_0x2b8->z_0x08;
                        e->projectDistance_0x2c4 = GeomDistanceXZ_8c02081c(
                            &var_groundQueryPoint_8c1bc460, &e->pathPointX_0x0ec);
                    }
                    TrafficUpdateFrameFlags_8c026f7e(e);
                } else if (waitState == 4) {
                    if (e->projectDistance_0x2c4 != 2.0f) {
                        float halfWindow = e->width_0x23c + 8.0f;
                        void *hitBox = TrafficPathScanBuild_8c02f0c8(task, e, e->junctionPath_0x484,
                                                     e->blockIndex_0x300,
                                                     e->pathDistanceCopy_0x2c0 - halfWindow,
                                                     halfWindow + waitAdvance);
                        if (hitBox != NULL) {
                            brakeDist = TrafficComputeBlockedSpeed_8c026eaa(e, (TrafficEntry *)hitBox);
                        }
                        TrafficUpdateFrameFlags_8c026f7e(e);
                    } else {
                        e->junctionWaitState_0x474 = 0;
                    }
                }
            }

            /* ---- pick the frame's speed limit ---- */
            {
                float signalLimit = 9999.0f;
                float laneLimit;
                float obstacleLimit = 9999.0f;
                float curveLimit = 9999.0f;
                Sint32 coastDown = 0;

                if (stopFlag == 0) {
                    e->curveLimitActive_0x428 = 0;
                } else {
                    signalLimit = TrafficRemainingPathDistance_8c026fb0(e);
                    if (e->curveLimitActive_0x428 == 0) {
                        float slack = speed * 18.0f - signalLimit + 2.409f;
                        if (slack > 0.5f || slack < 0.0f) {
                            signalLimit = 9999.0f;
                        }
                    }
                }

                laneLimit = e->followSpeedCap_0x418 <= e->laneOffsetRatio_0x414 ? e->followSpeedCap_0x418 : e->laneOffsetRatio_0x414;

                if (brakeDist != 9999.0f) {
                    obstacleLimit = brakeDist * 3000.0f / 108000.0f;
                    if (obstacleLimit <= 0.003f) {
                        obstacleLimit = 0.0f;
                    }
                }

                if (signalLimit != 9999.0f) {
                    if (speed <= 0.09259259f) {
                        curveLimit = speed - 0.003f;
                        if (curveLimit < 0.0f) {
                            curveLimit = 0.0f;
                        }
                    } else {
                        curveLimit = (speed / 18.0f) * 17.0f;
                    }
                }

                if (obstacleLimit == 9999.0f && curveLimit == 9999.0f) {
                    coastDown = 1;
                } else if (curveLimit <= obstacleLimit) {
                    if (curveLimit < laneLimit) {
                        laneLimit = curveLimit;
                        e->curveLimitActive_0x428 = 1;
                    } else {
                        coastDown = 1;
                    }
                } else if (laneLimit <= obstacleLimit) {
                    /* fall through to the shared clamp below */
                    coastDown = 1;
                } else {
                    laneLimit = obstacleLimit;
                    e->obstacleLimitActive_0x424 = 1;
                }
                if (coastDown != 0 && laneLimit < speed) {
                    laneLimit = speed - 0.003f;
                    if (laneLimit < 0.0f) {
                        laneLimit = 0.0f;
                    }
                }

                /* ---- integrate speed towards laneLimit ---- */
                if (e->obstacleLimitActive_0x424 != 0 || e->curveLimitActive_0x428 != 0 || laneLimit < speed) {
                    if (speed - laneLimit <= 0.02f) {
                        speed = laneLimit;
                    } else {
                        speed -= 0.02f;
                    }
                } else if (speed < laneLimit) {
                    speed += e->accelRate_0x290;
                    if (speed > laneLimit) {
                        speed = laneLimit;
                    }
                }
            }
        }

        e->followSpeedCap_0x418 = 9999.0f;

        if (speed != 0.0f) {
            e->pathDistance_0x2bc += speed;
            e->pathDistanceCopy_0x2c0 += speed;
            e->lookaheadCacheLen_0x4ec -= speed;

            if (e->driveState_0x2b4 == 2) {
                float pathDot = VecXZDot_8c0207d4((NJS_POINT3 *)&e->posX_0xf4,
                                                 (NJS_POINT3 *)&e->frontPointX_0x100,
                                                 (PointXZ *)&e->pathPointX_0x0ec);
                /* Path point is behind the entity. */
                if (pathDot < 0.0f) {
                    if (e->projectDistance_0x2c4 <= 2.0f) {
                        e->projectDistance_0x2c4 = GeomDistanceXZ_8c02081c(&e->posX_0xf4, &e->pathPointX_0x0ec);
                        if (e->projectDistance_0x2c4 >= 2.0f) {
                            e->projectDistance_0x2c4 = 2.0f;
                            e->driveState_0x2b4 = 0;
                        }
                    } else {
                        e->projectDistance_0x2c4 -= speed;
                        if (e->projectDistance_0x2c4 <= 2.0f) {
                            e->projectDistance_0x2c4 = 2.0f;
                            e->driveState_0x2b4 = 0;
                        }
                    }
                }
            }

            if (TrafficAdvanceOnPath_8c026ca2(0.0f, e) == 0) {
                if (e->attachmentWaitState_0x458 == 2) {
                    e->pendingAttachmentRelease_0x498 = e->attachmentId_0x45c;
                }
                if (TrafficRunEntryScript_8c027012(e) == 0) {
                    if (e->spawnPresetId_0x2f4 == var_activeTrafficPreset_8c227e14 &&
                        e->animKind_0x48c == 3) {
                        e->scriptBase_0x2f8 = e->scriptCursor_0x2fc;
                        TrafficReadScriptArgs_8c026710(e, e->scriptBase_0x2f8);
                        e->driveState_0x2b4 = 3;
                        return;
                    }
                    /* the script has nothing left to do this frame outside
                     * that reload case -- real asm behavior: this
                     * unconditionally despawns the entity, skipping the
                     * ring-buffer/blinker/render bookkeeping below,
                     * regardless of distance to the player. */
                    TaskKill_8c014b66(task);
                    return;
                }
                TrafficAdvanceOnPath_8c026ca2(0.0f, e);
            }
        }
    } else if (e->driveState_0x2b4 == 1) {
        /* ---- collision knockback ---- */
        if (e->groundProbe_0x190[0].attr_0x00 == 0 ||
            e->groundProbe_0x190[1].attr_0x00 == 0 ||
            e->groundProbe_0x190[2].attr_0x00 == 0 ||
            CollisionFindTaskHit_8c02e400(task, e) != 0) {
            speed = 0.0f;
        } else {
            float dx = speed * e->dirX_0x29c;
            float dz = speed * e->dirZ_0x2a0;
            heading = dx;
            e->posX_0xf4 += dx;
            e->posZ_0xfc += dz;
            e->frontPointX_0x100 += dx;
            e->frontPointZ_0x108 += dz;
            speed -= 0.1f;
        }

        if (speed <= 0.0f) {
            e->driveState_0x2b4 = 2;
            e->projectDistance_0x2c4 = GeomDistanceXZ_8c02081c(&e->posX_0xf4, &e->pathPointX_0x0ec);
            speed = 0.0f;
            e->groundProbe_0x190[0].count_0x0c = 0;
            e->groundProbe_0x190[1].count_0x0c = 0;
            e->groundProbe_0x190[2].count_0x0c = 0;
        }
    } else if (e->driveState_0x2b4 == 3) {
        /* ---- waiting for the reloaded script's own spawn box to clear ---- */
        if (TrafficPathScanBuild_8c02f0c8(task, e, e->resolvedArgs_0x304[0], 0, 0.0f, 8.0f) != 0) {
            return;
        }
        TrafficRunEntryScript_8c027012(e);
        return;
    }

    if (speed != 0.0f) {
        TrafficUpdateHeading_8c026bc4(heading, e);
    }

    /* ---- shared tail: ring buffer, blinker, distance/render dispatch ---- */
    {
        float accel = 0.0f;
        Sint32 i;
        /* real asm behavior: slot 3 is re-stored with its own (unchanged)
         * value after the shift below -- a genuine write, not just a
         * logical no-op, matched here for parity. Read through a local
         * rather than a bare self-assignment so the store survives. */
        float lastSlot = e->field_0x280[3];

        for (i = 0; i < 3; i++) {
            accel += e->field_0x280[i + 1];
            e->field_0x280[i] = e->field_0x280[i + 1];
        }
        e->field_0x280[3] = lastSlot;
        accel += speed - e->speed_0x27c;

        if (speed == 0.0f || (Sint32)e->pitchAngle_0x078 < 0) {
            e->blinker_0x080 |= 1;
        }
        e->blinker_0x080 |= e->extraLightFlags_0x510;

        if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_EVENING) {
            e->blinker_0x080 |= 0x10;
        } else if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT) {
            BusDrawFadeLights_8c028022((BusState *)e);
        }

        e->speed_0x27c = speed;

        {
            float dz = var_busState_8c1bb9d0.posZ_0x0fc - e->posZ_0xfc;
            float dx = var_busState_8c1bb9d0.posX_0x0f4 - e->posX_0xf4;
            e->busDistance_0x490 = njSqrt(dx * dx + dz * dz);
        }

        if (e->busDistance_0x490 <= 200.0f || e->spawnPresetId_0x2f4 == var_activeTrafficPreset_8c227e14) {
            e->mirrorVisible_0x268 = 0;
            BusDrawPlaceEntity_8c027c3c(e, accel);
            return;
        }
        TaskKill_8c014b66(task);
    }
}
