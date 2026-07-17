<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_test01()
    {
        $this->resolveSymbols();

        $titleDat = $this->allocBytes(file_get_contents(__DIR__ . '/data/TITLE.DAT'));
        $res = $this->alloc(0xc);
        $this->initUint32($res + 0x00, 0xcafe0001);
        $this->initUint32($res + 0x04, 0xcafe0002);
        $this->initUint32($res + 0x08, $titleDat);

        $spriteLocal = $this->isAsmObject() ? 0xffffb8 : 0xffffb8;

        $this->shouldWriteLong($spriteLocal + 6 * 4, 0xcafe0001);
        $this->shouldWriteLong($spriteLocal + 7 * 4, 0xcafe0002);
        $this->shouldWriteLong($spriteLocal + 5 * 4, 0);
        $this->shouldWriteFloat($spriteLocal + 3 * 4, 1.0);
        $this->shouldWriteFloat($spriteLocal + 4 * 4, 1.0);

        $this->shouldWriteFloat($spriteLocal + 0, 158.0);
        $this->shouldWriteFloat($spriteLocal + 4, 255.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 52, 4.5, 32);

        $this->singleCall('_TxtDrawSprite_8c014f54')
            ->with($res, 2, 42.0, 69.0, 4.5)
            ->run();
    }

    public function test_drawFortyFiveLogo()
    {
        $this->resolveSymbols();

        $titleDat = $this->allocBytes(file_get_contents(__DIR__ . '/data/TITLE.DAT'));
        $res = $this->alloc(0xc);
        $this->initUint32($res + 0x00, 0xcafe0001);
        $this->initUint32($res + 0x04, 0xcafe0002);
        $this->initUint32($res + 0x08, $titleDat);

        $spriteLocal = $this->isAsmObject() ? 0xffffb8 : 0xffffb8;

        $this->shouldWriteLong($spriteLocal + 6 * 4, 0xcafe0001);
        $this->shouldWriteLong($spriteLocal + 7 * 4, 0xcafe0002);
        $this->shouldWriteLong($spriteLocal + 5 * 4, 0);
        $this->shouldWriteFloat($spriteLocal + 3 * 4, 1.0);
        $this->shouldWriteFloat($spriteLocal + 4 * 4, 1.0);

        $this->shouldWriteFloat($spriteLocal + 0, 208.0);
        $this->shouldWriteFloat($spriteLocal + 4, 128.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 1, -5.0, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 190.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 108, -4.9999, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 222.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 109, -4.9998, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 254.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 110, -4.9997, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 286.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 111, -4.9996, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 318.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 112, -4.9995, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 350.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 113, -4.9994, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 382.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 114, -4.9993, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 414.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 115, -4.9992, 32);

        $this->shouldWriteFloat($spriteLocal + 0, 446.0);
        $this->shouldWriteFloat($spriteLocal + 4, 311.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 116, -4.9991, 32);

        // TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);
        $this->singleCall('_TxtDrawSprite_8c014f54')
            ->with($res, 0, 0.0, 0.0, -5.0)
            ->run();
    }

    public function test_drawBus()
    {
        $this->resolveSymbols();

        $titleDat = $this->allocBytes(file_get_contents(__DIR__ . '/data/TITLE.DAT'));
        $res = $this->alloc(0xc);
        $this->initUint32($res + 0x00, 0xcafe0001);
        $this->initUint32($res + 0x04, 0xcafe0002);
        $this->initUint32($res + 0x08, $titleDat);

        $spriteLocal = $this->isAsmObject() ? 0xffffb8 : 0xffffb8;

        $this->shouldWriteLong($spriteLocal + 6 * 4, 0xcafe0001);
        $this->shouldWriteLong($spriteLocal + 7 * 4, 0xcafe0002);
        $this->shouldWriteLong($spriteLocal + 5 * 4, 0);
        $this->shouldWriteFloat($spriteLocal + 3 * 4, 1.0);
        $this->shouldWriteFloat($spriteLocal + 4 * 4, 1.0);

        $this->shouldWriteFloat($spriteLocal + 0, 180.0);
        $this->shouldWriteFloat($spriteLocal + 4, 151.0);
        $this->shouldCall('_njDrawSprite2D')
            ->with($spriteLocal, 11, -4.0, 32);

        // TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
        $this->singleCall('_TxtDrawSprite_8c014f54')
            ->with($res, 1, 180.0, 0.0, -4.0)
            ->run();
    }

    // // TODO: Resource needs to be offsetted.
    // public function test_drawCharacter()
    // {
    //     $this->resolveSymbols();

    //     $fontDat = $this->allocBytes(file_get_contents(__DIR__ . '/data/BUS_FONT.FFF'));
    //     $res = $this->alloc(0xc);
    //     $this->initUint32($res + 0x00, 0xcafe0001);
    //     $this->initUint32($res + 0x04, 0xcafe0002);
    //     $this->initUint32($res + 0x08, $fontDat);

    //     $spriteLocal = 0xffffb8;

    //     $this->shouldWriteLong($spriteLocal + 6 * 4, 0xcafe0001);
    //     $this->shouldWriteLong($spriteLocal + 7 * 4, 0xcafe0002);
    //     $this->shouldWriteLong($spriteLocal + 5 * 4, 0);
    //     $this->shouldWriteFloat($spriteLocal + 3 * 4, 1.0);
    //     $this->shouldWriteFloat($spriteLocal + 4 * 4, 1.0);

    //     $this->shouldWriteFloat($spriteLocal + 0, 180.0);
    //     $this->shouldWriteFloat($spriteLocal + 4, 90.0);
    //     $this->shouldCall('_njDrawSprite2D')
    //         ->with($spriteLocal, 0, -4.0, 32);

    //     // TxtDrawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
    //     $this->singleCall('_TxtDrawSprite_8c014f54')
    //         ->with($res, 2000, 180.0, 90.0, -4.0)
    //         ->run();
    // }

    /**
     * Allocates a string and returns its address.
     */
    protected function allocBytes(string $str): int
    {
        $address = $this->alloc(strlen($str));
        foreach (str_split($str) as $i => $char) {
            $this->initUint8($address + $i, ord($char));
        }
        return $address;
    }

    protected function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_njDrawSprite2D', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
