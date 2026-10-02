<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** The task state, with its linked traffic-signal task at [0x34]. */
    private function makeState(int $linked, int $id = 0, int $counter = 0): int
    {
        $state = $this->alloc(0xd8);
        $this->initUint32($state + 0x00, $id);
        $this->initUint32($state + 0x04, $counter);
        $this->initUint32($state + 0xd0, $linked); // state[0x34]
        return $state;
    }

    /** The linked traffic-signal task's state. */
    private function makeLinked(int $frame, int $threshold, int $markA = 0, int $markB = 0): int
    {
        $linked = $this->alloc(0xd0);
        $this->initUint32($linked + 0x04, $threshold); // linked[1]
        $this->initUint32($linked + 0x0c, $frame); // linked[3]
        $this->initUint32($linked + 0xc8, $markA); // linked[0x32]
        $this->initUint32($linked + 0xcc, $markB); // linked[0x33]
        return $linked;
    }

    private function slots(int $count = 4): int
    {
        $slots = $this->alloc($count * 4);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $slots);
        return $slots;
    }

    public function test_open_frame_when_counter_reached_threshold()
    {
        $linked = $this->makeLinked(frame: 0, threshold: 5);
        $state = $this->makeState($linked, id: 1, counter: 5);
        $slots = $this->slots();

        $this->call('_attachmentTask_8c02833c')->with(0, $state);

        $this->shouldWriteLong($state + 0x0c, 1); // state[3]
        $this->shouldWriteLong($state + 0xc8, 1); // state[0x32]
        $this->shouldWriteLong($slots + 1 * 4, 1);
    }

    public function test_closed_frame_when_counter_below_threshold()
    {
        $linked = $this->makeLinked(frame: 0, threshold: 5);
        $state = $this->makeState($linked, id: 2, counter: 4);
        $slots = $this->slots();

        $this->call('_attachmentTask_8c02833c')->with(0, $state);

        $this->shouldWriteLong($state + 0x0c, 0); // state[3]
        $this->shouldWriteLong($state + 0xc8, 0); // state[0x32]
        $this->shouldWriteLong($slots + 2 * 4, 0);
    }

    public function test_open_frame_when_linked_is_past_its_first_frame()
    {
        // linked[3] != 0 skips the threshold compare entirely.
        $linked = $this->makeLinked(frame: 1, threshold: 5);
        $state = $this->makeState($linked, id: 0, counter: 0);
        $slots = $this->slots();

        $this->call('_attachmentTask_8c02833c')->with(0, $state);

        $this->shouldWriteLong($state + 0x0c, 1); // state[3]
        $this->shouldWriteLong($state + 0xc8, 0); // state[0x32]
        $this->shouldWriteLong($slots + 0, 1);
    }

    public function test_draws_both_of_the_linked_task_matrices()
    {
        $linked = $this->makeLinked(frame: 1, threshold: 5, markA: 1, markB: 1);
        $state = $this->makeState($linked, id: 0, counter: 0);
        $slots = $this->slots();

        $this->call('_attachmentTask_8c02833c')->with(0, $state);

        $this->shouldWriteLong($state + 0x0c, 1);
        $this->shouldWriteLong($state + 0xc8, 0);
        $this->shouldWriteLong($slots + 0, 1);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_BusDrawSignalAttachment_8c028206'), $state, $linked + 0x34);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_BusDrawSignalAttachment_8c028206'), $state, $linked + 0x74);
    }

    public function test_draws_only_the_second_matrix()
    {
        $linked = $this->makeLinked(frame: 1, threshold: 5, markA: 0, markB: 1);
        $state = $this->makeState($linked, id: 0, counter: 0);
        $slots = $this->slots();

        $this->call('_attachmentTask_8c02833c')->with(0, $state);

        $this->shouldWriteLong($state + 0x0c, 1);
        $this->shouldWriteLong($state + 0xc8, 0);
        $this->shouldWriteLong($slots + 0, 1);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_BusDrawSignalAttachment_8c028206'), $state, $linked + 0x74);
    }
};
