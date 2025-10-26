<?php

return [
    'groups' => [
        [
            'tests' => [
                "tests/012324/12324_task.php",
            ],
            'objects' => [
                "build/output/012324_peripheral_support_src.obj",
                "build/output/012324_peripheral_support_c.obj",
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
                "build/output/014f54_text_src.obj",
                "build/output/014f54_text_c.obj",
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
                "build/output/0100bc_sound_src.obj",
                "build/output/0100bc_sound_c.obj",
            ],
        ],
        [
            'tests' => ["tests/015ab8_title.php"],
            'objects' => [
                "build/output/015ab8_title_src.obj",
                "build/output/015ab8_title_c.obj",
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
                "build/output/0193c8_vm_menu_src.obj",
                "build/output/0193c8_vm_menu_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/0207d4.php",
            ],
            'objects' => [
                "build/output/0207d4_src.obj",
                "build/output/0207d4_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/016c58.php"
            ],
            'objects' => [
                "build/output/016c58_prompt_src.obj",
                "build/output/016c58_prompt_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/012f44.php",
            ],
            'objects' => [
                "build/output/012f44_src.obj",
                "build/output/012f44_c.obj",
            ],
        ],
        [
            'tests' => [
                "tests/011120/4338_initDatQueue_8c011124.php",
                "tests/011120/4384_AsqNop_11120.php",
                "tests/011120/4458_resetDatQueue_8c01116a.php",
                "tests/011120/4532_task_loadQueuedDats_8c0111b4.php",
                "tests/011120/4880_sortAndLoadDatQueue_8c011310.php",
                "tests/011120/5324_task_loadQueuedNjs_8c0114cc.php",
                "tests/011120/5814_sortAndLoadNjQueue_8c0116b6.php",
                "tests/011120/6052_freeNjQueue_8c0117a4.php",
                "tests/011120/6072_initTexlistQueue_8c0117b8.php",
                "tests/011120/6142_resetTexlistQueue_8c0117fe.php",
                "tests/011120/6172_AsqRequestTexlist_1181c.php",
                "tests/011120/6206_task_loadQueuedTexlists_8c01183e.php",
                "tests/011120/6648_loadTexlistQueue_8c0119f8.php",
                "tests/011120/6722_texlistQueueIsIdle_8c011a42.php",
                "tests/011120/6728_freeTexlistQueue_8c011a48.php",
                "tests/011120/6748_initPvmQueue_8c011a5c.php",
                "tests/011120/6848_AsqRequestPvm_11ac0.php",
                "tests/011120/6912_task_loadQueuedPvms_8c011b00.php",
                "tests/011120/7460_sortAndLoadPvmQueue_8c011d24.php",
                "tests/011120/7714_pvmQueueIsIdle_8c011e22.php",
                "tests/011120/7720_freePvmQueue_8c011e28.php",
                "tests/011120/7740_AsqReleaseAndFreeTexlist_11e3c.php",
                "tests/011120/7776_AsqFreeTexlist_11e60.php",
                "tests/011120/7808_task_processQueues_8c011e80.php",
                "tests/011120/7990_AsqInitQueues_11f36.php",
                "tests/011120/8044_AsqResetQueues_11f6c.php",
                "tests/011120/8062_AsqFreeQueues_11f7e.php",
                "tests/011120/8160_AsqProcessQueues_11fe0.php",
                "tests/011120/8240_AsqRequestNjPvmPairs_12030.php",
                "tests/011120/8446_AsqFreeNjPvmPairs_120fe.php",
                "tests/011120/8544_AsqSetSeedA_12160.php",
                "tests/011120/8550_AsqGetRandomA_12166.php",
                "tests/011120/8568_AsqGetRandomInRangeA_12178.php",
                "tests/011120/8610_AsqSetSeedB_121a2.php",
                "tests/011120/8616_AsqGetRandomB_121a8.php",
                "tests/011120/8638_AsqGetRandomInRangeB_121be.php",
                "tests/011120/8680_AsqFUN_121e8.php",
            ],
            'objects' => [
                "build/output/011120_asset_queues_src.obj",
                "build/output/011120_asset_queues_c.obj",
            ],
        ],
        [
            "tests" => [
                "tests/019e98_main_menu/19e98_MainMenuTask.php",
                "tests/019e98_main_menu/1a09a_switchToMainMenuTask.php",
            ],
            "objects" => [
                "build/output/019e98_main_menu_src.obj",
                "build/output/019e98_main_menu_c.obj",
            ]
        ],
        [
            "tests" => [
                "tests/016d2c/8c016d2c_interpolated.php",
                "tests/016d2c/8c016dc6_FUN.php",
                "tests/016d2c/8c016e6c_FUN.php",
                "tests/016d2c/8c016ed2_FUN.php",
                "tests/016d2c/8c016ee6_FUN.php",
                "tests/016d2c/8c016f98_FUN.php",
                "tests/016d2c/8c0170c6_FUN_pushDialogTask.php",
                "tests/016d2c/8c017108_FUN.php",
                "tests/016d2c/8c017126_FUN.php",
                "tests/016d2c/8c0172dc_FUN.php",
                "tests/016d2c/8c0173e6_FUN.php",
                "tests/016d2c/8c017420_FUN.php",
                "tests/016d2c/8c017590_drawCoursesButtons.php",
                "tests/016d2c/8c017718_StoryMenuTask.php",
                "tests/016d2c/8c017a20_FUN.php",
                "tests/016d2c/8c017ada_FreeRunMenuTask.php",
                "tests/016d2c/8c017d54_FUN.php",
                "tests/016d2c/8c017e18_CourseMenuSwitchFromTask.php",
                "tests/016d2c/8c017ef2_FUN.php",
            ],
            "objects" => [
                "build/output/016d2c_src.obj",
                "build/output/016d2c_c.obj",
            ]
        ]
    ],
];