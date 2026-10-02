/* @unit TrafficDrive */
#include <shinobi.h>

#include "025b98_traffic_drive.h"
#include "014a9c_tasks.h"
#include "026710_traffic.h"
#include "027958_bus_draw.h"
#include "02e400_collision.h"
#include "02e51c_attr_query.h"
#include "02df3c_traffic_lookahead.h"
#include "02f0c8_traffic_path_scan.h"
#include "0207d4_vec_xz.h"
#include "02081c_geom.h"
#include "028258_signal.h"
#include "013ae8_route.h"
#include "020214_drive_cue.h"
#include "1ba1c8_globals.h"

/* ====================
 * Functions
 * ====================
 */

/* A blocked frame ends the knockback outright instead of ramping it down
 * (original behavior). */
void TrafficDriveParkedTask_8c02656a(Task *task, TrafficEntry *e)
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

void TrafficDriveVehicleTask_8c025b98(Task *task, TrafficEntry *e)
{
    float speed = e->speed_0x27c;
    Sint32 signalId;
    /* For TrafficUpdateHeading_8c026bc4's unused parameter. */
    float heading = 0.0f;

    e->blinker_0x080 = 0;

    if (e->driveState_0x2b4 == 0 || e->driveState_0x2b4 == 2) {
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
        /* Uninitialized when stopped, as in the original. */
        e->signalId_0x410 = signalId;
        e->atGroundJunction_0x50c = 0;

        {
            /* Widened while braking, so the obstacle doesn't flap in and
             * out of range. */
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

        /* ---- turn ---- */
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

        /* ---- signal and crossing waits ---- */
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

            /* ---- lane change ---- */
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
                float stopDist = 9999.0f;
                float limit;
                float obstacleLimit = 9999.0f;
                float stopLimit = 9999.0f;
                Sint32 coastDown = 0;

                if (stopFlag == 0) {
                    e->curveLimitActive_0x428 = 0;
                } else {
                    stopDist = TrafficRemainingPathDistance_8c026fb0(e);
                    if (e->curveLimitActive_0x428 == 0) {
                        float slack = speed * 18.0f - stopDist + 2.409f;
                        if (slack > 0.5f || slack < 0.0f) {
                            stopDist = 9999.0f;
                        }
                    }
                }

                limit = e->followSpeedCap_0x418 <= e->laneOffsetRatio_0x414 ? e->followSpeedCap_0x418 : e->laneOffsetRatio_0x414;

                if (brakeDist != 9999.0f) {
                    obstacleLimit = brakeDist * 3000.0f / 108000.0f;
                    if (obstacleLimit <= 0.003f) {
                        obstacleLimit = 0.0f;
                    }
                }

                if (stopDist != 9999.0f) {
                    if (speed <= 0.09259259f) {
                        stopLimit = speed - 0.003f;
                        if (stopLimit < 0.0f) {
                            stopLimit = 0.0f;
                        }
                    } else {
                        stopLimit = (speed / 18.0f) * 17.0f;
                    }
                }

                if (obstacleLimit == 9999.0f && stopLimit == 9999.0f) {
                    coastDown = 1;
                } else if (stopLimit <= obstacleLimit) {
                    if (stopLimit < limit) {
                        limit = stopLimit;
                        e->curveLimitActive_0x428 = 1;
                    } else {
                        coastDown = 1;
                    }
                } else if (limit <= obstacleLimit) {
                    coastDown = 1;
                } else {
                    limit = obstacleLimit;
                    e->obstacleLimitActive_0x424 = 1;
                }
                if (coastDown != 0 && limit < speed) {
                    limit = speed - 0.003f;
                    if (limit < 0.0f) {
                        limit = 0.0f;
                    }
                }

                /* ---- integrate speed towards limit ---- */
                if (e->obstacleLimitActive_0x424 != 0 || e->curveLimitActive_0x428 != 0 || limit < speed) {
                    if (speed - limit <= 0.02f) {
                        speed = limit;
                    } else {
                        speed -= 0.02f;
                    }
                } else if (speed < limit) {
                    speed += e->accelRate_0x290;
                    if (speed > limit) {
                        speed = limit;
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
                    /* Despawns at once, whatever the distance to the bus. */
                    TaskKill_8c014b66(task);
                    return;
                }
                TrafficAdvanceOnPath_8c026ca2(0.0f, e);
            }
        }
    } else if (e->driveState_0x2b4 == 1) {
        /* ---- knockback ---- */
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
        /* ---- wait for the spawn box to clear after a script reload ---- */
        if (TrafficPathScanBuild_8c02f0c8(task, e, e->resolvedArgs_0x304[0], 0, 0.0f, 8.0f) != 0) {
            return;
        }
        TrafficRunEntryScript_8c027012(e);
        return;
    }

    if (speed != 0.0f) {
        TrafficUpdateHeading_8c026bc4(heading, e);
    }

    {
        float accel = 0.0f;
        Sint32 i;
        /* Original bug: slot 3 is stored back unchanged instead of getting
         * this frame's delta (BusTask_8c022bdc stores it), so the history
         * stays zero. */
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
