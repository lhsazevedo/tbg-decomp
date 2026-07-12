<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_notConnected()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

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
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 0);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_notMounted()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Not mounted
        $bupInfoData = [
            'ready'            => 0,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 0,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_BupMount_8c014c00')->with($drive);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 0);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_notReady()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Not ready
        $bupInfoData = [
            'ready'            => 0,
            'free_user_blocks' => 1,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 0);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_saveExists()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

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
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 5);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_saveExists2()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        // Save exists 2
        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 3,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0); // BUD_ERR_OK
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 6);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_saveUnformat()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 4,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0xffffff03); // BUD_ERR_UNFORMAT
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 1);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 1);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_saveBusy()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 4,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0xffffffff); // BUD_ERR_BUSY
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 1);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_saveIsPossible()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 4,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 4);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
            ->run();
    }

    public function test_notEnoughSpace()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $drive = random_int(0, 7);
        $saveName = $this->allocString('TOKYOBUS.003');

        $bupInfoData = [
            'ready'            => 1,
            'free_user_blocks' => 2,
            'connect'          => 1,
            'work'             => 1,
        ];
        $bupInfo = $this->initBackupInfo(1, $bupInfoData);

        $this->shouldCall('_BupGetInfo_8c014bba')->with($drive)->andReturn($bupInfo);
        $this->shouldCall('_buIsExistFile')->with($drive)->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + $drive * 4, 2);

        $this->singleCall('_VmMenuUpdateVmuStatus_8c01967c')
            ->with($drive, $saveName, 3)
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
        $address = $this->addressOf('_var_gBupInfo_8c1bc4ac') + $i * 0x5c;
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
