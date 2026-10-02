<?php

return [
    'callBlocklist' => ['/^LAB_/', '/^L\d+$/'],
    'sourcePaths' => [
        'Z:\\app\\src' => 'src',
    ],
    'groups' => [
        [
            // The matching build compiles this unit from C, so there is no
            // archived original asm to run these against -- the C object is
            // the only one.
            'tests' => [
                "tests/010e90_vibration/8c010e90_stepPattern.php",
                "tests/010e90_vibration/8c010f7a_VibStart.php",
                "tests/010e90_vibration/8c010fae_VibUpdate.php",
                "tests/010e90_vibration/8c010fbe_VibClear.php",
            ],
            'objects' => [
                "build/output_test/src/010e90_vibration.obj",
            ],
        ],
        [
            'tests' => [
                "tests/010fe8_heap/8c010fe8_heapInit.php",
                "tests/010fe8_heap/8c01102a_heapAlloc.php",
                "tests/010fe8_heap/8c0110c4_heapFree.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/010fe8_heap.obj",
                "build/output_test/src/010fe8_heap.obj",
            ],
        ],
        [
            'tests' => [
                "tests/014f54_sprite/8c014f54_SpriteDraw.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/014f54_sprite.obj",
                "build/output_test/src/014f54_sprite.obj",
            ],
        ],
        [
            'tests' => [
                "tests/015034_text/8c015034_getGlyphIndex.php",
                "tests/015034_text/8c015110_unpackGlyph.php",
                "tests/015034_text/8c01524c_TxtInit.php",
                "tests/015034_text/8c01529c_TxtDestroy.php",
                "tests/015034_text/8c0152fc_TxtCreateTextBox.php",
                "tests/015034_text/8c01543a_TxtPrepareTextBoxLayout.php",
                "tests/015034_text/8c0155e0_TxtDrawTextBox.php",
                "tests/015034_text/8c01594c_demoLoadTask.php",
                "tests/015034_text/8c0159ac_TxtStartAttractDemo.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/015034_text.obj",
                "build/output_test/src/015034_text.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0100bc_sound/8c0100bc_initEngineVol.php",
                "tests/0100bc_sound/8c010128_updateEngineVol.php",
                "tests/0100bc_sound/8c0102d8_updateEngineVoices.php",
                "tests/0100bc_sound/8c010972_SndSetAdxVol.php",
                "tests/0100bc_sound/8c010a40_SndUpdateAdxVolFade.php",
                "tests/0100bc_sound/8c010bae_SndStartAdxFadeOut.php",
                "tests/0100bc_sound/8c010c2c_unusedStartAdxCh1FadeIn.php",
                "tests/0100bc_sound/8c010cd6_SndPlayAdx.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0100bc_sound.obj",
                "build/output_test/src/0100bc_sound.obj",
            ],
        ],
        [
            'tests' => ["tests/015ab8_title.php"],
            'objects' => [
                "build/output_test/src/asm/decompiled/015ab8_title.obj",
                "build/output_test/src/015ab8_title.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0193c8_vm_select/8c0198a0_vmMenuTask.php",
                "tests/0193c8_vm_select/8c019852_drawVmWarning.php",
                "tests/0193c8_vm_select/8c0193c8_mountWaitTask.php",
                "tests/0193c8_vm_select/8c01940e_VmSelectMountAll.php",
                "tests/0193c8_vm_select/8c01946a_unmountTask.php",
                "tests/0193c8_vm_select/8c0194de_VmSelectUnmountAll.php",
                "tests/0193c8_vm_select/8c019504_VmSelectFreeAndClear.php",
                "tests/0193c8_vm_select/8c019550_VmSelectUpdateAllStatus.php",
                "tests/0193c8_vm_select/8c019e44_VmSelectEnter.php",
                "tests/0193c8_vm_select/8c01967c_VmSelectUpdateStatus.php",
                "tests/0193c8_vm_select/8c019730_saveFileExists.php",
                "tests/0193c8_vm_select/8c019788_initCursorLerp.php",
                "tests/0193c8_vm_select/8c0197c0_drawVmMenu.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0193c8_vm_select.obj",
                "build/output_test/src/0193c8_vm_select.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0207d4_vec_xz/8c0207d4_VecXZDot.php",
                "tests/0207d4_vec_xz/8c0207fa_VecXZCross.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0207d4_vec_xz.obj",
                "build/output_test/src/0207d4_vec_xz.obj",
            ],
        ],
        [
            'tests' => [
                "tests/020594_vehicle_model/8c020594_VehicleModelPlace.php",
                "tests/020594_vehicle_model/8c020676_unusedStepToward.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/020594_vehicle_model.obj",
                "build/output_test/src/020594_vehicle_model.obj",
            ],
        ],
        [
            'tests' => [
                "tests/02081c_geom/8c02081c_GeomDistanceXZ.php",
                "tests/02081c_geom/8c020842_GeomQuadOverlap.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/02081c_geom.obj",
                "build/output_test/src/02081c_geom.obj",
            ],
        ],
        [
            'tests' => [
                "tests/016c58.php"
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/016c58_prompt.obj",
                "build/output_test/src/016c58_prompt.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0129cc_game/8c0129cc_pauseUpdate.php",
                "tests/0129cc_game/8c012cbc_pauseTask.php",
                "tests/0129cc_game/8c012d06_pauseToggleTask.php",
                "tests/0129cc_game/8c012d5a_attractDemoTask.php",
                "tests/0129cc_game/game.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/0129cc_game.obj",
                "build/output_test/src/0129cc_game.obj",
            ],
        ],
        [
            'tests' => [
                "tests/011120_asset_queues/8c011124_initDatQueue.php",
                "tests/011120_asset_queues/8c011120_AsqNop.php",
                "tests/011120_asset_queues/8c01116a_resetDatQueue.php",
                "tests/011120_asset_queues/8c0111b4_taskLoadQueuedDats.php",
                "tests/011120_asset_queues/8c011310_sortAndLoadDatQueue.php",
                "tests/011120_asset_queues/8c0114cc_taskLoadQueuedNjs.php",
                "tests/011120_asset_queues/8c0116b6_sortAndLoadNjQueue.php",
                "tests/011120_asset_queues/8c0117a4_freeNjQueue.php",
                "tests/011120_asset_queues/8c0117b8_initTexlistQueue.php",
                "tests/011120_asset_queues/8c0117fe_resetTexlistQueue.php",
                "tests/011120_asset_queues/8c01181c_AsqRequestTexlist.php",
                "tests/011120_asset_queues/8c01183e_taskLoadQueuedTexlists.php",
                "tests/011120_asset_queues/8c0119f8_loadTexlistQueue.php",
                "tests/011120_asset_queues/8c011a42_texlistQueueIsIdle.php",
                "tests/011120_asset_queues/8c011a48_freeTexlistQueue.php",
                "tests/011120_asset_queues/8c011a5c_initPvmQueue.php",
                "tests/011120_asset_queues/8c011ac0_AsqRequestPvm.php",
                "tests/011120_asset_queues/8c011b00_taskLoadQueuedPvms.php",
                "tests/011120_asset_queues/8c011d24_sortAndLoadPvmQueue.php",
                "tests/011120_asset_queues/8c011e22_pvmQueueIsIdle.php",
                "tests/011120_asset_queues/8c011e28_freePvmQueue.php",
                "tests/011120_asset_queues/8c011e3c_AsqReleaseAndFreeTexlist.php",
                "tests/011120_asset_queues/8c011e60_asqFreeTexlist.php",
                "tests/011120_asset_queues/8c011e80_taskProcessQueues.php",
                "tests/011120_asset_queues/8c011f36_AsqInitQueues.php",
                "tests/011120_asset_queues/8c011f6c_AsqResetQueues.php",
                "tests/011120_asset_queues/8c011f7e_AsqFreeQueues.php",
                "tests/011120_asset_queues/8c011fe0_AsqProcessQueues.php",
                "tests/011120_asset_queues/8c012030_AsqRequestModels.php",
                "tests/011120_asset_queues/8c0120fe_AsqFreeModels.php",
                "tests/011120_asset_queues/8c012160_AsqSetSeedA.php",
                "tests/011120_asset_queues/8c012166_AsqGetRandomA.php",
                "tests/011120_asset_queues/8c012178_AsqGetRandomInRangeA.php",
                "tests/011120_asset_queues/8c0121a2_AsqSetSeedB.php",
                "tests/011120_asset_queues/8c0121a8_AsqGetRandomB.php",
                "tests/011120_asset_queues/8c0121be_AsqGetRandomInRangeB.php",
                "tests/011120_asset_queues/8c0121e8_AsqApplyButtonConfig.php",
            ],
            'objects' => [
                "build/output_test/src/asm/decompiled/011120_asset_queues.obj",
                "build/output_test/src/011120_asset_queues.obj",
            ],
        ],
        [
            "tests" => [
                "tests/019e98_main_menu/8c019e98_mainMenuTask.php",
                "tests/019e98_main_menu/8c01a09a_MainMenuEnter.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/019e98_main_menu.obj",
                "build/output_test/src/019e98_main_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016d2c_course_menu/8c016d2c_CourseMenuInterpolateCursor.php",
                "tests/016d2c_course_menu/8c016dc6_startCursorMove.php",
                "tests/016d2c_course_menu/8c016e6c_drawInteger.php",
                "tests/016d2c_course_menu/8c016ed2_getWeekDayIndex.php",
                "tests/016d2c_course_menu/8c016ee6_CourseMenuDrawDateAndExp.php",
                "tests/016d2c_course_menu/8c016f98_instructorDialogTask.php",
                "tests/016d2c_course_menu/8c0170c6_CourseMenuSpawnDialogTask.php",
                "tests/016d2c_course_menu/8c017108_swapDialogMessageBox.php",
                "tests/016d2c_course_menu/8c017126_handleCourseMenuInput.php",
                "tests/016d2c_course_menu/8c0172dc_CourseMenuBuildCourseUnlockList.php",
                "tests/016d2c_course_menu/8c0173e6_CourseMenuApplyUnlocks.php",
                "tests/016d2c_course_menu/8c017420_buildStoryDialogQueue.php",
                "tests/016d2c_course_menu/8c017590_drawCourseButtons.php",
                "tests/016d2c_course_menu/8c017718_storyMenuTask.php",
                "tests/016d2c_course_menu/8c017a20_buildFreeRunDialogQueue.php",
                "tests/016d2c_course_menu/8c017ada_freeRunMenuTask.php",
                "tests/016d2c_course_menu/8c017d54_refreshCourseGrid.php",
                "tests/016d2c_course_menu/8c017e18_CourseMenuEnter.php",
                "tests/016d2c_course_menu/8c017ef2_CourseMenuReturn.php",
                "tests/016d2c_course_menu/8c01803e_drawFixedInteger.php",
                "tests/016d2c_course_menu/8c018118_drawRouteInfo.php",
                "tests/016d2c_course_menu/8c0181b6_courseConfirmMenuTask.php",
                "tests/016d2c_course_menu/8c0184cc_courseMenuConfirmInit.php",
                "tests/016d2c_course_menu/8c01852c_CourseMenuRequestCommonResources.php",
                "tests/016d2c_course_menu/8c018568_CourseMenuRequestSysResgrp.php",
                "tests/016d2c_course_menu/8c0185c4_CourseMenuFreeResourceGroup.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/016d2c_course_menu.obj",
                "build/output_test/src/016d2c_course_menu.obj",
            ]
            ],
        [
            "tests" => [
                "tests/012324_input/8c012324_inputMenuTask.php",
                "tests/012324_input/8c012504_inputManualTask.php",
                "tests/012324_input/8c012718_inputAutoTask.php",
                "tests/012324_input/8c0128cc_InputSpawnTask.php",
                "tests/012324_input/8c012970_InputPollDrive.php",
                "tests/012324_input/8c012984_InputCheckColdBoot.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/012324_input.obj",
                "build/output_test/src/012324_input.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01d290_album/8c01d290_drawGrid.php",
                "tests/01d290_album/8c01d300_albumMenuTask.php",
                "tests/01d290_album/8c01d6e2_AlbumEnter.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01d290_album.obj",
                "build/output_test/src/01d290_album.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01d7fc_results/8c01d7fc_drawScoreDigits.php",
                "tests/01d7fc_results/8c01d864_drawPrompt.php",
                "tests/01d7fc_results/8c01df8e_spawnResultsTask.php",
                "tests/01d7fc_results/8c01e0b4_ResultsShowPassedRun.php",
                "tests/01d7fc_results/8c01e24e_ResultsShowFailedRun.php",
                "tests/01d7fc_results/8c01d8e0_resultsTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01d7fc_results.obj",
                "build/output_test/src/01d7fc_results.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02af78_event/8c02af78_setProgressFlag.php",
                "tests/02af78_event/8c02afbe_hasEventProgressFlag.php",
                "tests/02af78_event/8c02aff0_EventHasProfileProgressFlag.php",
                "tests/02af78_event/8c02b022_setRunEventFlag.php",
                "tests/02af78_event/8c02b030_hasRunEventFlag.php",
                "tests/02af78_event/8c02b03c_EventScanCandidates.php",
                "tests/02af78_event/8c02b170_EventPickForSegment.php",
                "tests/02af78_event/8c02b292_EventApplyFlags.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02af78_event.obj",
                "build/output_test/src/02af78_event.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02c884_stop/8c02c884_resetStopState.php",
                "tests/02c884_stop/8c02c8ae_pickWaitingPassengers.php",
                "tests/02c884_stop/8c02ca96_StopKillTaskGroup.php",
                "tests/02c884_stop/8c02caba_StopSetup.php",
                "tests/02c884_stop/8c02ccae_advanceStopSegment.php",
                "tests/02c884_stop/8c02cd6a_StopGetSegment.php",
                "tests/02c884_stop/8c02cd7a_StopGetArea.php",
                "tests/02c884_stop/8c02ccc6_StopUpdateHeadings.php",
                "tests/02c884_stop/8c02cd92_drawStopMarker.php",
                "tests/02c884_stop/8c02ce48_StopUpdateArrival.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02c884_stop.obj",
                "build/output_test/src/02c884_stop.obj",
            ]
        ],
        [
            "tests" => [
                "tests/013ae8_route/8c013ae8_requestVehicleAssets.php",
                "tests/013ae8_route/8c013b5a_RouteFreeVehicleAssets.php",
                "tests/013ae8_route/8c013bbc_RouteClearModelSlots.php",
                "tests/013ae8_route/8c013c34_syncRouteModelAssets.php",
                "tests/013ae8_route/8c013d42_finishAssetLoad.php",
                "tests/013ae8_route/8c013d78_RouteStartModelLoadPass.php",
                "tests/013ae8_route/8c013dae_RouteFreeAllModels.php",
                "tests/013ae8_route/8c013df6_syncPedestrianAssets.php",
                "tests/013ae8_route/8c013ee4_RouteFreePedestrianAssets.php",
                "tests/013ae8_route/8c013f22_freeSegmentAssets.php",
                "tests/013ae8_route/8c013f78_syncSegmentAssets.php",
                "tests/013ae8_route/8c014088_requestCourseAssets.php",
                "tests/013ae8_route/8c014322_RouteClearLatch.php",
                "tests/013ae8_route/8c01432a_RouteGetLatch.php",
                "tests/013ae8_route/8c014330_RouteSetLatch.php",
                "tests/013ae8_route/8c014338_routeLoadTask.php",
                "tests/013ae8_route/8c0144fc_RouteSpawnLoadTask.php",
                "tests/013ae8_route/8c014550_segmentReloadTask.php",
                "tests/013ae8_route/8c01468e_RouteSpawnSegmentReloadTask.php",
                "tests/013ae8_route/8c014784_RouteUnusedTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/013ae8_route.obj",
                "build/output_test/src/013ae8_route.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01614c_replay_menu/8c01614c_ReplayMenuKillDriveTasks.php",
                "tests/01614c_replay_menu/8c016182_ReplayMenuFreeSessionAssets.php",
                "tests/01614c_replay_menu/8c01628c_saveMenuTask.php",
                "tests/01614c_replay_menu/8c016636_openSaveMenu.php",
                "tests/01614c_replay_menu/8c01666a_listMenuTask.php",
                "tests/01614c_replay_menu/8c01673a_ReplayMenuOpen.php",
                "tests/01614c_replay_menu/8c016770_ReplayMenuResetDemoCursor.php",
                "tests/01614c_replay_menu/8c01677e_ReplayMenuDemoRecordTask.php",
                "tests/01614c_replay_menu/8c0167c0_startCourse.php",
                "tests/01614c_replay_menu/8c0167ca_replaySaveTask.php",
                "tests/01614c_replay_menu/8c016924_startReplaySave.php",
                "tests/01614c_replay_menu/8c0169bc_replayLoadTask.php",
                "tests/01614c_replay_menu/8c016bf4_ReplayMenuDemoPlayTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01614c_replay_menu.obj",
                "build/output_test/src/01614c_replay_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/018644_file_select/8c018644_loadFileTask.php",
                "tests/018644_file_select/8c018784_startVmLoad.php",
                "tests/018644_file_select/8c0187d0_FileSelectFreeBuffers.php",
                "tests/018644_file_select/8c018804_FileSelectIsSaveValid.php",
                "tests/018644_file_select/8c018862_FileSelectResetSettingDefaults.php",
                "tests/018644_file_select/8c0188bc_FileSelectResetKeyConfigDefaults.php",
                "tests/018644_file_select/8c0188dc_FileSelectResetSoundDefaults.php",
                "tests/018644_file_select/8c01890a_FileSelectResetProgress.php",
                "tests/018644_file_select/8c01895e_FileSelectResetNewGame.php",
                "tests/018644_file_select/8c0189d2_FileSelectResetOptionDefaults.php",
                "tests/018644_file_select/8c0189fc_FileSelectApplySoundSettings.php",
                "tests/018644_file_select/8c018a22_buildFileList.php",
                "tests/018644_file_select/8c018aa2_drawNumber.php",
                "tests/018644_file_select/8c018b4c_drawFileCard.php",
                "tests/018644_file_select/8c018d46_drawFileSelect.php",
                "tests/018644_file_select/8c018e7e_fileSelectTask.php",
                "tests/018644_file_select/8c019334_FileSelectEnter.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/018644_file_select.obj",
                "build/output_test/src/018644_file_select.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01a148_option/8c01a148_settingTask.php",
                "tests/01a148_option/8c01a3c0_enterSetting.php",
                "tests/01a148_option/8c01a3da_cycleValue.php",
                "tests/01a148_option/8c01a42a_drawSensitivityBar.php",
                "tests/01a148_option/8c01a4b4_keyConfigEditExit.php",
                "tests/01a148_option/8c01a50c_keyConfigTask.php",
                "tests/01a148_option/8c01a89c_enterKeyConfig.php",
                "tests/01a148_option/8c01a8b6_audioEditValue.php",
                "tests/01a148_option/8c01a904_soundTestFieldRead.php",
                "tests/01a148_option/8c01a926_soundTestFieldAdjust.php",
                "tests/01a148_option/8c01aaaa_soundTestFieldDraw.php",
                "tests/01a148_option/8c01ab08_audioTask.php",
                "tests/01a148_option/8c01afd8_enterAudio.php",
                "tests/01a148_option/8c01b00a_topMenuTask.php",
                "tests/01a148_option/8c01b122_OptionEnter.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01a148_option.obj",
                "build/output_test/src/01a148_option.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01b19c_system_menu/8c01b19c_SystemMenuApplyLoadedProgress.php",
                "tests/01b19c_system_menu/8c01b1c0_writeDecimalDigits.php",
                "tests/01b19c_system_menu/8c01b206_updateVmsComment.php",
                "tests/01b19c_system_menu/8c01b26c_SystemMenuWriteToVmu.php",
                "tests/01b19c_system_menu/8c01b3ac_menuTask.php",
                "tests/01b19c_system_menu/8c01ba64_SystemMenuEnter.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01b19c_system_menu.obj",
                "build/output_test/src/01b19c_system_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01bb48_vm_game/8c01bb48_advanceLcdAnim.php",
                "tests/01bb48_vm_game/8c01bc44_pollBupOp.php",
                "tests/01bb48_vm_game/8c01bd30_saveExecFile.php",
                "tests/01bb48_vm_game/8c01be90_drawSelectScreen.php",
                "tests/01bb48_vm_game/8c01bf2a_selectSlot.php",
                "tests/01bb48_vm_game/8c01bfec_vmGameTask.php",
                "tests/01bb48_vm_game/8c01bde4_defragDisk.php",
                "tests/01bb48_vm_game/8c01be30_loadFileEx.php",
                "tests/01bb48_vm_game/8c01be60_rewriteExecFile.php",
                "tests/01bb48_vm_game/8c01c880_VmGameEnter.php",
                "tests/01bb48_vm_game/8c01c8dc_VmGameResetLcdAnims.php",
                "tests/01bb48_vm_game/8c01c8fc_VmGameSetLcdAnim.php",
                "tests/01bb48_vm_game/8c01c910_VmGameUpdateLcd.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01bb48_vm_game.obj",
                "build/output_test/src/01bb48_vm_game.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01c980_profile_file/8c01c980_ProfileFileUpdateUnlocks.php",
                "tests/01c980_profile_file/8c01c9f2_drawUnlockGrid.php",
                "tests/01c980_profile_file/8c01cac8_drawPage.php",
                "tests/01c980_profile_file/8c01cbec_enterPage.php",
                "tests/01c980_profile_file/8c01ccec_menuTask.php",
                "tests/01c980_profile_file/8c01d1c4_ProfileFileEnter.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01c980_profile_file.obj",
                "build/output_test/src/01c980_profile_file.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01e27c/8c01e27c_lessonDescriptionTask.php",
                "tests/01e27c/8c01e576_enterDescription.php",
                "tests/01e27c/8c01e992_buildDialogQueue.php",
                "tests/01e27c/8c01e920_enterGuide.php",
                "tests/01e27c/8c01ead8_drawDigits.php",
                "tests/01e27c/8c01e63c_lessonGuideTask.php",
                "tests/01e27c/8c01ebc8_scrollTowardSelection.php",
                "tests/01e27c/8c01ebf2_lessonMenuTask.php",
                "tests/01e27c/8c01f114_PracticeMenuEnter.php",
                "tests/01e27c/8c01f21c_PracticeMenuReturn.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01e27c_practice_menu.obj",
                "build/output_test/src/01e27c_practice_menu.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01f3c0_ending/8c01f3c0_selectEndingDialog.php",
                "tests/01f3c0_ending/8c01f42c_updateDialogScene.php",
                "tests/01f3c0_ending/8c01f50e_scrollCreditsText.php",
                "tests/01f3c0_ending/8c01f658_creditsTask.php",
                "tests/01f3c0_ending/8c01f954_EndingStart.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01f3c0_ending.obj",
                "build/output_test/src/01f3c0_ending.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01fa78_hud/8c01fa78_showMark.php",
                "tests/01fa78_hud/8c01fa80_drawTimeDigits.php",
                "tests/01fa78_hud/8c01fbac_drawHud.php",
                "tests/01fa78_hud/8c01ff48_updateTask.php",
                "tests/01fa78_hud/8c02018c_HudSpawnTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/01fa78_hud.obj",
                "build/output_test/src/01fa78_hud.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02171c/8c02171c_TileStreamClearSlots.php",
                "tests/02171c/8c021724_TileStreamTeardown.php",
                "tests/02171c/8c02175a_TileStreamInit.php",
                "tests/02171c/8c0217de_lookupTile.php",
                "tests/02171c/8c021810_TileStreamLoad.php",
                "tests/02171c/8c02190a_TileStreamRequestUpload.php",
                "tests/02171c/8c021a24_TileStreamReleaseAll.php",
                "tests/02171c/8c021b34_TileStreamDrawTile.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02171c_tile_stream.obj",
                "build/output_test/src/02171c_tile_stream.obj",
            ]
        ],
        [
            "tests" => [
                "tests/022464_render/8c02239c_RenderResetQueues.php",
                "tests/022464_render/8c0223ea_RenderQueueDraw.php",
                "tests/022464_render/8c022420_RenderQueueDraw2.php",
                "tests/022464_render/8c022464_drawLayer_type0.php",
                "tests/022464_render/8c022464_drawLayer_types1to4.php",
                "tests/022464_render/8c022464_drawLayer_types5to6.php",
                "tests/022464_render/8c022464_drawLayer_multi.php",
                "tests/022464_render/8c022560_RenderDrawFrame_idle.php",
                "tests/022464_render/8c022560_RenderDrawFrame_fade_start.php",
                "tests/022464_render/8c022560_RenderDrawFrame_fade_complete.php",
                "tests/022464_render/8c022560_RenderDrawFrame_fade_out.php",
                "tests/022464_render/8c022560_RenderDrawFrame_arrival_no_mirror.php",
                "tests/022464_render/8c022560_RenderDrawFrame_arrival_variants.php",
                "tests/022464_render/8c022560_RenderDrawFrame_edge_states.php",
                "tests/022464_render/8c0228a2_RenderStartRunFade_fade_in_start.php",
                "tests/022464_render/8c022910_RenderDrawFrameMainOnly_idle.php",
                "tests/022464_render/8c022910_RenderDrawFrameMainOnly_fade_start.php",
                "tests/022464_render/8c022910_RenderDrawFrameMainOnly_fade_complete.php",
                "tests/022464_render/8c022910_RenderDrawFrameMainOnly_gate_draw.php",
                "tests/022464_render/8c022910_RenderDrawFrameMainOnly_edge_states.php",
                "tests/022464_render/8c022a54_fadeInTask.php",
                "tests/022464_render/8c022a9c_RenderStartFadeIn.php",
                "tests/022464_render/8c022ad0_fadeOutTask.php",
                "tests/022464_render/8c022b60_RenderStartFadeOut.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/022464_render.obj",
                "build/output_test/src/022464_render.obj",
            ]
        ],
        [
            "tests" => [
                "tests/028258_signal/8c028258_trafficSignalTask.php",
                "tests/028258_signal/8c02840c_snapPointToGround.php",
                "tests/028258_signal/8c0288b2_SignalGet.php",
                "tests/028258_signal/8c028900_SignalGetFrame.php",
                "tests/028258_signal/8c0288be_SignalFree.php",
                "tests/028258_signal/8c02890c_SignalClearPedCrossingFlags.php",
                "tests/028258_signal/8c028958_SignalClearCrossingOccupied.php",
                "tests/028258_signal/8c02897a_SignalMarkPedCrossing.php",
                "tests/028258_signal/8c028984_SignalMarkCrossingOccupied.php",
                "tests/028258_signal/8c02898e_SignalIsCrossingOccupied.php",
                "tests/028258_signal/8c028998_SignalIsPedCrossing.php",
                "tests/028258_signal/8c02833c_linkedTrafficSignalTask.php",
                "tests/028258_signal/8c0283d4_setTrafficSignalLightCallback.php",
                "tests/028258_signal/8c0283e8_execTrafficSignalGroupTask.php",
                "tests/028258_signal/8c02845a_SignalInit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/028258_signal.obj",
                "build/output_test/src/028258_signal.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0289ac_objects/8c0289ac_advancePedPathPos.php",
                "tests/0289ac_objects/8c028a38_drawPedestriansMirror.php",
                "tests/0289ac_objects/8c028b74_drawPedestrians.php",
                "tests/0289ac_objects/8c028dd0_ObjectsRelocatePedGroupLists.php",
                "tests/0289ac_objects/8c028de8_ObjectsRelocatePedGroupDefs.php",
                "tests/0289ac_objects/8c028e00_pedestrianTask.php",
                "tests/0289ac_objects/8c02903e_pedStaticObjectTask.php",
                "tests/0289ac_objects/8c029078_pedGroupTask.php",
                "tests/0289ac_objects/8c0293f6_pedestriansTask.php",
                "tests/0289ac_objects/8c0296d6_ObjectsInitPedestrianGroups.php",
                "tests/0289ac_objects/8c0297da_ObjectsKillPedestrianGroups.php",
                "tests/0289ac_objects/8c029868_resolveObjectChildren.php",
                "tests/0289ac_objects/8c029878_drawBlinkers.php",
                "tests/0289ac_objects/8c029904_routeBlinkerTask.php",
                "tests/0289ac_objects/8c029920_ObjectsInitBlinkers.php",
                "tests/0289ac_objects/8c029acc_ObjectsClearAssetRequestTable.php",
                "tests/0289ac_objects/8c029ad4_ObjectsStartAssetRequests.php",
                "tests/0289ac_objects/8c029cfe_ObjectsFreeAssetRequests.php",
                "tests/0289ac_objects/8c029e46_drawFlyByModel.php",
                "tests/0289ac_objects/8c029e68_flyByModelTask.php",
                "tests/0289ac_objects/8c029e94_rowFlyByTask.php",
                "tests/0289ac_objects/8c029f42_initDatBlob.php",
                "tests/0289ac_objects/8c029f54_advanceDatBlob.php",
                "tests/0289ac_objects/8c029f2a_drawDatModel.php",
                "tests/0289ac_objects/8c029fcc_rowDatTask.php",
                "tests/0289ac_objects/8c02a048_drawRowModel.php",
                "tests/0289ac_objects/8c02a08a_rowModelTask.php",
                "tests/0289ac_objects/8c02a0d6_drawRowMotionModel.php",
                "tests/0289ac_objects/8c02a120_rowMotionModelTask.php",
                "tests/0289ac_objects/8c02a1b2_drawRowSimpleModel.php",
                "tests/0289ac_objects/8c02a1f0_rowSimpleModelTask.php",
                "tests/0289ac_objects/8c02a206_drawRowMaterialModel.php",
                "tests/0289ac_objects/8c02a27c_rowMaterialModelTask.php",
                "tests/0289ac_objects/8c02a47c_drawFumiCrossing.php",
                "tests/0289ac_objects/8c02a4f8_fumiCrossingTask.php",
                "tests/0289ac_objects/8c02a322_resolveObjectGrandchildren.php",
                "tests/0289ac_objects/8c02a370_setGrandchildEvalFlags.php",
                "tests/0289ac_objects/8c02a5d0_setSimpleLightCallback.php",
                "tests/0289ac_objects/8c02a60e_execRowTaskGroupTask.php",
                "tests/0289ac_objects/8c02a6ac_ObjectsSpawnTasks.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/0289ac_objects.obj",
                "build/output_test/src/0289ac_objects.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02a9fc_message_box/8c02a9fc_relocateMessageText.php",
                "tests/02a9fc_message_box/8c02aa28_MessageBoxClearAssets.php",
                "tests/02a9fc_message_box/8c02aa36_MessageBoxRequestAssets.php",
                "tests/02a9fc_message_box/8c02ab7a_messageBoxTask.php",
                "tests/02a9fc_message_box/8c02ad8c_MessageBoxStart.php",
                "tests/02a9fc_message_box/8c02adee_MessageBoxFreeAssets.php",
                "tests/02a9fc_message_box/8c02ae3e_MessageBoxOpenTextbox.php",
                "tests/02a9fc_message_box/8c02aefc_MessageBoxSwapFor.php",
                "tests/02a9fc_message_box/8c02af1c_MessageBoxMenuTextboxText.php",
                "tests/02a9fc_message_box/8c02af32_MessageBoxFreeTextboxes.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02a9fc_message_box.obj",
                "build/output_test/src/02a9fc_message_box.obj",
            ]
        ],
        [
            "tests" => [
                "tests/026710_traffic/8c026710_TrafficResolvePathBlocks.php",
                "tests/026710_traffic/8c026748_initEntryState.php",
                "tests/026710_traffic/8c026bc4_TrafficUpdateHeading.php",
                "tests/026710_traffic/8c026ca2_TrafficAdvanceOnPath.php",
                "tests/026710_traffic/8c026da4_TrafficRelocatePlacementTable.php",
                "tests/026710_traffic/8c026dcc_TrafficMarkSignalIdsInUse.php",
                "tests/026710_traffic/8c026eaa_TrafficComputeBlockedSpeed.php",
                "tests/026710_traffic/8c026f7e_TrafficBlinkTurnSignal.php",
                "tests/026710_traffic/8c026fb0_TrafficRemainingPathDistance.php",
                "tests/026710_traffic/8c026fcc_TrafficSeekPathRecord.php",
                "tests/026710_traffic/8c027012_TrafficRunEntryScript.php",
                "tests/026710_traffic/8c0272b8_spawnEntry.php",
                "tests/026710_traffic/8c02756a_applyLighting.php",
                "tests/026710_traffic/8c0275d4_updateTask.php",
                "tests/026710_traffic/8c02769e_TrafficInit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/026710_traffic.obj",
                "build/output_test/src/026710_traffic.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020914/8c020914_GroundQueryFindPolygon.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/020914_ground_query.obj",
                "build/output_test/src/020914_ground_query.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020b6c/8c020b6c_GroundProbeTrackPolygon.php",
                "tests/020b6c/8c020f7e_GroundProbeInterpolateHeight.php",
                "tests/020b6c/8c020fe4_GroundProbeFindPolygonAtHeight.php",
                "tests/020b6c/8c021290_GroundProbeTrackPolygonAtHeight.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/020b6c_ground_probe.obj",
                "build/output_test/src/020b6c_ground_probe.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0206f0_intersect/8c0206f0_IntersectSegments.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/0206f0_intersect.obj",
                "build/output_test/src/0206f0_intersect.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02786c_vehicle_parts/8c02786c_VehiclePartsBind.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02786c_vehicle_parts.obj",
                "build/output_test/src/02786c_vehicle_parts.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e400_collision/8c02e400_CollisionFindTaskHit.php",
                "tests/02e400_collision/8c02e486_CollisionQueueReset.php",
                "tests/02e400_collision/8c02e48e_CollisionQueueAdd.php",
                "tests/02e400_collision/8c02e4ac_CollisionQueueTest.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02e400_collision.obj",
                "build/output_test/src/02e400_collision.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02f0c8_traffic_path_scan/8c02f0c8_TrafficPathScanBuild.php",
                "tests/02f0c8_traffic_path_scan/8c02f212_TrafficPathScanNext.php",
                "tests/02f0c8_traffic_path_scan/8c02f28a_TrafficPathScanJunctionOccupied.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02f0c8_traffic_path_scan.obj",
                "build/output_test/src/02f0c8_traffic_path_scan.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02b2f0_drive_msg/8c02b2f0_drawMsgGlyphRow.php",
                "tests/02b2f0_drive_msg/8c02b388_DriveMsgDraw.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02b2f0_drive_msg.obj",
                "build/output_test/src/02b2f0_drive_msg.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02b464_grading/8c02b464_adjust.php",
                "tests/02b464_grading/8c02b578_armCooldowns.php",
                "tests/02b464_grading/8c02b6d4_handleBump.php",
                "tests/02b464_grading/8c02b7ea_gradeWallHit.php",
                "tests/02b464_grading/8c02b864_gradeOffCourseSevere.php",
                "tests/02b464_grading/8c02b886_gradeOffCourse.php",
                "tests/02b464_grading/8c02b8b8_gradeSignals.php",
                "tests/02b464_grading/8c02b986_gradeLaneUse.php",
                "tests/02b464_grading/8c02bb1c_gradeIntersection.php",
                "tests/02b464_grading/8c02bcd8_gradeFrame.php",
                "tests/02b464_grading/8c02c072_taskCallback.php",
                "tests/02b464_grading/8c02c46a_GradeReset.php",
                "tests/02b464_grading/8c02c624_onFadeStopEnded.php",
                "tests/02b464_grading/8c02c76a_onFadeRunFailed.php",
                "tests/02b464_grading/8c02c586_GradeRunComplete.php",
                "tests/02b464_grading/8c02c69a_driveEndFadeTask.php",
                "tests/02b464_grading/8c02c738_beginDriveEnd.php",
                "tests/02b464_grading/8c02c784_GradeOnFadeDriveEnd.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02b464_grading.obj",
                "build/output_test/src/02b464_grading.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02df3c_traffic_lookahead/8c02df3c_TrafficLookaheadInit.php",
                "tests/02df3c_traffic_lookahead/8c02dfca_TrafficLookaheadScan.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02df3c_traffic_lookahead.obj",
                "build/output_test/src/02df3c_traffic_lookahead.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02f320_replay_codec/8c02f320_ReplayCodecInit.php",
                "tests/02f320_replay_codec/8c02f3a0_ReplayCodecGetBit.php",
                "tests/02f320_replay_codec/8c02f3e0_ReplayCodecGetBits.php",
                "tests/02f320_replay_codec/8c02f49c_ReplayCodecPutBit.php",
                "tests/02f320_replay_codec/8c02f4da_ReplayCodecPutBits.php",
                "tests/02f320_replay_codec/8c02f556_ReplayCodecSwapNodes.php",
                "tests/02f320_replay_codec/8c02f58a_ReplayCodecListInsert.php",
                "tests/02f320_replay_codec/8c02f636_ReplayCodecLzwFindChild.php",
                "tests/02f320_replay_codec/8c02f668_ReplayCodecLzwInsertChild.php",
                "tests/02f320_replay_codec/8c02f6ac_ReplayCodecLzwRemoveChild.php",
                "tests/02f320_replay_codec/8c02f704_ReplayCodecInitTables.php",
                "tests/02f320_replay_codec/8c02f740_ReplayCodecExtendDict.php",
                "tests/02f320_replay_codec/8c02f824_ReplayCodecWriteCode.php",
                "tests/02f320_replay_codec/8c02f892_ReplayCodecReadCode.php",
                "tests/02f320_replay_codec/8c02f934_ReplayCodecPack.php",
                "tests/02f320_replay_codec/8c02fa14_ReplayCodecUnpack.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02f320_replay_codec.obj",
                "build/output_test/src/02f320_replay_codec.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e2dc_bus_collision/8c02e2dc_BusCollisionFindHit.php",
                "tests/02e2dc_bus_collision/8c02e35a_findNextHit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02e2dc_bus_collision.obj",
                "build/output_test/src/02e2dc_bus_collision.obj",
            ]
        ],
        [
            "tests" => [
                "tests/023310_bus_init/8c023310_placeBus.php",
                "tests/023310_bus_init/8c023610_BusInitStart.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/023310_bus_init.obj",
                "build/output_test/src/023310_bus_init.obj",
            ]
        ],
        [
            "tests" => [
                "tests/022bdc_bus/8c022bdc_BusTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/022bdc_bus.obj",
                "build/output_test/src/022bdc_bus.obj",
            ]
        ],
        [
            "tests" => [
                "tests/024b4c_bus_camera/8c024b4c_BusCameraSaveState.php",
                "tests/024b4c_bus_camera/8c024b86_BusCameraRestoreState.php",
                "tests/024b4c_bus_camera/8c024f32_BusCameraApplyMode.php",
                "tests/024b4c_bus_camera/8c024bb8_BusCameraDrawBusModel.php",
                "tests/024b4c_bus_camera/8c024d6c_positionCamera.php",
                "tests/024b4c_bus_camera/8c025078_BusCameraUpdate.php",
                "tests/024b4c_bus_camera/8c025604_BusCameraUpdateMirror.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/024b4c_bus_camera.obj",
                "build/output_test/src/024b4c_bus_camera.obj",
            ]
        ],
        [
            "tests" => [
                "tests/023938_bus_drive/8c023bce_BusDriveEnterKnockback.php",
                "tests/023938_bus_drive/8c023bea_hitWall.php",
                "tests/023938_bus_drive/8c023938_BusDriveSampleGround.php",
                "tests/023938_bus_drive/8c023cba_BusDriveApplyGround.php",
                "tests/023938_bus_drive/8c023e7e_BusDriveFindLaneTarget.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/023938_bus_drive.obj",
                "build/output_test/src/023938_bus_drive.obj",
            ]
        ],
        [
            "tests" => [
                "tests/025870_cinematic/8c025870_CinematicBoardingCamera.php",
                "tests/025870_cinematic/8c0258ba_applyShotPosition.php",
                "tests/025870_cinematic/8c025906_CinematicUpdateCamera.php",
                "tests/025870_cinematic/8c0259e8_demoShotTask.php",
                "tests/025870_cinematic/8c025af4_CinematicStartTour.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/025870_cinematic.obj",
                "build/output_test/src/025870_cinematic.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02e51c_attr_query/8c02e51c_AttrQueryFindConvexPolygon.php",
                "tests/02e51c_attr_query/8c02eab4_AttrQueryFindConvexPolygonAtHeight.php",
                "tests/02e51c_attr_query/8c02e69c_AttrQueryFindPolygon.php",
                "tests/02e51c_attr_query/8c02ec50_AttrQueryFindPolygonAtHeight.php",
                "tests/02e51c_attr_query/8c02f08a_AttrQueryRegionOccupied.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02e51c_attr_query.obj",
                "build/output_test/src/02e51c_attr_query.obj",
            ]
        ],
        [
            "tests" => [
                "tests/027958_bus_draw/8c027958_BusDrawUpdateModels.php",
                "tests/027958_bus_draw/8c0281ac_BusDrawSignal.php",
                "tests/027958_bus_draw/8c028206_BusDrawSignalAttachment.php",
                "tests/027958_bus_draw/8c028022_BusDrawFadeLights.php",
                "tests/027958_bus_draw/8c027a88_drawAhead.php",
                "tests/027958_bus_draw/8c027bac_drawMirror.php",
                "tests/027958_bus_draw/8c027c3c_BusDrawPlaceEntity.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/027958_bus_draw.obj",
                "build/output_test/src/027958_bus_draw.obj",
            ]
        ],
        [
            "tests" => [
                "tests/025b98_traffic_drive/8c02656a_TrafficDriveParkedTask.php",
                "tests/025b98_traffic_drive/8c025b98_TrafficDriveVehicleTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/025b98_traffic_drive.obj",
                "build/output_test/src/025b98_traffic_drive.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02d19c_passenger/8c02d19c_drawPassengerSprite.php",
                "tests/02d19c_passenger/8c02d1f4_drawInterior.php",
                "tests/02d19c_passenger/8c02d5ca_PassengerSeatedTask.php",
                "tests/02d19c_passenger/8c02d5d8_setCountUpStep.php",
                "tests/02d19c_passenger/8c02d21c_PassengerBoardTask.php",
                "tests/02d19c_passenger/8c02d46c_PassengerExitTask.php",
                "tests/02d19c_passenger/8c02d644_PassengerStopSceneTask.php",
                "tests/02d19c_passenger/8c02d8f0_PassengerSkipStopTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02d19c_passenger.obj",
                "build/output_test/src/02d19c_passenger.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02d968_stop_spawn/8c02d968_StopSpawnInit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02d968_stop_spawn.obj",
                "build/output_test/src/02d968_stop_spawn.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02d06c_stop_draw/8c02d06c_StopDrawWaitingPassengers.php",
                "tests/02d06c_stop_draw/8c02d0fc_StopDrawLightBegin.php",
                "tests/02d06c_stop_draw/8c02d146_StopDrawLightEnd.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02d06c_stop_draw.obj",
                "build/output_test/src/02d06c_stop_draw.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02412c_bus_line/8c02412c_BusLineAdvance.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/02412c_bus_line.obj",
                "build/output_test/src/02412c_bus_line.obj",
            ]
        ],
        [
            "tests" => [
                "tests/020214_drive_cue/8c020214_driveCueTask.php",
                "tests/020214_drive_cue/8c020528_DriveCueInit.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/020214_drive_cue.obj",
                "build/output_test/src/020214_drive_cue.obj",
            ]
        ],
        [
            "tests" => [
                "tests/021b9c_tile_draw/8c021b9c_drawTileGrid.php",
                "tests/021b9c_tile_draw/8c021ec4_drawTileGridMirror.php",
                "tests/021b9c_tile_draw/8c0221d0_enqueueTask.php",
                "tests/021b9c_tile_draw/8c0222dc_TileDrawSpawnTask.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/021b9c_tile_draw.obj",
                "build/output_test/src/021b9c_tile_draw.obj",
            ]
        ],
        [
            "tests" => [
                "tests/024280_bus_input/8c024280_BusInputCapMirrorTraffic.php",
                "tests/024280_bus_input/8c0242ce_updateGearSelector.php",
                "tests/024280_bus_input/8c024320_applyThrottle.php",
                "tests/024280_bus_input/8c024530_applyBraking.php",
                "tests/024280_bus_input/8c024606_applyBrakingSfx.php",
                "tests/024280_bus_input/8c0246b2_BusInputUpdate.php",
            ],
            "objects" => [
                "build/output_test/src/asm/decompiled/024280_bus_input.obj",
                "build/output_test/src/024280_bus_input.obj",
            ]
        ],
    ],
];
