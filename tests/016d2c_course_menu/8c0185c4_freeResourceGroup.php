<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_does_nothing_when_resource_group_empty()
    {
        $this->resolveSymbols();

        $resGroup = $this->alloc(0x10);

        // Set tlist_0x00 to -1 (empty/uninitialized)
        $this->initUint32($resGroup + 0x00, 0xffffffff);

        $this->call('_CourseMenuFreeResourceGroup_8c0185c4')->with($resGroup);

        // Function should return early without doing anything
    }

    public function test_frees_all_resource_group_components()
    {
        $this->resolveSymbols();

        $resGroup = $this->alloc(0x10);

        // Set resource group fields with valid pointers
        $tlist = 0xbeba0000;
        $contents = 0xbeba1000;
        $tanim = 0xbeba2000;
        
        $this->initUint32($resGroup + 0x00, $tlist);      // tlist_0x00
        $this->initUint32($resGroup + 0x04, $tanim);      // tanim_0x04
        $this->initUint32($resGroup + 0x08, $contents);   // contents_0x08

        $this->call('_CourseMenuFreeResourceGroup_8c0185c4')->with($resGroup);

        // Step 1: Release and free texlist
        $this->shouldCall('_AsqReleaseAndFreeTexlist_11e3c')->with($tlist);

        // Step 2: Free contents
        $this->shouldCall('_syFree')->with($contents);

        // Step 3: Free tanim
        $this->shouldCall('_syFree')->with($tanim);

        // Step 4: Mark resource group as empty by setting tlist_0x00 to -1
        $this->shouldWriteLong($resGroup + 0x00, 0xffffffff);
    }

    private function resolveSymbols(): void
    {
        // No additional symbols needed
    }
};

