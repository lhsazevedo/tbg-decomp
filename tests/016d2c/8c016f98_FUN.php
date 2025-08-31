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
    public function test_state_0_handles_missing_message()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $sound = $this->alloc(0x10);
        $this->initUint32($task + 0x18, $sound);
        $this->initUint32($sound, 3);

        $message = $this->allocString('');
        $state = $this->alloc(0x20);
        $ukn = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 0);
        $this->initUint32($state + 0x04, $ukn);
        $this->initUint32($ukn + 0x00, $message);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLongTo("_var_8c225fb4", 0);
        $this->shouldCall("_freeTask_8c014b66")->with($task);
    }

    public function test_state_0_plays_sound()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $sound = $this->alloc(0x10);
        $this->initUint32($task + 0x18, $sound);
        $this->initUint32($sound, 3);

        $message = $this->allocString('Hello, World!');
        $state = $this->alloc(0x20);
        $ukn = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 0);
        $this->initUint32($state + 0x04, $ukn);
        $this->initUint32($ukn + 0x00, $message);
        $this->initUint32($ukn + 0x04, 0xbebacafe);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldCall('_snd_8c010cd6')->with(2, 3);
        $this->shouldWriteLong($task + 0x18, $sound + 4);
        $this->shouldCall("_swapMessageBoxFor_8c02aefc")
            ->with("Hello, World!")
            ->andReturn(42);
        $this->shouldWriteLong($state + 0x08, 42);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x60, 0xbebacafe);
        $this->shouldWriteLong($state + 0x0c, 1);
        $this->shouldWriteLong($state + 0x10, 0);
        $this->shouldWriteLong($state + 0x00, 1);
        $this->shouldWriteLongTo('_var_8c225fb8', 1);
    }

    public function test_state_0_handles_null_sound_ptr()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x18, 0);

        $message = $this->allocString('Hello, World!');
        $state = $this->alloc(0x20);
        $ukn = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 0);
        $this->initUint32($state + 0x04, $ukn);
        $this->initUint32($ukn + 0x00, $message);
        $this->initUint32($ukn + 0x04, 0xbebacafe);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldCall("_swapMessageBoxFor_8c02aefc")
            ->with("Hello, World!")
            ->andReturn(42);
        $this->shouldWriteLong($state + 0x08, 42);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x60, 0xbebacafe);
        $this->shouldWriteLong($state + 0x0c, 1);
        $this->shouldWriteLong($state + 0x10, 0);
        $this->shouldWriteLong($state + 0x00, 1);
        $this->shouldWriteLongTo('_var_8c225fb8', 1);
    }

    public function test_state_0_skips_sound()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $sound = $this->alloc(0x10);
        $this->initUint32($task + 0x18, $sound);
        $this->initUint32($sound, 0);

        $message = $this->allocString('Hello, World!');
        $state = $this->alloc(0x20);
        $ukn = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 0);
        $this->initUint32($state + 0x04, $ukn);
        $this->initUint32($ukn + 0x00, $message);
        $this->initUint32($ukn + 0x04, 0xbebacafe);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldCall("_swapMessageBoxFor_8c02aefc")
            ->with("Hello, World!")
            ->andReturn(42);
        $this->shouldWriteLong($state + 0x08, 42);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x60, 0xbebacafe);
        $this->shouldWriteLong($state + 0x0c, 1);
        $this->shouldWriteLong($state + 0x10, 0);
        $this->shouldWriteLong($state + 0x00, 1);
        $this->shouldWriteLongTo('_var_8c225fb8', 1);
    }

    public function test_state_1_processes_normally()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 1);
        $this->initUint32($state + 0x10, 2);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x10, 3);
        $this->shouldWriteLong($state + 0x0c, 4);
        $this->shouldWriteLong($state + 0x00, 3);
        $this->shouldWriteLongTo('_var_8c225fb8', 4);
    }

    public function test_state_1_skips_when_field_0x10_less_than_3()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 1);
        $this->initUint32($state + 0x10, 1);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x10, 2);
        $this->shouldWriteLongTo('_var_8c225fb8', 3);
    }

    public function test_state_1_skips_when_field_0x0c_less_than_field_0x08()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 1);
        $this->initUint32($state + 0x10, 2);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x10, 3);
        $this->shouldWriteLong($state + 0x0c, 4);
        $this->shouldWriteLong($state + 0x10, 0);
        $this->shouldWriteLongTo('_var_8c225fb8', 4);
    }

    public function test_state_1_responds_to_a_press()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 1);
        $this->initUint32($state + 0x10, 2);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x10, 99);
        $this->shouldWriteLong($state + 0x00, 2);
        $this->shouldCall('_FUN_8c010ca6')->with(1);

        $this->shouldWriteLong($state + 0x10, 100);
        $this->shouldWriteLong($state + 0x0c, 4);
        $this->shouldWriteLong($state + 0x00, 3);
        $this->shouldWriteLongTo('_var_8c225fb8', 4);
    }

    public function test_state_1_responds_to_a_press_when_field_0x0c_less_than_field_0x08()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 1);
        $this->initUint32($state + 0x10, 2);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x10, 99);
        $this->shouldWriteLong($state + 0x00, 2);
        $this->shouldCall('_FUN_8c010ca6')->with(1);

        $this->shouldWriteLong($state + 0x10, 100);
        $this->shouldWriteLong($state + 0x0c, 4);
        $this->shouldWriteLong($state + 0x10, 0);
        $this->shouldWriteLongTo('_var_8c225fb8', 4);
    }

    public function test_state_2_waits_for_a_press()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 2);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0);
        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        // $this->shouldWriteLong($state + 0x0c, 5);
        $this->shouldWriteLong($state + 0x00, 1);
        $this->shouldWriteLongTo('_var_8c225fb8', 3);
    }

    public function test_state_2_waits_for_timer()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 2);
        $this->initUint32($state + 0x0c, 2);
        $this->initUint32($state + 0x08, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 4);
        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x0c, 4);
        // $this->shouldWriteLong($state + 0x00, 3);
        $this->shouldWriteLongTo('_var_8c225fb8', 4);
    }

    public function test_state_2_advances()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 2);
        $this->initUint32($state + 0x0c, 3);
        $this->initUint32($state + 0x08, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 4);
        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x0c, 5);
        $this->shouldWriteLong($state + 0x00, 3);
        $this->shouldWriteLongTo('_var_8c225fb8', 5);
    }

    public function test_state_3_advances_dialog_when_a_is_pressed()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);

        $state = $this->alloc(0x20);
        $this->initUint32($state + 0x00, 3);
        $this->initUint32($state + 0x04, 0xcafe0004);
        $this->initUint32($state + 0x0c, 3);
        // $this->initUint32($state + 0x08, 5);
        $this->initUint32($state + 0x14, 1);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4);

        $this->call('_dialogSequenceTask_8c016f98')->with($task, $state);

        $this->shouldWriteLong($state + 0x04, 0xcafe000c);
        $this->shouldWriteLong($state + 0x00, 0);
        $this->shouldWriteLong($state + 0x14, 0x1112);

        $this->shouldCall('_njCos')
            ->with(0x1112)
            ->andReturn(0.5);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8'),
            44,
            32.0,
            -16.0 + 8 * 0.5,
            -3.0
        );

        $this->shouldWriteLongTo('_var_8c225fb8', 3);
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 52 * 2);
        $this->setSize('_var_8c225fb4', 4);
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        // Functions
        $this->setSize('_snd_8c010cd6', 0x4);
        $this->setSize('_freeTask_8c014b66', 0x4);
    }
};

