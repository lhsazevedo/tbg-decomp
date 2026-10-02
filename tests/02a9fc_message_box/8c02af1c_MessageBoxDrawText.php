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
        $this->setSize('_TxtDrawTextBox_8c0155e0', 4);

        $base = $this->addressOf('_var_messageTextBoxA_8c1bc404');
        $this->rellocate('_var_messageTextBoxB_8c1bc408', $base + 4);
        $this->rellocate('_var_messageTextBoxIndex_8c1bc40c', $base + 8);
    }

    public function test_draws_box0_when_active_index_is_0(): void
    {
        $this->resolveSymbols();

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), $box0);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), $box1);
        $this->initUint32($this->addressOf('_var_messageTextBoxIndex_8c1bc40c'), 0);

        $this->call('_MessageBoxDrawText_8c02af1c')
            ->with(7);

        $this->shouldCall('_TxtDrawTextBox_8c0155e0')
            ->with($box0, 7)
            ->andReturn(42);

        $this->shouldReturn(42);
    }

    public function test_draws_box1_when_active_index_is_1(): void
    {
        $this->resolveSymbols();

        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), $box0);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), $box1);
        $this->initUint32($this->addressOf('_var_messageTextBoxIndex_8c1bc40c'), 1);

        $this->call('_MessageBoxDrawText_8c02af1c')
            ->with(12);

        $this->shouldCall('_TxtDrawTextBox_8c0155e0')
            ->with($box1, 12)
            ->andReturn(0);

        $this->shouldReturn(0);
    }
};
