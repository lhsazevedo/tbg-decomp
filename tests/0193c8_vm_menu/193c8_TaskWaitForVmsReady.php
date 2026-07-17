<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_freeWhenAllReady()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x04);

        $bupInfos = $this->initBackupInfos([
            ['connect' => 0,          'ready' => 0],
            ['connect' => 0xcafe0001, 'ready' => 1],
            ['connect' => 0,          'ready' => 0],
            ['connect' => 0,          'ready' => 0],
            ['connect' => 0,          'ready' => 0],
            ['connect' => 0,          'ready' => 0],
            ['connect' => 0xcafe0006, 'ready' => 1],
            ['connect' => 0,          'ready' => 0],
        ]);

        foreach ($bupInfos as $i => $bupInfo) {
            $this->shouldCall('_BupGetInfo_8c014bba')->with($i)->andReturn($bupInfo);
        }

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldWriteLongTo('_var_8c22606c', 0);

        $this->singleCall('_taskWaitForVmsReady_8c0193c8')->with($task, 0)->run();
    }

    public function test_freeWhenNoneConnected()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x04);

        $bupInfos = $this->initBackupInfos([
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
        ]);

        foreach ($bupInfos as $i => $bupInfo) {
            $this->shouldCall('_BupGetInfo_8c014bba')->with($i)->andReturn($bupInfo);
        }

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldWriteLongTo('_var_8c22606c', 0);

        $this->singleCall('_taskWaitForVmsReady_8c0193c8')->with($task, 0)->run();
    }

    public function test_waitsForReady()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x04);

        $bupInfos = $this->initBackupInfos([
            ['connect' => 0, 'ready' => 0],
            ['connect' => 1, 'ready' => 1],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
            ['connect' => 1, 'ready' => 0],
            ['connect' => 0, 'ready' => 0],
        ]);

        foreach (array_slice($bupInfos, 0, 7) as $i => $bupInfo) {
            $this->shouldCall('_BupGetInfo_8c014bba')->with($i)->andReturn($bupInfo);
        }

        $this->singleCall('_taskWaitForVmsReady_8c0193c8')->with($task, 0)->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_BupGetInfo_8c014bba', 0x4);
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }

    private function initBackupInfo($connect, $ready)
    {
        $address = $this->alloc(0x50);
        $this->initUint32Array($address, array_fill(0, 0x50 / 4, 0));
        $this->initUint32($address + 0x4c, $connect);
        $this->initUint32($address + 0x0, $ready);
        return $address;
    }

    private function initBackupInfos(array $infos)
    {
        $addresses = [];
        foreach ($infos as $info) {
            $addresses[] = $this->initBackupInfo($info['connect'], $info['ready']);
        }
        return $addresses;
    }

    private function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
