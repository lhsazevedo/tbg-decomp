<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_loads_common_resources()
    {
        $this->resolveSymbols();

        $menuStateBase = $this->addressOf('_menuState_8c1bc7a8');

        $this->call('_requestCommonResources_8c01852c');

        // Step 1: Request common_parts.dat
        $this->shouldCall('_AsqRequestDat_11182')->with(
            "\\SYSTEM",
            "common_parts.dat",
            $menuStateBase + 0x04  // resourceGroupA_0x00.tanim_0x04
        );

        // Step 2: Request common.dat
        $this->shouldCall('_AsqRequestDat_11182')->with(
            "\\SYSTEM",
            "common.dat",
            $menuStateBase + 0x08  // resourceGroupA_0x00.contents_0x08
        );

        // Step 3: Request common.pvm
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            "\\SYSTEM",
            "common.pvm",
            $menuStateBase,  // menuState address (for texlist)
            1,
            0
        );
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        // Allocate string constants in ROM
        $this->rellocate('_const_8c036290', $this->allocString("\\SYSTEM"));
        $this->rellocate('_const_8c036298', $this->allocString("common_parts.dat"));
        $this->rellocate('_const_8c0362ac', $this->allocString("common.dat"));
        $this->rellocate('_const_8c0362b8', $this->allocString("common.pvm"));
    }
};

