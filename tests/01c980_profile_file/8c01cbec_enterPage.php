<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * enterPage_8c01cbec: if the selected row's resgrp is already the
 * active sys resgrp there is nothing to load -- jingle and fade the page
 * in (STATE_PAGE_FADE_IN, 6). Otherwise request the resgrp and park in
 * STATE_PAGE_LOAD (5).
 */
return new class extends TestCase {
    const RESGROUP_INFO_SIZE = 0x10; // sizeof(ResourceGroupInfo)
    const MIDI = 0xd1d1d1d1;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_currentSysResGroupInfo_8c225fb0', 4);
        $this->setSize('_var_tex_8c157af8', 4);

        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);

        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);
        $this->setSize('_CourseMenuFreeResourceGroup_8c0185c4', 4);
        $this->setSize('_njGarbageTexture', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
    }

    // Address that init_rowResgrps_8c045148[$row] resolves to (&init_profileResgrps_8c0450d8[row + 1]).
    private function pageResgrpInfo(int $row): int
    {
        return $this->addressOf('_init_profileResgrps_8c0450d8') + (($row + 1) * self::RESGROUP_INFO_SIZE);
    }

    private function selectRow(int $row): void
    {
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x40, $row);
    }

    private function setSelected(int $value): void
    {
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, $value);
    }

    public function test_page_already_loaded_jump_option_plays_track_8(): void
    {
        $this->resolveSymbols();
        $this->selectRow(2);
        $this->setSelected(0); // PAGE_OPTION_JUMP_PREV
        $this->initUint32(
            $this->addressOf('_var_currentSysResGroupInfo_8c225fb0'),
            $this->pageResgrpInfo(2)
        );

        $this->shouldCall('_sdMidiPlay')->with(
            self::MIDI, 1, 8, 0
        );
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 6);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);

        $this->singleCall('_enterPage_8c01cbec')->run();
    }

    public function test_page_already_loaded_step_option_plays_track_7(): void
    {
        $this->resolveSymbols();
        $this->selectRow(2);
        $this->setSelected(1); // PAGE_OPTION_PREV
        $this->initUint32(
            $this->addressOf('_var_currentSysResGroupInfo_8c225fb0'),
            $this->pageResgrpInfo(2)
        );

        $this->shouldCall('_sdMidiPlay')->with(
            self::MIDI, 1, 7, 0
        );
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 6);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);

        $this->singleCall('_enterPage_8c01cbec')->run();
    }

    public function test_page_not_loaded_requests_resgrp(): void
    {
        $this->resolveSymbols();
        $this->selectRow(3);
        $this->setSelected(0);
        // Sentinel that never matches any valid ResourceGroupInfo address.
        $this->initUint32($this->addressOf('_var_currentSysResGroupInfo_8c225fb0'), 0);

        $this->shouldCall('_CourseMenuFreeResourceGroup_8c0185c4')->with(
            $this->addressOf('_var_resourceGroup_8c2263a8')
        );
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'), 0xc00
        );
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->addressOf('_var_resourceGroup_8c2263a8'),
            $this->pageResgrpInfo(3)
        );
        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0, 0, 0,
            $this->addressOf('_RouteClearLatch_8c014322'),
        );
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 5);

        $this->singleCall('_enterPage_8c01cbec')->run();
    }
};
