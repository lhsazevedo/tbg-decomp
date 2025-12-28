<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_test01()
    {
        $this->resolveSymbols();

        $GLYPH_WIDTH = 0x18;
        $GLYPH_HEIGHT = 0x20;
        $WIDTH = 0x240;
        $HEIGHT = 0x40;
        $GLYPH_COUNT = ($WIDTH / $GLYPH_WIDTH) * ($HEIGHT / $GLYPH_HEIGHT) + 0x28;

        $box = $this->alloc(0x3c);
        $glyphIndexes = $this->alloc($GLYPH_COUNT * 2);
        $this->shouldCall('_syMalloc')->with(0x3c)->andReturn($box);

        $this->shouldWriteLong($box + 0x00, 0x20);
        $this->shouldWriteLong($box + 0x04, 0x178);
        $this->shouldWriteFloat($box + 0x08, -2.0);
        $this->shouldWriteLong($box + 0x0c, $WIDTH);
        $this->shouldWriteLong($box + 0x10, $HEIGHT);
        $this->shouldWriteLong($box + 0x14, -1);
        $this->shouldWriteLong($box + 0x18, 69);
        $this->shouldWriteWord($box + 0x24, 0x0000);
        $this->shouldWriteWord($box + 0x26, 0xa94a);
        $this->shouldWriteWord($box + 0x28, 0xbdef);
        $this->shouldWriteWord($box + 0x2a, 0xc631);

        $this->shouldCall('_syMalloc')->with($GLYPH_COUNT * 2)->andReturn($glyphIndexes);
        $this->shouldWriteLong($box + 0x2c, $glyphIndexes);
        $this->shouldCall('_syMalloc')->with($HEIGHT * 4 / 32)->andReturn(0xbebacafe);
        $this->shouldWriteLong($box + 0x34, 0xbebacafe);
        $this->shouldWriteLong($box + 0x30, 42);
        
        for ($i = 0; $i < $GLYPH_COUNT; $i++) {
            $this->shouldWriteWord($glyphIndexes + $i * 2, 0xffff);
        }

        $this->shouldWriteLong($box + 0x38, 0);

        // Note: The game passes 0 instead of 42 and 69.
        $this->singleCall('_TxtCreateTextBox_8c0152fc')
            // TODO: -1 instead of 0xffffffff in the last argument
            ->with(0x20, 0x178, -2.0, $WIDTH, $HEIGHT, 42, 69, 0xffffffff)
            ->singleShouldReturn($box)
            ->run();
    }

    protected function resolveSymbols(): void
    {
        //$this->setSize('_var_fontResourceGroup_8c1bc794', 8);
        // Functions
        $this->setSize('_syMalloc', 4);
        $this->setSize('__divls', 4);
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
};
