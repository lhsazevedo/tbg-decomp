<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_requestsPairs()
    {
        $basedir = $this->allocString('\\DIR');

        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $destPairSize = 8;
        $destPairCount = 3;
        $destPairs = $this->alloc($destPairSize * $destPairCount);

        $pairANjFilename = $this->allocString('A.NJ');
        $pairAPvmFilename = $this->allocString('A.PVM');
        $this->initUint32($pairs + 0 * $pairSize + 0x0, $pairANjFilename);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, $pairAPvmFilename);

        $pairBNjFilename = $this->allocString('B.NJ');
        $pairBPvmFilename = $this->allocString('B.PVM');
        $this->initUint32($pairs + 1 * $pairSize + 0x0, $pairBNjFilename);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, $pairBPvmFilename);

        // Last pair has empty strings
        $emptyString = $this->allocString('');
        $this->initUint32($pairs + 2 * $pairSize + 0x0, $emptyString);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, $emptyString);

        $this->shouldCall('_syMalloc')
            ->with(3 * $destPairSize)
            ->andReturn($destPairs);

        // First pair
        $this->shouldCall('_AsqRequestNj_11492')
            ->with(
                $basedir,
                $pairANjFilename,
                0,
                $destPairs + 0 * $destPairSize + 0x4
            )
            ->andReturn(1);

        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with(
                $basedir,
                $pairAPvmFilename,
                $destPairs + 0 * $destPairSize + 0x0,
                3,
                0
            )
            ->andReturn(1);

        // Second pair
        $this->shouldCall('_AsqRequestNj_11492')
            ->with(
                $basedir,
                $pairBNjFilename,
                0,
                $destPairs + 1 * $destPairSize + 0x4
            )
            ->andReturn(1);

        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with(
                $basedir,
                $pairBPvmFilename,
                $destPairs + 1 * $destPairSize + 0x0,
                3,
                0
            )
            ->andReturn(1);


        $this->shouldWrite($destPairs + 2 * $destPairSize + 0x0, 0);

        $this->singleCall('_AsqRequestNjPvmPairs_12030')
            ->with($basedir, $pairs, 3)
            ->singleShouldReturn($destPairs)
            ->run();
    }

    public function test_handlesNjRequestFailure()
    {
        $basedir = $this->allocString('\\DIR');

        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $destPairSize = 8;
        $destPairCount = 3;
        $destPairs = $this->alloc($destPairSize * $destPairCount);

        $pairANjFilename = $this->allocString('');
        $pairAPvmFilename = $this->allocString('A.PVM');
        $this->initUint32($pairs + 0 * $pairSize + 0x0, $pairANjFilename);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, $pairAPvmFilename);

        $pairBNjFilename = $this->allocString('B.NJ');
        $pairBPvmFilename = $this->allocString('B.PVM');
        $this->initUint32($pairs + 1 * $pairSize + 0x0, $pairBNjFilename);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, $pairBPvmFilename);

        // Last pair has empty strings
        $emptyString = $this->allocString('');
        $this->initUint32($pairs + 2 * $pairSize + 0x0, $emptyString);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, $emptyString);

        $this->shouldCall('_syMalloc')
            ->with(3 * $destPairSize)
            ->andReturn($destPairs);

        // First pair
        $this->shouldCall('_AsqRequestNj_11492')
            ->with(
                $basedir,
                $pairANjFilename,
                0,
                $destPairs + 0 * $destPairSize + 0x4
            )
            ->andReturn(0);

        $this->shouldWrite($destPairs + 0 * $destPairSize + 0x4, -1);

        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with(
                $basedir,
                $pairAPvmFilename,
                $destPairs + 0 * $destPairSize + 0x0,
                3,
                0
            )
            ->andReturn(1);

        // Second pair
        $this->shouldCall('_AsqRequestNj_11492')
            ->with(
                $basedir,
                $pairBNjFilename,
                0,
                $destPairs + 1 * $destPairSize + 0x4
            )
            ->andReturn(1);

        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with(
                $basedir,
                $pairBPvmFilename,
                $destPairs + 1 * $destPairSize + 0x0,
                3,
                0
            )
            ->andReturn(1);


        $this->shouldWrite($destPairs + 2 * $destPairSize + 0x0, 0);

        $this->singleCall('_AsqRequestNjPvmPairs_12030')
            ->with($basedir, $pairs, 3)
            ->singleShouldReturn($destPairs)
            ->run();
    }

    public function test_handlesPvmRequestFailure()
    {
        $basedir = $this->allocString('\\DIR');

        $pairSize = 8;
        $pairCount = 3;
        $pairs = $this->alloc($pairSize * $pairCount);

        $destPairSize = 8;
        $destPairCount = 3;
        $destPairs = $this->alloc($destPairSize * $destPairCount);

        $pairANjFilename = $this->allocString('A.NJ');
        $pairAPvmFilename = $this->allocString('');
        $this->initUint32($pairs + 0 * $pairSize + 0x0, $pairANjFilename);
        $this->initUint32($pairs + 0 * $pairSize + 0x4, $pairAPvmFilename);

        $pairBNjFilename = $this->allocString('B.NJ');
        $pairBPvmFilename = $this->allocString('B.PVM');
        $this->initUint32($pairs + 1 * $pairSize + 0x0, $pairBNjFilename);
        $this->initUint32($pairs + 1 * $pairSize + 0x4, $pairBPvmFilename);

        // Last pair has empty strings
        $emptyString = $this->allocString('');
        $this->initUint32($pairs + 2 * $pairSize + 0x0, $emptyString);
        $this->initUint32($pairs + 2 * $pairSize + 0x4, $emptyString);

        $this->shouldCall('_syMalloc')
            ->with(3 * $destPairSize)
            ->andReturn($destPairs);

        // First pair
        $this->shouldCall('_AsqRequestNj_11492')
            ->with(
                $basedir,
                $pairANjFilename,
                0,
                $destPairs + 0 * $destPairSize + 0x4
            )
            ->andReturn(1);

        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with(
                $basedir,
                $pairAPvmFilename,
                $destPairs + 0 * $destPairSize + 0x0,
                3,
                0
            )
            ->andReturn(0);

        $this->shouldWrite($destPairs + 0 * $destPairSize + 0x0, -1);

        // Second pair
        $this->shouldCall('_AsqRequestNj_11492')
            ->with(
                $basedir,
                $pairBNjFilename,
                0,
                $destPairs + 1 * $destPairSize + 0x4
            )
            ->andReturn(1);

        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with(
                $basedir,
                $pairBPvmFilename,
                $destPairs + 1 * $destPairSize + 0x0,
                3,
                0
            )
            ->andReturn(1);


        $this->shouldWrite($destPairs + 2 * $destPairSize + 0x0, 0);

        $this->singleCall('_AsqRequestNjPvmPairs_12030')
            ->with($basedir, $pairs, 3)
            ->singleShouldReturn($destPairs)
            ->run();
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
