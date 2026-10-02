<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    // Spawned task state offsets (word index * 4).
    const ST_00 = 0x00;
    const ST_04 = 0x04;

    public function test_starts_message_box_task_and_textbox(): void
    {
        $handle = $this->alloc(4);

        $this->initUint32($this->addressOf('_var_messageTextDat_8c228518'), $handle);

        $groupTable = $this->alloc(8);
        $groupEntry = 0x11223344;
        $this->initUint32($groupTable + 0x04, $groupEntry);
        $this->initUint32($this->addressOf('_var_eventSlides_8c228480'), $groupTable);
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), 1);

        $this->initUint32($this->addressOf('_var_eventCount_8c1bb8e8'), 5);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x1c);

        $this->call('_MessageBoxStart_8c02ad8c');

        $this->shouldCall('_relocateMessageText_8c02a9fc')->with($handle);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_slideshowTask_8c02ab7a'),
            )
            ->do(function () use ($task, $state) {
                // Verify the stack-passed alloc_size (5th arg) directly,
                // since with() only checks the two leading register args.
                $sizeArg = $this->memory->readUInt32($this->registers[15]->value);
                if (!$sizeArg->equals(0x1c)) {
                    throw new \Exception(
                        "Unexpected TaskSpawn alloc_size $sizeArg, expecting 0x1c"
                    );
                }
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);

        $this->shouldWriteLong($state + 0x10, $groupEntry);

        $this->shouldCall('_MessageBoxOpenTextbox_8c02ae3e')->with(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);

        $this->shouldWriteLong($state + self::ST_00, 0);
        $this->shouldWriteLongTo('_var_messageBoxActive_8c22847c', 1);
        $this->shouldWriteLongTo('_var_eventCount_8c1bb8e8', 6);
    }
};
