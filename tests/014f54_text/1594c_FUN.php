<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_basic()
    {
        $var_demoBuf_8c1ba3c4 = $this->alloc(5 * 4);
        $this->initUint32($var_demoBuf_8c1ba3c4 + 0 * 4, 0xcafe0000);
        $this->initUint32($var_demoBuf_8c1ba3c4 + 1 * 4, 0xcafe0001);
        $this->initUint32($var_demoBuf_8c1ba3c4 + 2 * 4, 0xcafe0002);
        $this->initUint32($var_demoBuf_8c1ba3c4 + 3 * 4, 0xcafe0003);
        $this->initUint32($this->addressOf('_var_demoBuf_8c1ba3c4'), $var_demoBuf_8c1ba3c4);

        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(1);

        $this->shouldWriteTo('_var_currentCourse_8c1bb868', 0xcafe0001);
        $this->shouldWriteTo('_var_inputMapSel_8c1bb8c8', 0xcafe0002);
        $this->shouldWriteTo('_var_seed_8c157a64', 0xcafe0003);

        $local1 = $this->isAsmObject() ? 0xffffe4 : 0xffffec;

        $this->shouldWriteLong($local1, $this->addressOf('_var_demoBuffer_8c1bc828'));

        $this->shouldCall('_FUN_8c02f320');
        $this->shouldCall('_FUN_readDemo_8c02fa14')
            ->with(
                $var_demoBuf_8c1ba3c4 + 4 * 4,
                $local1,
                0xcafe0000
            );
        $this->shouldCall('_syFree')->with($var_demoBuf_8c1ba3c4);
        $this->shouldWriteLongTo('_var_demoBuf_8c1ba3c4', -1);
        $this->shouldCall('_TaskFree_8c014b66')->with(0xbeba1337);
        $this->shouldCall('_FUN_8c01328c');

        $this->singleCall('_FUN_8c01594c')
            ->with(0xbeba1337)
            ->run();
    }

    public function test_skip()
    {
        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(0);

        $this->singleCall('_FUN_8c01594c')
            ->with()
            ->run();
    }

    protected function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_RouteLoadIsPvmReady_8c01432a', 4);
        // $this->setSize('_strlen', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
