<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_it_unpacks835a()
    {
        $this->resolveSymbols();

        $busFont = $this->allocBytes(file_get_contents(__DIR__ . '/data/BUS_FONT.FFF'));
        $texture = $this->alloc(0x400 * 2); // 2048 bytes
        $colors = $this->alloc(4 * 2);
        $this->initUint16Array($colors, [
            0xcaf0,
            0xcaf1,
            0xcaf2,
            0xcaf3,
        ]);

        $srcLocal = $this->isAsmObject() ? 0xfff4dc : 0xfff4e4;

        // TODO: Move implementation to Simulator
        // TODO: Handle calling conventions for expectations in Simulator
        $mvn = function () {
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

        if (!$this->isAsmObject()) {
            $this->shouldCall('__slow_mvn')->do($mvn);
            $this->shouldCall('__slow_mvn')->do($mvn);
        }

        // ƒZ
        $this->shouldCall('_getGlyphIndex_8c015034')
            ->with(0x835a)
            ->andReturn(0x117);

        if ($this->isAsmObject()) {
            for ($i = 0; $i < 0x400; $i++) {
                $this->shouldWriteWord($texture + $i * 2, 0);
                $this->shouldWriteWord($srcLocal + $i * 2, 0);
            }
        }

        if ($this->isAsmObject()) {
            $this->shouldCall('__divls');
        }

        $writes = [
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 2, 3, 3, 3, 3, 2, 2, 1, 2, 3, 3, 2, 0, 0,
            0, 0, 0, 0, 1, 3, 3, 2, 2, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 0, 0,
            0, 0, 0, 0, 1, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0,
            0, 0, 0, 1, 2, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0,
            0, 0, 2, 3, 2, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 1, 0,
            0, 0, 2, 1, 2, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 1, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 0, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 1, 0, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 2, 1, 1, 2, 2, 2, 2, 3, 3, 2, 2, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        ];

        $writes = array_map(fn ($v) => [0xcaf0, 0xcaf1, 0xcaf2, 0xcaf3][$v], $writes);

        $j = 0;
        for ($i = 0; $i < 0x400; $i++) {
            if (($i % 0x20) < 0x18) {
                $this->shouldWriteWord($srcLocal + $i * 2, $writes[$j]);
                $j++;
            }
        }

        $this->shouldCall('_njTwiddledTexture')
            ->with($texture, $srcLocal, 0x20);

        $this->singleCall('_unpackGlyph_8c015110')
            ->with(0x835a, $colors, $busFont, $texture)
            ->run();
    }

    public function test_it_unpacks889f()
    {
        $this->resolveSymbols();

        $busFont = $this->allocBytes(file_get_contents(__DIR__ . '/data/BUS_FONT.FFF'));
        $texture = $this->alloc(0x400 * 2); // 2048 bytes
        $colors = $this->alloc(4 * 2);
        $this->initUint16Array($colors, [
            0xcaf0,
            0xcaf1,
            0xcaf2,
            0xcaf3,
        ]);

        $srcLocal = $this->isAsmObject() ? 0xfff4dc : 0xfff4e4;

        // TODO: Move implementation to Simulator
        // TODO: Handle calling conventions for expectations in Simulator
        $mvn = function () {
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

        if (!$this->isAsmObject()) {
            $this->shouldCall('__slow_mvn')->do($mvn);
            $this->shouldCall('__slow_mvn')->do($mvn);
        }

        // ˆŸ
        $this->shouldCall('_getGlyphIndex_8c015034')
            ->with(0x889f)
            ->andReturn(0x1c5);


        if ($this->isAsmObject()) {
            for ($i = 0; $i < 0x400; $i++) {
                $this->shouldWriteWord($texture + $i * 2, 0);
                $this->shouldWriteWord($srcLocal + $i * 2, 0);
            }
        }

        if ($this->isAsmObject()) {
            $this->shouldCall('__divls');
        }

        $writes = [
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0,
            0, 0, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 1, 0, 0, 1, 3, 3, 2, 1, 1, 3, 3, 3, 2, 1, 1, 0, 0, 0, 0,
            0, 0, 0, 2, 3, 3, 1, 0, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 0, 0,
            0, 0, 0, 2, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 3, 3, 3, 0, 0,
            0, 0, 0, 2, 3, 3, 2, 1, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 1, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 1, 3, 3, 2, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 1, 3, 3, 2, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 1, 3, 3, 2, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 1, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 1, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 2, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 2, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 2, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 2, 3, 3, 1, 0,
            0, 0, 0, 2, 3, 3, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 2, 3, 3, 0, 0,
            0, 0, 0, 2, 3, 3, 2, 2, 3, 3, 3, 3, 2, 2, 3, 3, 3, 2, 2, 3, 3, 3, 2, 0,
            0, 0, 0, 2, 3, 3, 2, 2, 2, 3, 3, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 1, 0,
            0, 0, 0, 2, 3, 3, 1, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 2, 2, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 2, 3, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1,
            0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        ];

        $writes = array_map(fn ($v) => [0xcaf0, 0xcaf1, 0xcaf2, 0xcaf3][$v], $writes);

        $j = 0;
        for ($i = 0; $i < 0x400; $i++) {
            if (($i % 0x20) < 0x18) {
                $this->shouldWriteWord($srcLocal + $i * 2, $writes[$j]);
                $j++;
            }
        }

        $this->shouldCall('_njTwiddledTexture')
            ->with($texture, $srcLocal, 0x20);

        $this->singleCall('_unpackGlyph_8c015110')
            ->with(0x889f, $colors, $busFont, $texture)
            ->run();
    }

    /**
     * Allocates a string and returns its address.
     */
    protected function allocBytes(string $str): int
    {
        $address = $this->alloc(strlen($str));
        for ($i = 0; $i < strlen($str); $i++) {
            $this->initUint8($address + $i, ord($str[$i]));
        }
        return $address;
    }

    private function initUint16Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint16($address + $i * 2, $value);
        }
    }

    protected function resolveSymbols(): void
    {
        // Functions
        $this->setSize('__divls', 4);
        $this->setSize('_njTwiddledTexture', 4);

        // offset = glyphIndex * PACKED_GLYPH_SIZE is computed as
        // (glyphIndex * 3 << 8) / 4, i.e. glyphIndex * 0x300 divided by 4.
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
