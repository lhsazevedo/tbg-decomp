<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_returns_zero_when_same_resource_group_already_loaded()
    {
        $this->resolveSymbols();

        $resGroup = $this->alloc(0x10);
        $resGroupInfo = $this->alloc(0x10);

        // Set var_8c225fb0 to same resource group
        $this->initUint32($this->addressOf('_var_8c225fb0'), $resGroupInfo);

        $this->call('_requestSysResgrp_8c018568')->with($resGroup, $resGroupInfo);

        // Should return 0 early without doing anything
        $this->shouldReturn(0);
    }

    public function test_loads_new_resource_group_without_freeing_when_empty()
    {
        $this->resolveSymbols();

        $resGroup = $this->alloc(0x10);
        $resGroupInfo = $this->alloc(0x10);

        // Initialize resource group as empty (tlist_0x00 = -1)
        $this->initUint32($resGroup + 0x00, 0xffffffff);

        // var_8c225fb0 points to different resource group
        $this->initUint32($this->addressOf('_var_8c225fb0'), 0x12345678);

        // Initialize resource group info fields
        $partsStr = $this->allocString("test_parts.dat");
        $datStr = $this->allocString("test.dat");
        $pvmStr = $this->allocString("test.pvm");
        $this->initUint32($resGroupInfo + 0x00, $partsStr);  // parts
        $this->initUint32($resGroupInfo + 0x04, $datStr);    // dat
        $this->initUint32($resGroupInfo + 0x08, $pvmStr);    // pvm
        $this->initUint32($resGroupInfo + 0x0c, 5);          // tex_count

        $this->call('_requestSysResgrp_8c018568')->with($resGroup, $resGroupInfo);

        // Step 1: Update var_8c225fb0 to point to new resource group
        $this->shouldWriteLongTo('_var_8c225fb0', $resGroupInfo);

        // Step 2: Request parts dat
        $this->shouldCall('_AsqRequestDat_11182')->with(
            "\\SYSTEM",
            "test_parts.dat",
            $resGroup + 0x04
        );

        // Step 3: Request main dat
        $this->shouldCall('_AsqRequestDat_11182')->with(
            "\\SYSTEM",
            "test.dat",
            $resGroup + 0x08
        );

        // Step 4: Request pvm
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            "\\SYSTEM",
            "test.pvm",
            $resGroup,
            5,
            0
        );

        // Should return 1
        $this->shouldReturn(1);
    }

    public function test_frees_old_resource_group_before_loading_new_one()
    {
        $this->resolveSymbols();

        $resGroup = $this->alloc(0x10);
        $resGroupInfo = $this->alloc(0x10);

        // Initialize resource group with valid tlist (not -1)
        $this->initUint32($resGroup + 0x00, 0xbebacafe);

        // var_8c225fb0 points to different resource group
        $this->initUint32($this->addressOf('_var_8c225fb0'), 0x12345678);

        // Initialize resource group info fields
        $partsStr = $this->allocString("course_parts.dat");
        $datStr = $this->allocString("course.dat");
        $pvmStr = $this->allocString("course.pvm");
        $this->initUint32($resGroupInfo + 0x00, $partsStr);  // parts
        $this->initUint32($resGroupInfo + 0x04, $datStr);    // dat
        $this->initUint32($resGroupInfo + 0x08, $pvmStr);    // pvm
        $this->initUint32($resGroupInfo + 0x0c, 3);          // tex_count

        $this->call('_requestSysResgrp_8c018568')->with($resGroup, $resGroupInfo);

        // Step 1: Update var_8c225fb0 to point to new resource group
        $this->shouldWriteLongTo('_var_8c225fb0', $resGroupInfo);

        // Step 2: Free old resource group since tlist_0x00 is not -1
        $this->shouldCall('_freeResourceGroup_8c0185c4')->with($resGroup);

        // Step 3: Request parts dat
        $this->shouldCall('_AsqRequestDat_11182')->with(
            "\\SYSTEM",
            "course_parts.dat",
            $resGroup + 0x04
        );

        // Step 4: Request main dat
        $this->shouldCall('_AsqRequestDat_11182')->with(
            "\\SYSTEM",
            "course.dat",
            $resGroup + 0x08
        );

        // Step 5: Request pvm
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            "\\SYSTEM",
            "course.pvm",
            $resGroup,
            3,
            0
        );

        // Should return 1
        $this->shouldReturn(1);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_8c225fb0', 4);

        // Allocate string constant for "\\SYSTEM"
        $this->rellocate('_const_8c036290', $this->allocString("\\SYSTEM"));
    }
};

