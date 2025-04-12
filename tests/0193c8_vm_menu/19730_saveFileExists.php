<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_notConnected()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Not connected
        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 1,
            'connect'          => 0,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);

        $this->singleCall('_saveFileExists_19730')
            ->with($drive, $saveName)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_notMounted()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Not connected
        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 0,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);

        $this->singleCall('_saveFileExists_19730')
            ->with($drive, $saveName)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_notReady()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Not connected
        $bupInfoData = [
            'ready'            => 0,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);

        $this->singleCall('_saveFileExists_19730')
            ->with($drive, $saveName)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_saveExists()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Save exists
        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0); // BUD_ERR_OK

        $this->singleCall('_saveFileExists_19730')
            ->with($drive, $saveName)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_saveUnformat()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Save exists
        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0xffffff03); // BUD_ERR_UNFORMAT

        $this->singleCall('_saveFileExists_19730')
            ->with($drive, $saveName)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_saveBusy()
    {
        $this->resolveSymbols();

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Save exists
        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0xffffffff); // BUD_ERR_BUSY

        $this->singleCall('_saveFileExists_19730')
            ->with($drive, $saveName)
            ->singleShouldReturn(0)
            ->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_vmuStatus_8c226048', 4 * 9);

        // Functions
        $this->setSize('_BupGetInfo_8c014bba', 4);
        $this->setSize('_BupMount_8c014c00', 4);
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }

    private function initBackupInfo($i, $info)
    {
        $address = $this->addressOf('_gBupInfo_8c1bc4ac') + $i * 0x5c;
        $this->initUint32Array($address, array_fill(0, 0x5c / 4, 0));
        $this->initUint16($address + 0x0, $info['ready']);
        $this->initUint16($address + 0x3a, $info['free_user_blocks']);
        $this->initUint16($address + 0x4c, $info['connect']);
        $this->initUint16($address + 0x50, $info['work']);
        return $address;
    }

    // private function isAsmObject(): bool
    // {
    //     return str_ends_with($this->objectFile, '_src.obj');
    // }
};
