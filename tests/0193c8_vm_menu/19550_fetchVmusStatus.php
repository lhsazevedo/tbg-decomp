<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_fetchVmus()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $bupInfos = [
            [    // Not connected
                'ready'            => 1,
                'free_user_blocks' => 1,
                'connect'          => 0,
                'work'             => 1,
            ], [ // Not mounted
                'ready'            => 1,
                'free_user_blocks' => 1,
                'connect'          => 0xbabe0001,
                'work'             => 0,
            ], [ // Not ready
                'ready'            => 0,
                'free_user_blocks' => 1,
                'connect'          => 0xbabe0002,
                'work'             => 1,
            ], [ // Save data exists
                'ready'            => 1,
                'free_user_blocks' => 1,
                'connect'          => 0xbabe0003,
                'work'             => 1,
            ], [ // Save data exists 2
                'ready'            => 1,
                'free_user_blocks' => 4,
                'connect'          => 0xbabe0004,
                'work'             => 1,
            ], [ // Unformat
                'ready'            => 1,
                'free_user_blocks' => 4,
                'connect'          => 0xbabe0005,
                'work'             => 1,
            ], [ // Busy
                'ready'            => 1,
                'free_user_blocks' => 4,
                'connect'          => 0xbabe0005,
                'work'             => 1,
            ], [ // Saving is possible
                'ready'            => 1,
                'free_user_blocks' => 3,
                'connect'          => 0xbabe0005,
                'work'             => 1,
            ],
        ];
        $bupAddresses = $this->initBackupInfos($bupInfos);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bupAddresses[0]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 0 * 4, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn($bupAddresses[1]);
        $this->shouldCall('_BupMount_8c014c00')->with(1);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(2)->andReturn($bupAddresses[2]);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(3)->andReturn($bupAddresses[3]);
        $this->shouldCall('_buIsExistFile')->with(3, "TOKYOBUS.001")->andReturn(0); // BUD_ERR_OK
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 3 * 4, 5); // Save data exists

        $this->shouldCall('_BupGetInfo_8c014bba')->with(4)->andReturn($bupAddresses[4]);
        $this->shouldCall('_buIsExistFile')->with(4, "TOKYOBUS.001")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(4, "TOKYOBUS.002")->andReturn(0); // BUD_ERR_OK
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 4 * 4, 6); // Save data exists 2

        $this->shouldCall('_BupGetInfo_8c014bba')->with(5)->andReturn($bupAddresses[5]);
        $this->shouldCall('_buIsExistFile')->with(5, "TOKYOBUS.001")->andReturn(0xffffff03); // BUD_ERR_UNFORMAT
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 5 * 4, 1); // Not available
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 5 * 4, 1); // Not available

        $this->shouldCall('_BupGetInfo_8c014bba')->with(6)->andReturn($bupAddresses[6]);
        $this->shouldCall('_buIsExistFile')->with(6, "TOKYOBUS.001")->andReturn(0xffffffff); // BUD_ERR_BUSY
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 6 * 4, 1); // Not available

        $this->shouldCall('_BupGetInfo_8c014bba')->with(7)->andReturn($bupAddresses[7]);
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.001")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.002")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.003")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.004")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.005")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.006")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.007")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.008")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.009")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(7, "TOKYOBUS.010")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 7 * 4, 4); // Saving is possible

        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 8 * 4, 3);

        $this->singleCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->run();
    }

    public function test_fetchVmusNotEnoughSpace()
    {
        $this->resolveSymbols();

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,0,0,0,0,0,0,0,0,0]
        );

        $bupInfos = [
            [    // Not enough space
                'ready'            => 1,
                'free_user_blocks' => 2,
                'connect'          => 1,
                'work'             => 1,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ], [
                'ready'            => 0,
                'free_user_blocks' => 0,
                'connect'          => 0,
                'work'             => 0,
            ],
        ];
        $bupAddresses = $this->initBackupInfos($bupInfos);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bupAddresses[0]);
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.001")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.002")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.003")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.004")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.005")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.006")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.007")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.008")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.009")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldCall('_buIsExistFile')->with(0, "TOKYOBUS.010")->andReturn(0xffffff05); // BUD_ERR_FILE_NOT_FOUND
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 0 * 4, 2); // Not enough space

        $this->shouldCall('_BupGetInfo_8c014bba')->with(1)->andReturn($bupAddresses[1]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 1 * 4, 0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(2)->andReturn($bupAddresses[2]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 2 * 4, 0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(3)->andReturn($bupAddresses[3]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 3 * 4, 0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(4)->andReturn($bupAddresses[4]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 4 * 4, 0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(5)->andReturn($bupAddresses[5]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 5 * 4, 0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(6)->andReturn($bupAddresses[6]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 6 * 4, 0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(7)->andReturn($bupAddresses[7]);
        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 7 * 4, 0);

        $this->shouldWriteLong($this->addressOf('_var_vmuStatus_8c226048') + 8 * 4, 3);

        $this->singleCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
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

    private function initBackupInfos(array $infos)
    {
        $addresses = [];
        foreach ($infos as $i => $info) {
            $addresses[] = $this->initBackupInfo($i, $info);
        }
        return $addresses;
    }
};
