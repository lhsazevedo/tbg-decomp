<?php

return [
    'sourcePaths' => [
        'Z:\\app\\src' => 'src',
    ],
    'groups' => [
        [
            'tests' => [
                "tests/010fe8_heap/8c010fe8_heapInit.php",
                "tests/010fe8_heap/8c01102a_heapAlloc.php",
                "tests/010fe8_heap/8c0110c4_heapFree.php",
            ],
            'objects' => [
                "build/output_test/010fe8_heap_src.obj",
                "build/output_test/010fe8_heap_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012324/12324_task.php",
            ],
            'objects' => [
                "build/output_test/012324_peripheral_support_src.obj",
                "build/output_test/012324_peripheral_support_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/014f54_text/14f54_drawSprite.php",
                "tests/014f54_text/15034_getGlyphDatOffset.php",
                "tests/014f54_text/15110_unpackGlyphTexture.php",
                "tests/014f54_text/1524c_TxtInit.php",
                "tests/014f54_text/1529c_TxtDestroy.php",
                "tests/014f54_text/152fc_TxtCreateTextBox.php",
                "tests/014f54_text/1543a_TxtPrepareTextBoxLayout.php",
                "tests/014f54_text/155e0_TxtDrawTextbox.php",
                "tests/014f54_text/1594c_FUN.php",
                "tests/014f54_text/159ac_FUN_demo.php",
            ],
            'objects' => [
                "build/output_test/014f54_text_src.obj",
                "build/output_test/014f54_text_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0100bc_sound/0100bc_initUknVol.php",
                "tests/0100bc_sound/010128_midiSetVol.php",
                "tests/0100bc_sound/0102d8_FUN.php",
                "tests/0100bc_sound/010972_setAdxVol.php",
                "tests/0100bc_sound/010a40_FUN_adxVol.php",
                "tests/0100bc_sound/010bae_FUN.php",
                "tests/0100bc_sound/010c2c_FUN.php",
                "tests/0100bc_sound/010cd6_snd.php",
            ],
            'objects' => [
                "build/output_test/0100bc_sound_src.obj",
                "build/output_test/0100bc_sound_c.obj",
            ],
        ],
        [
            'tests' => ["tests/015ab8_title.php"],
            'objects' => [
                "build/output_test/015ab8_title_src.obj",
                "build/output_test/015ab8_title_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0193c8_vm_menu/198a0_VmMenuTask.php",
                "tests/0193c8_vm_menu/19852_drawVmuWarning.php",
                "tests/0193c8_vm_menu/193c8_TaskWaitForVmsReady.php",
                "tests/0193c8_vm_menu/1940e_VmMenuMountVms.php",
                "tests/0193c8_vm_menu/1946a_TaskUnmountVms.php",
                "tests/0193c8_vm_menu/194de_VmMenuUnmountVms.php",
                "tests/0193c8_vm_menu/19504_VmMenuFreeAndClear.php",
                "tests/0193c8_vm_menu/19550_fetchVmusStatus.php",
                "tests/0193c8_vm_menu/19e44_VmMenuSwitchFromTask.php",
                "tests/0193c8_vm_menu/1967c_VmMenuUpdateVmuStatus.php",
                "tests/0193c8_vm_menu/19730_saveFileExists.php",
                "tests/0193c8_vm_menu/19788_initCursorLerp.php",
                "tests/0193c8_vm_menu/197c0_drawVmMenu.php",
            ],
            'objects' => [
                "build/output_test/0193c8_vm_menu_src.obj",
                "build/output_test/0193c8_vm_menu_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0207d4.php",
            ],
            'objects' => [
                "build/output_test/0207d4_src.obj",
                "build/output_test/0207d4_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/016c58.php"
            ],
            'objects' => [
                "build/output_test/016c58_prompt_src.obj",
                "build/output_test/016c58_prompt_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012f44_game.php",
            ],
            'objects' => [
                "build/output_test/012f44_game_src.obj",
                "build/output_test/012f44_game_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/011120/4338_initDatQueue_8c011124.php",
                "tests/011120/4384_AsqNop_11120.php",
                "tests/011120/4458_resetDatQueue_8c01116a.php",
                "tests/011120/4532_taskLoadQueuedDats_8c0111b4.php",
                "tests/011120/4880_sortAndLoadDatQueue_8c011310.php",
                "tests/011120/5324_taskLoadQueuedNjs_8c0114cc.php",
                "tests/011120/5814_sortAndLoadNjQueue_8c0116b6.php",
                "tests/011120/6052_freeNjQueue_8c0117a4.php",
                "tests/011120/6072_initTexlistQueue_8c0117b8.php",
                "tests/011120/6142_resetTexlistQueue_8c0117fe.php",
                "tests/011120/6172_AsqRequestTexlist_1181c.php",
                "tests/011120/6206_taskLoadQueuedTexlists_8c01183e.php",
                "tests/011120/6648_loadTexlistQueue_8c0119f8.php",
                "tests/011120/6722_texlistQueueIsIdle_8c011a42.php",
                "tests/011120/6728_freeTexlistQueue_8c011a48.php",
                "tests/011120/6748_initPvmQueue_8c011a5c.php",
                "tests/011120/6848_AsqRequestPvm_11ac0.php",
                "tests/011120/6912_taskLoadQueuedPvms_8c011b00.php",
                "tests/011120/7460_sortAndLoadPvmQueue_8c011d24.php",
                "tests/011120/7714_pvmQueueIsIdle_8c011e22.php",
                "tests/011120/7720_freePvmQueue_8c011e28.php",
                "tests/011120/7740_AsqReleaseAndFreeTexlist_11e3c.php",
                "tests/011120/7776_AsqFreeTexlist_11e60.php",
                "tests/011120/7808_taskProcessQueues_8c011e80.php",
                "tests/011120/7990_AsqInitQueues_11f36.php",
                "tests/011120/8044_AsqResetQueues_11f6c.php",
                "tests/011120/8062_AsqFreeQueues_11f7e.php",
                "tests/011120/8160_AsqProcessQueues_11fe0.php",
                "tests/011120/8240_AsqRequestModels_12030.php",
                "tests/011120/8446_AsqFreeModels_120fe.php",
                "tests/011120/8544_AsqSetSeedA_12160.php",
                "tests/011120/8550_AsqGetRandomA_12166.php",
                "tests/011120/8568_AsqGetRandomInRangeA_12178.php",
                "tests/011120/8610_AsqSetSeedB_121a2.php",
                "tests/011120/8616_AsqGetRandomB_121a8.php",
                "tests/011120/8638_AsqGetRandomInRangeB_121be.php",
                "tests/011120/8680_AsqApplyButtonConfig_121e8.php",
            ],
            'objects' => [
                "build/output_test/011120_asset_queues_src.obj",
                "build/output_test/011120_asset_queues_c.obj",
            ],
        ],
        [
            "tests" => [
                "tests/019e98_main_menu/19e98_MainMenuTask.php",
                "tests/019e98_main_menu/1a09a_switchToMainMenuTask.php",
            ],
            "objects" => [
                "build/output_test/019e98_main_menu_src.obj",
                "build/output_test/019e98_main_menu_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016d2c_course_menu/8c016d2c_CourseMenuInterpolateCursor.php",
                "tests/016d2c_course_menu/8c016dc6_cursorOffTarget.php",
                "tests/016d2c_course_menu/8c016e6c_drawInteger.php",
                "tests/016d2c_course_menu/8c016ed2_getWeekDayIndex.php",
                "tests/016d2c_course_menu/8c016ee6_CourseMenuDrawDateAndExp.php",
                "tests/016d2c_course_menu/8c016f98_dialogSequenceTask.php",
                "tests/016d2c_course_menu/8c0170c6_FUN_pushDialogTask.php",
                "tests/016d2c_course_menu/8c017108_swapDialogMessageBox.php",
                "tests/016d2c_course_menu/8c017126_handleCourseMenuInput.php",
                "tests/016d2c_course_menu/8c0172dc_CourseMenuBuildCourseUnlockList.php",
                "tests/016d2c_course_menu/8c0173e6_CourseMenuApplyUnlocks.php",
                "tests/016d2c_course_menu/8c017420_buildCourseMenuDialogFlow.php",
                "tests/016d2c_course_menu/8c017590_drawCourseButtons.php",
                "tests/016d2c_course_menu/8c017718_StoryMenuTask.php",
                "tests/016d2c_course_menu/8c017a20_buildFreeRunMenuDialogFlow.php",
                "tests/016d2c_course_menu/8c017ada_FreeRunMenuTask.php",
                "tests/016d2c_course_menu/8c017d54_FUN.php",
                "tests/016d2c_course_menu/8c017e18_CourseMenuSwitchFromTask.php",
                "tests/016d2c_course_menu/8c017ef2_FUN.php",
                "tests/016d2c_course_menu/8c01803e_drawFixedInteger.php",
                "tests/016d2c_course_menu/8c018118_drawRouteInfo.php",
                "tests/016d2c_course_menu/8c0181b6_courseConfirmMenuTask.php",
                "tests/016d2c_course_menu/8c0184cc_courseMenuConfirmInit.php",
                "tests/016d2c_course_menu/8c01852c_requestCommonResources.php",
                "tests/016d2c_course_menu/8c018568_requestSysResgrp.php",
                "tests/016d2c_course_menu/8c0185c4_freeResourceGroup.php",
            ],
            "objects" => [
                "build/output_test/016d2c_course_menu_src.obj",
                "build/output_test/016d2c_course_menu_c.obj",
            ]
            ],
        [
            "tests" => [
                "tests/012504_input/8c012504_task.php",
                "tests/012504_input/8c012718_inputTaskAlt.php",
                "tests/012504_input/8c0128cc_InputPushTask.php",
                "tests/012504_input/8c012970_InputDispatchTask.php",
                "tests/012504_input/8c012984_InputSetName.php",
            ],
            "objects" => [
                "build/output_test/012504_input_src.obj",
                "build/output_test/012504_input_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016bf4_demo_input/8c016bf4_DemoInputTask.php"
            ],
            "objects" => [
                "build/output_test/016bf4_demo_input_src.obj",
                "build/output_test/016bf4_demo_input_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01d290_album/1d290_AlbumDrawGrid.php",
                "tests/01d290_album/1d300_AlbumMenuTask.php",
                "tests/01d290_album/1d6e2_AlbumSwitchFromTask.php",
            ],
            "objects" => [
                "build/output_test/01d290_album_src.obj",
                "build/output_test/01d290_album_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/02af78_event/8c02af78_setProgressFlag.php",
                "tests/02af78_event/8c02afbe_hasProgressFlag.php",
                "tests/02af78_event/8c02aff0_hasProgressFlagAlt.php",
                "tests/02af78_event/8c02b022_setRunEventFlag.php",
                "tests/02af78_event/8c02b030_hasRunEventFlag.php",
                "tests/02af78_event/8c02b03c_EventScanCandidates.php",
                "tests/02af78_event/8c02b170_EventPickForSegment.php",
                "tests/02af78_event/8c02b292_EventApplyFlags.php",
            ],
            "objects" => [
                "build/output_test/02af78_event_src.obj",
                "build/output_test/02af78_event_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/013ae8_route_load/13ae8_requestVehicleAssets.php",
                "tests/013ae8_route_load/13b5a_freeVehicleAssets.php",
                "tests/013ae8_route_load/13bbc_RouteLoadClearModelSlots.php",
                "tests/013ae8_route_load/13c34_syncRouteModelAssets.php",
                "tests/013ae8_route_load/13d42_finishAssetLoad.php",
                "tests/013ae8_route_load/13d78_RouteLoadStartRouteModelLoadPass.php",
                "tests/013ae8_route_load/13dae_RouteLoadFreeAllRouteModels.php",
                "tests/013ae8_route_load/13df6_syncPedestrianAssets.php",
                "tests/013ae8_route_load/13ee4_RouteLoadFreePedestrianAssets.php",
                "tests/013ae8_route_load/13f22_freeSegmentModels.php",
                "tests/013ae8_route_load/13f78_syncSegmentModels.php",
                "tests/013ae8_route_load/14088_loadRouteModels.php",
                "tests/013ae8_route_load/14322_RouteLoadResetPvmReady.php",
                "tests/013ae8_route_load/1432a_RouteLoadIsPvmReady.php",
                "tests/013ae8_route_load/14330_RouteLoadSetPvmReady.php",
                "tests/013ae8_route_load/14338_routeLoadTask.php",
                "tests/013ae8_route_load/144fc_RouteLoadPushTask.php",
                "tests/013ae8_route_load/14550_unknownSegmentReloadTask.php",
                "tests/013ae8_route_load/1468e_RouteLoadPushSegmentReloadTask.php",
                "tests/013ae8_route_load/14784_RouteLoadUnusedTask.php",
            ],
            "objects" => [
                "build/output_test/013ae8_route_load_src.obj",
                "build/output_test/013ae8_route_load_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/0129cc_pause/8c0129cc_Update.php",
                "tests/0129cc_pause/8c012cbc_PauseTask.php",
                "tests/0129cc_pause/8c012d06_PauseToggleTask.php",
                "tests/0129cc_pause/8c012d5a_PauseDemoEndTask.php",
            ],
            "objects" => [
                "build/output_test/0129cc_pause_src.obj",
                "build/output_test/0129cc_pause_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/01614c_debug_menu/8c01614c_FUN.php",
                "tests/01614c_debug_menu/8c016182_FUN.php",
                "tests/01614c_debug_menu/8c01628c_saveMenuTask.php",
                "tests/01614c_debug_menu/8c016636_openSaveMenu.php",
                "tests/01614c_debug_menu/8c01666a_listMenuTask.php",
                "tests/01614c_debug_menu/8c01673a_DebugMenuOpen.php",
                "tests/01614c_debug_menu/8c016770_FUN.php",
                "tests/01614c_debug_menu/8c01677e_DebugMenuDemoRecordTask.php",
                "tests/01614c_debug_menu/8c0167c0_FUN.php",
                "tests/01614c_debug_menu/8c0167ca_replaySaveTask.php",
                "tests/01614c_debug_menu/8c016924_startReplaySave.php",
                "tests/01614c_debug_menu/8c0169bc_replayLoadTask.php",
            ],
            "objects" => [
                "build/output_test/01614c_debug_menu_src.obj",
                "build/output_test/01614c_debug_menu_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/018644/8c018644_VmLoadTask.php",
            ],
            "objects" => [
                "build/output_test/018644_vm_load_src.obj",
                "build/output_test/018644_vm_load_c.obj",
            ]
        ],
    ],
];
