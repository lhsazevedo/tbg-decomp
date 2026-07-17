<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_unmountsConnectedVms()
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

        $task = 0xbebacafe;

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bupAddresses[0]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn($bupAddresses[1]);
        $this->shouldCall('_buStat')->with(1)->andReturn(0); // BUD_STAT_READY
        $this->shouldCall('_BupUnmount_8c014c46')->with(1);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(2)->andReturn($bupAddresses[2]);
        $this->shouldCall('_buStat')->with(2)->andReturn(0); // BUD_STAT_READY
        $this->shouldCall('_BupGetInfo_8c014bba')->with(3)->andReturn($bupAddresses[3]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(4)->andReturn($bupAddresses[4]);
        $this->shouldCall('_buStat')->with(4)->andReturn(0); // BUD_STAT_READY
        $this->shouldCall('_BupUnmount_8c014c46')->with(4);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(5)->andReturn($bupAddresses[5]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(6)->andReturn($bupAddresses[6]);
        $this->shouldCall('_buStat')->with(6)->andReturn(0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(7)->andReturn($bupAddresses[7]);

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldWriteLongTo('_var_8c22606c', 0);

        $this->singleCall('_taskUnmountVms_8c01946a')->with($task)->run();
    }

    public function test_doNotFreeWhenThereAreBusyVmus()
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

        $task = 0xbebacafe;

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bupAddresses[0]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn($bupAddresses[1]);
        $this->shouldCall('_buStat')->with(1)->andReturn(0); // BUD_STAT_READY
        $this->shouldCall('_BupUnmount_8c014c46')->with(1);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(2)->andReturn($bupAddresses[2]);
        $this->shouldCall('_buStat')->with(2)->andReturn(-1); // BUD_STAT_BUSY
        $this->shouldCall('_BupGetInfo_8c014bba')->with(3)->andReturn($bupAddresses[3]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(4)->andReturn($bupAddresses[4]);
        $this->shouldCall('_buStat')->with(4)->andReturn(0); // BUD_STAT_READY
        $this->shouldCall('_BupUnmount_8c014c46')->with(4);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(5)->andReturn($bupAddresses[5]);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(6)->andReturn($bupAddresses[6]);
        $this->shouldCall('_buStat')->with(6)->andReturn(0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(7)->andReturn($bupAddresses[7]);

        $this->singleCall('_taskUnmountVms_8c01946a')->with($task)->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_BupGetInfo_8c014bba', 0x4);
        $this->setSize('_BupUnmount_8c014c46', 0x4);
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
};
