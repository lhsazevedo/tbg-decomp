<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_free()
    {
        // NJS_TEXNAMES
        $texnameSize = 0xc;
        $textures = $this->alloc($texnameSize * 3);
        $textures;

        $texnameA = $textures;
        $texAFile = $this->allocString('TEX_A');
        $this->initTexname($texnameA, $texAFile, 0, 0);
        
        $texnameB = $texnameA + $texnameSize;
        $texBFile = $this->allocString('TEX_B');
        $this->initTexname($texnameB, $texBFile, 0, 0);

        $texnameC = $texnameB + $texnameSize;
        $texCFile = $this->allocString('TEX_C');
        $this->initTexname($texnameC, $texCFile, 0, 0);

        // NJS_TEXLIST
        $texlist = $this->alloc(0x8);
        $this->initUint32($texlist + 0x0, $textures);
        $this->initUint32($texlist + 0x4, 3);  // nbTexture

        $this->shouldCall('_njReleaseTexture')->with($texlist);
        $this->shouldCall('_syFree')->with($texAFile);
        $this->shouldCall('_syFree')->with($textures);
        $this->shouldCall('_syFree')->with($texlist);

        $this->singleCall('_AsqReleaseAndFreeTexlist_8c011e3c')
            ->with($texlist)
            ->run();
    }

    private function initTexname($address, int $name, int $attr, int $addr)
    {
        $this->initUint32($address, $name);
        $this->initUint32($address + 4, $attr);
        $this->initUint32($address + 8, $addr);
    }
};
