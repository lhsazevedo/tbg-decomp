<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_code_8140()
    {
        // ¡
        // $this->testGlyph(0x4100, 0x117, movn: 2);

        // ¦
        // $this->testGlyph(0x81a6, 0x117, movn: 1);

        // "@"
        $this->testGlyph(0x8140, 0x000);
    }

    // A
    public function test_code_8141() { $this->testGlyph(0x8141, 0x001); }

    // ž
    public function test_code_819e() { $this->testGlyph(0x819e, 0x05d); }

    // Ÿ
    public function test_code_819f() { $this->testGlyph(0x819f, 0x05e); }

    // ¦
    public function test_code_81a6() { $this->testGlyph(0x81a6, 0x065); }

    // ¨
    public function test_code_81a8() { $this->testGlyph(0x81a8, 0x067); }

    // ¾
    public function test_code_81be() { $this->testGlyph(0x81be, 0x07d); }

    // ¾
    public function test_code_81de() { $this->testGlyph(0x81de, 0x09d); }

    // ô
    public function test_code_81f4() { $this->testGlyph(0x81f4, 0x0b3); }

    // ü
    public function test_code_81fc() { $this->testGlyph(0x81fc, 0x0bb); }

    // ‚`
    public function test_code_8260() { $this->testGlyph(0x8260, 0x076); }

    // ‚
    public function test_code_8281() { $this->testGlyph(0x8281, 0x090); }

    // ‚Ÿ
    public function test_code_828f() { $this->testGlyph(0x828f, 0x09e); }

    // ‚ñ
    public function test_code_82f1() { $this->testGlyph(0x82f1, 0x0fc); }

    // ƒ@
    public function test_code_8340() { $this->testGlyph(0x8340, 0x0fd); }

    // ƒ–
    public function test_code_8396() { $this->testGlyph(0x8396, 0x152); }

    // ƒŸ
    public function test_code_839f() { $this->testGlyph(0x839f, 0x153); }

    // ƒ¿
    public function test_code_83bf() { $this->testGlyph(0x83bf, 0x16b); }

    // „@
    public function test_code_8440() { $this->testGlyph(0x8440, 0x183); }

    // „p
    public function test_code_8470() { $this->testGlyph(0x8470, 0x1a4); }

    // „Ÿ
    public function test_code_849f() { $this->testGlyph(0x849f, 0x65, movn: 2); }

    // „¾
    public function test_code_84be() { $this->testGlyph(0x84be, 0x65, movn: 2); }

    // ˆŸ
    public function test_code_889f() { $this->testGlyph(0x889f, 0x1c5); }

    // ˆ®
    public function test_code_88ae() { $this->testGlyph(0x88ae, 0x1d4); }

    // ˆü
    public function test_code_88fc() { $this->testGlyph(0x88fc, 0x222); }

    // ‰@
    public function test_code_8940() { $this->testGlyph(0x8940, 0x223); }

    // ‰ž
    public function test_code_899e() { $this->testGlyph(0x899e, 0x280); }

    // ‰Ÿ
    public function test_code_899f() { $this->testGlyph(0x899f, 0x281); }

    // ‰ü
    public function test_code_89fc() { $this->testGlyph(0x89fc, 0x2de); }

    // Š@
    public function test_code_8a40() { $this->testGlyph(0x8a40, 0x2df); }

    // ŠO
    public function test_code_8a4f() { $this->testGlyph(0x8a4f, 0x2ee); }

    // Š_
    public function test_code_8a5f() { $this->testGlyph(0x8a5f, 0x2fe); }

    // Šo
    public function test_code_8a6f() { $this->testGlyph(0x8a6f, 0x30e); }

    // Š€
    public function test_code_8a80() { $this->testGlyph(0x8a80, 0x31e); }

    // Š
    public function test_code_8a90() { $this->testGlyph(0x8a90, 0x32e); }

    // Šž
    public function test_code_8a9e() { $this->testGlyph(0x8a9e, 0x33c); }

    // Šü
    public function test_code_8afc() { $this->testGlyph(0x8afc, 0x39a); }

    // ‹@
    public function test_code_8b40() { $this->testGlyph(0x8b40, 0x39b); }

    // ‹ž
    public function test_code_8b9e() { $this->testGlyph(0x8b9e, 0x3f8); }

    // ‹Ÿ
    public function test_code_8b9f() { $this->testGlyph(0x8b9f, 0x3f9); }

    // ‹ü
    public function test_code_8bfc() { $this->testGlyph(0x8bfc, 0x456); }

    // Œ@
    public function test_code_8c40() { $this->testGlyph(0x8c40, 0x457); }

    // Œž
    public function test_code_8c9e() { $this->testGlyph(0x8c9e, 0x4b4); }

    // ŒŸ
    public function test_code_8c9f() { $this->testGlyph(0x8c9f, 0x4b5); }

    // Œü
    public function test_code_8cfc() { $this->testGlyph(0x8cfc, 0x512); }

    // @
    public function test_code_8d40() { $this->testGlyph(0x8d40, 0x513); }

    // ž
    public function test_code_8d9e() { $this->testGlyph(0x8d9e, 0x570); }

    // Ÿ
    public function test_code_8d9f() { $this->testGlyph(0x8d9f, 0x571); }

    // ü
    public function test_code_8dfc() { $this->testGlyph(0x8dfc, 0x5ce); }


    // Ž@
    public function test_code_8e40() { $this->testGlyph(0x8e40, 0x5cf); }

    // Žž
    public function test_code_8e9e() { $this->testGlyph(0x8e9e, 0x62c); }

    // ŽŸ
    public function test_code_8e9f() { $this->testGlyph(0x8e9f, 0x62d); }

    // Žü
    public function test_code_8efc() { $this->testGlyph(0x8efc, 0x68a); }


    // @
    public function test_code_8f40() { $this->testGlyph(0x8f40, 0x68b); }

    // ž
    public function test_code_8f9e() { $this->testGlyph(0x8f9e, 0x6e8); }

    // Ÿ
    public function test_code_8f9f() { $this->testGlyph(0x8f9f, 0x6e9); }

    // ü
    public function test_code_8ffc() { $this->testGlyph(0x8ffc, 0x746); }


    // @
    public function test_code_9040() { $this->testGlyph(0x9040, 0x747); }

    // ž
    public function test_code_909e() { $this->testGlyph(0x909e, 0x7a4); }

    // Ÿ
    public function test_code_909f() { $this->testGlyph(0x909f, 0x7a5); }

    // ü
    public function test_code_90fc() { $this->testGlyph(0x90fc, 0x802); }


    // 
    public function test_code_9140() { $this->testGlyph(0x9140, 0x803); }

    // 
    public function test_code_919e() { $this->testGlyph(0x919e, 0x860); }

    // 
    public function test_code_919f() { $this->testGlyph(0x919f, 0x861); }

    // 
    public function test_code_91fc() { $this->testGlyph(0x91fc, 0x8be); }


    // 
    public function test_code_9240() { $this->testGlyph(0x9240, 0x8bf); }

    // 
    public function test_code_929e() { $this->testGlyph(0x929e, 0x91c); }

    // 
    public function test_code_929f() { $this->testGlyph(0x929f, 0x91d); }

    // 
    public function test_code_92fc() { $this->testGlyph(0x92fc, 0x97a); }


    // 
    public function test_code_9340() { $this->testGlyph(0x9340, 0x97b); }

    // 
    public function test_code_939e() { $this->testGlyph(0x939e, 0x9d8); }

    // 
    public function test_code_939f() { $this->testGlyph(0x939f, 0x9d9); }

    // 
    public function test_code_93fc() { $this->testGlyph(0x93fc, 0xa36); }


    // 
    public function test_code_9440() { $this->testGlyph(0x9440, 0xa37); }

    // 
    public function test_code_949e() { $this->testGlyph(0x949e, 0xa94); }

    // 
    public function test_code_949f() { $this->testGlyph(0x949f, 0xa95); }

    // 
    public function test_code_94fc() { $this->testGlyph(0x94fc, 0xaf2); }


    // 
    public function test_code_9540() { $this->testGlyph(0x9540, 0xaf3); }

    // 
    public function test_code_959e() { $this->testGlyph(0x959e, 0xb50); }

    // 
    public function test_code_959f() { $this->testGlyph(0x959f, 0xb51); }

    // 
    public function test_code_95fc() { $this->testGlyph(0x95fc, 0xbae); }


    // 
    public function test_code_9640() { $this->testGlyph(0x9640, 0xbaf); }

    // 
    public function test_code_969e() { $this->testGlyph(0x969e, 0xc0c); }

    // 
    public function test_code_969f() { $this->testGlyph(0x969f, 0xc0d); }

    // 
    public function test_code_96fc() { $this->testGlyph(0x96fc, 0xc6a); }


    // 
    public function test_code_9740() { $this->testGlyph(0x9740, 0xc6b); }

    // 
    public function test_code_979e() { $this->testGlyph(0x979e, 0xcc8); }

    // 
    public function test_code_979f() { $this->testGlyph(0x979f, 0xcc9); }

    // 
    public function test_code_97fc() { $this->testGlyph(0x97fc, 0xd26); }


    // 
    public function test_code_9840() { $this->testGlyph(0x9840, 0xd27); }

    // 
    public function test_code_989e() { $this->testGlyph(0x989e, 0xd84); }

    // 
    public function test_code_989f() { $this->testGlyph(0x989f, 0x65, movn: 2); }

    // 
    public function test_code_98fc() { $this->testGlyph(0x98fc, 0x65, movn: 2); }


    // 
    public function test_code_9940() { $this->testGlyph(0x9940, 0x65, movn: 2); }

    // 
    public function test_code_999e() { $this->testGlyph(0x999e, 0x65, movn: 2); }

    // 
    public function test_code_999f() { $this->testGlyph(0x999f, 0x65, movn: 2); }

    // 
    public function test_code_99fc() { $this->testGlyph(0x99fc, 0x65, movn: 2); }


    // 
    public function test_code_9a40() { $this->testGlyph(0x9a40, 0x65, movn: 2); }

    // 
    public function test_code_9a9e() { $this->testGlyph(0x9a9e, 0x65, movn: 2); }

    // 
    public function test_code_9a9f() { $this->testGlyph(0x9a9f, 0x65, movn: 2); }

    // 
    public function test_code_9afc() { $this->testGlyph(0x9afc, 0x65, movn: 2); }


    // 
    public function test_code_9b40() { $this->testGlyph(0x9b40, 0x65, movn: 2); }

    // 
    public function test_code_9b9e() { $this->testGlyph(0x9b9e, 0x65, movn: 2); }

    // 
    public function test_code_9b9f() { $this->testGlyph(0x9b9f, 0x65, movn: 2); }

    // 
    public function test_code_9bfc() { $this->testGlyph(0x9bfc, 0x65, movn: 2); }


    // 
    public function test_code_9c40() { $this->testGlyph(0x9c40, 0x65, movn: 2); }

    // 
    public function test_code_9c9e() { $this->testGlyph(0x9c9e, 0x65, movn: 2); }

    // 
    public function test_code_9c9f() { $this->testGlyph(0x9c9f, 0x65, movn: 2); }

    // 
    public function test_code_9cfc() { $this->testGlyph(0x9cfc, 0x65, movn: 2); }


    // 
    public function test_code_9d40() { $this->testGlyph(0x9d40, 0x65, movn: 2); }

    // 
    public function test_code_9d9e() { $this->testGlyph(0x9d9e, 0x65, movn: 2); }

    // 
    public function test_code_9d9f() { $this->testGlyph(0x9d9f, 0x65, movn: 2); }

    // 
    public function test_code_9dfc() { $this->testGlyph(0x9dfc, 0x65, movn: 2); }


    // 
    public function test_code_9e40() { $this->testGlyph(0x9e40, 0x65, movn: 2); }

    // 
    public function test_code_9e9e() { $this->testGlyph(0x9e9e, 0x65, movn: 2); }

    // 
    public function test_code_9e9f() { $this->testGlyph(0x9e9f, 0x65, movn: 2); }

    // 
    public function test_code_9efc() { $this->testGlyph(0x9efc, 0x65, movn: 2); }


    // 
    public function test_code_9f40() { $this->testGlyph(0x9f40, 0x65, movn: 2); }

    // 
    public function test_code_9f9e() { $this->testGlyph(0x9f9e, 0x65, movn: 2); }

    // 
    public function test_code_9f9f() { $this->testGlyph(0x9f9f, 0x65, movn: 2); }

    // 
    public function test_code_9ffc() { $this->testGlyph(0x9ffc, 0x65, movn: 2); }


    // 
    public function test_code_e040() { $this->testGlyph(0xe040, 0x65, movn: 2); }

    // 
    public function test_code_e09e() { $this->testGlyph(0xe09e, 0x65, movn: 2); }

    // 
    public function test_code_e09f() { $this->testGlyph(0xe09f, 0x65, movn: 2); }

    // 
    public function test_code_e0fc() { $this->testGlyph(0xe0fc, 0x65, movn: 2); }

    // 
    public function test_code_e0a1() { $this->testGlyph(0xe0a1, 0x65, movn: 2); }

    // public function test_1()
    // {
    //     // ƒZ
    //     $this->testGlyph(0x835a, 0x117);
    // }

    // public function test_2()
    // {
    //     // [
    //     $this->testGlyph(0x815b, 0x01b);
    // }

    // public function test_3()
    // {
    //     // ƒu
    //     $this->testGlyph(0x8375, 0x132);
    // }

    // public function test_5()
    // {
    //     // ‰Â
    //     $this->testGlyph(0x89c2, 0x2a4);
    // }
    // public function test_6()
    // {
    //     // ”\
    //     $this->testGlyph(0x945c, 0xa53);
    // }

    // public function test_7()
    // {
    //     // ‚Å
    //     $this->testGlyph(0x82c5, 0x0d0);
    // }

    // public function test_8()
    // {
    //     // ‚·
    //     $this->testGlyph(0x82b7, 0x0c2);
    // }

    protected function testGlyph(int $code, int $offset, int $movn = 1): void
    {
        $this->resolveSymbols();

        // TODO: Move implementation to Simulator
        // TODO: Handle calling conventions for expectations in Simulator
        $menuState = $this->addressOf('_menuState_8c1bc7a8');
        $mvn = function () use ($menuState) {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            // TODO: Really move to Simulator
            // TODO: Expect correct move sources
            // if (!$src->equals($menuState + 0x28)) {
            //     throw new \Exception('Unexpected move source ' . $this->registers[2]->readable());
            // }

            // if (!$dst->equals($menuState + 0x20)) {
            //     throw new \Exception('Unexpected move dest ' . $this->registers[1]->readable());
            // }

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $movn = $this->isAsmObject() ? $movn : 0;
        for ($i = 0; $i < $movn; $i++) {
            $this->shouldCall('__slow_mvn')->do($mvn);
        }

    $this->singleCall('_getGlyphIndex_8c015034')
        ->with($code)
        ->singleShouldReturn($offset)
        ->run();
    }

    protected function resolveSymbols(): void
    {
        $this->setSize('_const_8c035f24', 0x4e);
        $this->initUint16Array($this->addressOf('_const_8c035f24'), [
            0x0000, 0x005E, 0x006C, 0x00AA, 0x00FD, 0x0153, 0x0183, 0x01C5,
            0x0223, 0x0281, 0x02DF, 0x033D, 0x039B, 0x03F9, 0x0457, 0x04B5,
            0x0513, 0x0571, 0x05CF, 0x062D, 0x068B, 0x06E9, 0x0747, 0x07A5,
            0x0803, 0x0861, 0x08BF, 0x091D, 0x097B, 0x09D9, 0x0A37, 0x0A95,
            0x0AF3, 0x0B51, 0x0BAF, 0x0C0D, 0x0C6B, 0x0CC9, 0x0D27, 0x0000,
        ]);
        // Functions
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    protected function initUint16Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint16($address + $i * 2, $value);
        }
    }

    // private function initUint32Array(int $address, array $values): void
    // {
    //     foreach ($values as $i => $value) {
    //         $this->initUint32($address + $i * 4, $value);
    //     }
    // }
};
