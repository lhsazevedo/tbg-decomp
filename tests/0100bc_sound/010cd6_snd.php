<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_case0()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b1010_1010);

        $this->shouldCall('_ADXT_Stop')->with(0xcafe0000);
        $this->shouldCall('_ADXT_StartAfs')->with(0xcafe0000, 0, 42);
        $this->shouldWriteLongTo('_init_8c03bd80', 0b1010_1011);

        $this->singleCall('_snd_8c010cd6')
            ->with(0, 42)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_case1()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b1010_1010);

        $this->shouldCall('_ADXT_Stop')->with(0xcafe0001);
        $this->shouldCall('_ADXT_StartAfs')->with(0xcafe0001, 0, 42);
        $this->shouldWriteLongTo('_init_8c03bd80', 0b1011_1010);

        $this->singleCall('_snd_8c010cd6')
            ->with(1, 42)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_case2()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b1010_1010);

        $this->shouldCall('_ADXT_Stop')->with(0xcafe0001);
        $this->shouldCall('_ADXT_StartAfs')->with(0xcafe0001, 1, 42);
        $this->shouldWriteLongTo('_init_8c03bd80', 0b1011_1010);

        $this->singleCall('_snd_8c010cd6')
            ->with(2, 42)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_caseDefault()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b1010_1010);

        $this->singleCall('_snd_8c010cd6')
            ->with(3, 42)
            ->singleShouldReturn(0)
            ->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_ADXT_StartAfs', 4);
        $this->setSize('_ADXT_Stop', 4);

        // Basic inits
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 0x00, 0xcafe0000);
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 0x04, 0xcafe0001);
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 0x08, 0xcafe0002);
    }
};
