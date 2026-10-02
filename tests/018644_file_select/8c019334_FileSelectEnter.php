<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private int $ms;

    /*
     * The card already holds saves (SAVE_EXISTS_NO_SPACE): show the "loading,
     * don't power off" box, light the VMS LCD, and kick off the streaming load.
     */
    public function test_save_exists_no_space_loads(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(5);

        $this->call('_FileSelectEnter_8c019334')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')
            ->with($task, $this->addressOf('_fileSelectTask_8c018e7e'));
        $this->shouldWriteLong($this->addressOf('_var_loadedSaveCount_8c22600c'), 0);
        $this->shouldWriteLong($this->ms + 0x18, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('ロード実行中です<E>電源を切らないで下さい');
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(1);
        $this->shouldCall('_startVmLoad_8c018784');
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->tail();
    }

    /* Same path, reached via SAVE_EXISTS. */
    public function test_save_exists_loads(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(6);

        $this->call('_FileSelectEnter_8c019334')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')
            ->with($task, $this->addressOf('_fileSelectTask_8c018e7e'));
        $this->shouldWriteLong($this->addressOf('_var_loadedSaveCount_8c22600c'), 0);
        $this->shouldWriteLong($this->ms + 0x18, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('ロード実行中です<E>電源を切らないで下さい');
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(1);
        $this->shouldCall('_startVmLoad_8c018784');
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->tail();
    }

    /* No save on the card: build the card list straight away and start at READY (3). */
    public function test_build_file_list(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(0);

        $this->call('_FileSelectEnter_8c019334')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')
            ->with($task, $this->addressOf('_fileSelectTask_8c018e7e'));
        $this->shouldWriteLong($this->addressOf('_var_loadedSaveCount_8c22600c'), 0);
        $this->shouldCall('_buildFileList_8c018a22');
        $this->shouldWriteLong($this->ms + 0x18, 3);
        $this->tail();
    }

    /* Common exit: reset the cursor and fade in. */
    private function tail(): void
    {
        $this->shouldWriteLong($this->ms + 0x3c, 0);
        $this->shouldWriteLong($this->ms + 0x38, 0);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    private function setup(int $vmuStatus): void
    {
        foreach ([
            '_TaskSwitch_8c014b3e', '_MessageBoxSetText_8c02aefc',
            '_VmGameSetLcdAnim_8c01c8fc', '_RenderStartFadeIn_8c022a9c',
        ] as $fn) {
            $this->setSize($fn, 4);
        }

        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->setSize('_var_vmuStatus_8c226048', 0x24);
        $this->setSize('_var_vmBusy_8c157a7c', 4);

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), $vmuStatus);
    }
};
