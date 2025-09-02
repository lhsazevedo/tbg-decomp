<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_1()
    {
        $this->resolveSymbols();

        $text1 = $this->allocString('Hey there!');
        $dialogs1 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs1 + 0x00, $text1);
        $this->initUint32($dialogs1 + 0x04, 1);

        $text2 = $this->allocString('Hey there!');
        $dialogs2 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs2 + 0x00, $text2);
        $this->initUint32($dialogs2 + 0x04, 42);

        $this->setSize('_init_dialogSequences_8c044c08', 0x8);
        $this->initUint32($this->addressOf('_init_dialogSequences_8c044c08') + 0x00, $dialogs1);
        $this->initUint32($this->addressOf('_init_dialogSequences_8c044c08') + 0x04, $dialogs2);

        $this->call('_FUN_swapDialogMessageBox_8c017108')->with(1);

        $this->shouldCall('_swapMessageBoxFor_8c02aefc')
            ->with($text2)
            ->andReturn(42);

        $this->shouldWrite($this->addressOf('_var_8c225fb8'), 42);
    }

    public function resolveSymbols()
    {

    }
};
