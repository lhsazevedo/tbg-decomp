<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_freeAndClearsMountedVmus()
    {
        $this->resolveSymbols();

        $bupInfos = [
            ['work' => 0],
            ['work' => 0xbabe0001],
            ['work' => 0xbabe0002],
            ['work' => 0],
            ['work' => 0xbabe0004],
            ['work' => 0],
            ['work' => 0xbabe0006],
            ['work' => 0],
        ];
        $bupAddresses = $this->initBackupInfos($bupInfos);

        $this->shouldCall('_syFree')->with(0xbabe0001);
        $this->shouldCall('_BupClearInfo_8c014c8a')->with(1);

        $this->shouldCall('_syFree')->with(0xbabe0002);
        $this->shouldCall('_BupClearInfo_8c014c8a')->with(2);

        $this->shouldCall('_syFree')->with(0xbabe0004);
        $this->shouldCall('_BupClearInfo_8c014c8a')->with(4);

        $this->shouldCall('_syFree')->with(0xbabe0006);
        $this->shouldCall('_BupClearInfo_8c014c8a')->with(6);

        $this->singleCall('_VmMenuFreeAndClear_8c019504')->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_gBupInfo_8c1bc4ac', 0x5c * 8);
        // Functions
        $this->setSize('_syFree', 0x4);
        $this->setSize('_BupClearInfo_8c014c8a', 0x4);
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }

    private function initBackupInfo($i, $work)
    {
        $address = $this->addressOf('_var_gBupInfo_8c1bc4ac') + $i * 0x5c;
        $this->initUint32Array($address, array_fill(0, 0x5c / 4, 0));
        $this->initUint32($address + 0x50, $work);
        return $address;
    }

    private function initBackupInfos(array $infos)
    {
        $addresses = [];
        foreach ($infos as $i => $info) {
            $addresses[] = $this->initBackupInfo($i, $info['work']);
        }
        return $addresses;
    }

    private function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
