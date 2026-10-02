<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // var_messageTextBoxA_8c1bc404/408/40c are consecutive words in the real binary
    // (accessed as (&var_messageTextBoxA_8c1bc404)[idx] elsewhere in this unit); force the
    // same adjacency here since the linker doesn't place separately
    // declared symbols next to each other by default.
    private function resolveSymbols(): void
    {
        $this->setSize('_TxtPrepareTextBoxLayout_8c01543a', 4);

        $base = $this->addressOf('_var_messageTextBoxA_8c1bc404');
        $this->rellocate('_var_messageTextBoxB_8c1bc408', $base + 4);
        $this->rellocate('_var_messageTextBoxIndex_8c1bc40c', $base + 8);
    }

    public function test_toggles_index_from_0_to_1_and_lays_out_box1(): void
    {
        $this->resolveSymbols();

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), $box0);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), $box1);
        $this->initUint32($this->addressOf('_var_messageTextBoxIndex_8c1bc40c'), 0);

        $string = $this->allocString('hello');

        $this->call('_MessageBoxSetText_8c02aefc')
            ->with($string);

        $this->shouldWriteLongTo('_var_messageTextBoxIndex_8c1bc40c', 1);

        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box1, $string)
            ->andReturn(5);

        $this->shouldReturn(5);
    }

    public function test_toggles_index_from_1_to_0_and_lays_out_box0(): void
    {
        $this->resolveSymbols();

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), $box0);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), $box1);
        $this->initUint32($this->addressOf('_var_messageTextBoxIndex_8c1bc40c'), 1);

        $string = $this->allocString('world');

        $this->call('_MessageBoxSetText_8c02aefc')
            ->with($string);

        $this->shouldWriteLongTo('_var_messageTextBoxIndex_8c1bc40c', 0);

        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box0, $string)
            ->andReturn(3);

        $this->shouldReturn(3);
    }
};
