<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_mountsConnectedVms()
    {
        $this->resolveSymbols();

        $bupInfos = [
            ['connect' => 0,          'work' => 0],
            ['connect' => 0xcafe0001, 'work' => 0xbabe0001],
            ['connect' => 0xcafe0002, 'work' => 0],
            ['connect' => 0,          'work' => 0],
            ['connect' => 0xcafe0004, 'work' => 0xbabe0004],
            ['connect' => 0,          'work' => 0],
            ['connect' => 0xcafe0006, 'work' => 0],
            ['connect' => 0,          'work' => 0],
        ];
        $bupAddresses = $this->initBackupInfos($bupInfos);
        $createdTask = 0xffffe0;
        $createdState = 0xffffe4;

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bupAddresses[0]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn($bupAddresses[1]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(2)->andReturn($bupAddresses[2]);
        $this->shouldCall('_BupMount_8c014c00')->with(2);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(3)->andReturn($bupAddresses[3]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(4)->andReturn($bupAddresses[4]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(5)->andReturn($bupAddresses[5]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(6)->andReturn($bupAddresses[6]);
        $this->shouldCall('_BupMount_8c014c00')->with(6);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(7)->andReturn($bupAddresses[7]);

        $this->shouldCall('_TaskPush_8c014ae8')
        ->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_taskWaitForVmsReady_8c0193c8'),
            $createdTask,
            $createdState,
            0
        );
        $this->shouldWriteLongTo('_var_8c22606c', 1);

        $this->singleCall('_VmMenuMountVms_8c01940e')->run();
    }

    public function test_skipsMountedVms()
    {
        $this->resolveSymbols();

        $bupInfos = [
            ['connect' => 0xcafe0000, 'work' => 0xbabe0000],
            ['connect' => 0xcafe0001, 'work' => 0xbabe0001],
            ['connect' => 0xcafe0002, 'work' => 0xbabe0002],
            ['connect' => 0xcafe0003, 'work' => 0xbabe0003],
            ['connect' => 0xcafe0004, 'work' => 0xbabe0004],
            ['connect' => 0xcafe0005, 'work' => 0xbabe0005],
            ['connect' => 0xcafe0006, 'work' => 0xbabe0006],
            ['connect' => 0xcafe0007, 'work' => 0xbabe0007],
        ];
        $bupAddresses = $this->initBackupInfos($bupInfos);
        $createdTask = 0xffffe0;
        $createdState = 0xffffe4;

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bupAddresses[0]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn($bupAddresses[1]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(2)->andReturn($bupAddresses[2]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(3)->andReturn($bupAddresses[3]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(4)->andReturn($bupAddresses[4]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(5)->andReturn($bupAddresses[5]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(6)->andReturn($bupAddresses[6]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(7)->andReturn($bupAddresses[7]);

        $this->shouldCall('_TaskPush_8c014ae8')
        ->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_taskWaitForVmsReady_8c0193c8'),
            $createdTask,
            $createdState,
            0
        );
        $this->shouldWriteLongTo('_var_8c22606c', 1);

        $this->singleCall('_VmMenuMountVms_8c01940e')->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_BupGetInfo_8c014bba', 0x4);
        $this->setSize('_BupMount_8c014c00', 0x4);
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }

    private function initBackupInfo($connect, $work)
    {
        $address = $this->alloc(0x54);
        $this->initUint32Array($address, array_fill(0, 0x50 / 4, 0));
        $this->initUint32($address + 0x4c, $connect);
        $this->initUint32($address + 0x50, $work);
        return $address;
    }

    private function initBackupInfos(array $infos)
    {
        $addresses = [];
        foreach ($infos as $info) {
            $addresses[] = $this->initBackupInfo($info['connect'], $info['work']);
        }
        return $addresses;
    }

    private function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
