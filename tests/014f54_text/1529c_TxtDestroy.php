<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_test01()
    {
        $this->resolveSymbols();

        // TODO: calloc
        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, -18));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);
        $this->initUint16($var_8c1bc7a0 + 0x000 * 2, -19);
        $this->initUint16($var_8c1bc7a0 + 0x001 * 2, -20);
        $this->initUint16($var_8c1bc7a0 + 0x199 * 2, -20);
        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 8);
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), 0xcafe0000);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0001);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x001 * 8);
        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x199 * 8);

        $this->shouldCall('_syFree')->with($var_glyphTexlists_8c1bc790);
        $this->shouldCall('_syFree')->with(0xcafe0000);
        $this->shouldCall('_syFree')->with(0xcafe0001);
        $this->shouldCall('_syFree')->with($var_8c1bc7a0);

        $this->singleCall('_TxtDestroy_8c01529c')->run();
    }

    // In play the table holds the glyph index (0..0x1ff) for a slot in use and 0xffff for a
    // free one. The original compares the zero-extended value, so every slot in use is released.
    public function test_inUseSlotsAreReleased()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);
        $this->initUint16($var_8c1bc7a0 + 0x000 * 2, 0x000);
        $this->initUint16($var_8c1bc7a0 + 0x010 * 2, 0x010);
        $this->initUint16($var_8c1bc7a0 + 0x1ff * 2, 0x1ff);
        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 8);
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), 0xcafe0000);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0001);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x000 * 8);
        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x010 * 8);
        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0x1ff * 8);

        $this->shouldCall('_syFree')->with($var_glyphTexlists_8c1bc790);
        $this->shouldCall('_syFree')->with(0xcafe0000);
        $this->shouldCall('_syFree')->with(0xcafe0001);
        $this->shouldCall('_syFree')->with($var_8c1bc7a0);

        $this->singleCall('_TxtDestroy_8c01529c')->run();
    }

    protected function resolveSymbols(): void
    {
        //$this->setSize('_var_fontResourceGroup_8c1bc794', 8);
        // Functions
        // $this->setSize('__divls', 4);
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
