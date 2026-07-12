<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itFreesPairs()
    {
        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $pairsPtr = $this->alloc(4);
        $this->initUint32($pairsPtr, $pairs);

        $this->initUint32($pairs + 0 * $pairSize + 0x0, 0xcafe0001);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, 0xcafe0002);

        $this->initUint32($pairs + 1 * $pairSize + 0x0, 0xcafe1001);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, 0xcafe1002);

        $this->initUint32($pairs + 2 * $pairSize + 0x0, 0);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, 0);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0xcafe0001);
        $this->shouldCall('_syFree')->with(0xcafe0002);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0xcafe1001);
        $this->shouldCall('_syFree')->with(0xcafe1002);

        $this->shouldCall('_syFree')->with($pairs);

        $this->shouldWrite($pairsPtr, -1);

        $this->singleCall('_AsqFreeModels_8c0120fe')
            ->with($pairsPtr)
            ->run();
    }

    public function test_itIgnoresAlreadyFreedList()
    {
        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $pairsPtr = $this->alloc(4);
        $this->initUint32($pairsPtr, -1);

        // Keeping this just to make sure it's not being called
        $this->initUint32($pairs + 0 * $pairSize + 0x0, 0xcafe0001);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, 0xcafe0002);

        $this->initUint32($pairs + 1 * $pairSize + 0x0, 0xcafe1001);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, 0xcafe1002);

        $this->initUint32($pairs + 2 * $pairSize + 0x0, 0);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, 0);

        $this->singleCall('_AsqFreeModels_8c0120fe')
            ->with($pairsPtr)
            ->run();
    }

    public function test_itSkipsUnsetTexlists()
    {
        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $pairsPtr = $this->alloc(4);
        $this->initUint32($pairsPtr, $pairs);

        $this->initUint32($pairs + 0 * $pairSize + 0x0, -1);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, 0xcafe0002);

        $this->initUint32($pairs + 1 * $pairSize + 0x0, 0xcafe1001);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, 0xcafe1002);

        $this->initUint32($pairs + 2 * $pairSize + 0x0, 0);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, 0);

        $this->shouldCall('_syFree')->with(0xcafe0002);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0xcafe1001);
        $this->shouldCall('_syFree')->with(0xcafe1002);

        $this->shouldCall('_syFree')->with($pairs);

        $this->shouldWrite($pairsPtr, -1);

        $this->singleCall('_AsqFreeModels_8c0120fe')
            ->with($pairsPtr)
            ->run();
    }

    public function test_itSkipsUnsetNj()
    {
        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $pairsPtr = $this->alloc(4);
        $this->initUint32($pairsPtr, $pairs);

        $this->initUint32($pairs + 0 * $pairSize + 0x0, 0xcafe0001);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, -1);

        $this->initUint32($pairs + 1 * $pairSize + 0x0, 0xcafe1001);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, 0xcafe1002);

        $this->initUint32($pairs + 2 * $pairSize + 0x0, 0);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, 0);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0xcafe0001);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0xcafe1001);
        $this->shouldCall('_syFree')->with(0xcafe1002);

        $this->shouldCall('_syFree')->with($pairs);

        $this->shouldWrite($pairsPtr, -1);

        $this->singleCall('_AsqFreeModels_8c0120fe')
            ->with($pairsPtr)
            ->run();
    }
};
