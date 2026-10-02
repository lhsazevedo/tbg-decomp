<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_advances_counter_and_skips_placements_when_positions_are_zero()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);

        $obj = $this->makeObject();
        $this->initUint32($obj + 0x04, 3); // frame counter
        $this->initUint32($obj + 0x1c, fdec(0.0)); // placement A posX
        $this->initUint32($obj + 0x28, fdec(0.0)); // placement B posX

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 4);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
    }

    public function test_counter_reaching_threshold_at_last_slot_wraps_to_zero()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);

        $obj = $this->makeObject(thresholds: [100, 100, 100]);
        $this->initUint32($obj + 0x00, 1); // type/slot id
        $this->initUint32($obj + 0x04, 99); // frame counter, one below threshold
        $this->initUint32($obj + 0x0c, 2); // threshold index, last slot
        $this->initUint32($obj + 0x1c, fdec(0.0)); // placement A posX
        $this->initUint32($obj + 0x28, fdec(0.0)); // placement B posX

        $slots = $this->alloc(0x0c);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $slots);

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 100);
        $this->shouldWriteLong($obj + 0x04, 0);
        $this->shouldWriteLong($obj + 0x0c, 3);
        $this->shouldWriteLong($obj + 0x0c, 0);
        $this->shouldWriteLong($slots + 1 * 4, 0);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
    }

    public function test_counter_reaching_threshold_cycles_slot_and_updates_lookup()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);

        $obj = $this->makeObject(thresholds: [100, 100, 100]);
        $this->initUint32($obj + 0x00, 2); // type/slot id
        $this->initUint32($obj + 0x04, 99); // frame counter, one below threshold
        $this->initUint32($obj + 0x0c, 1); // threshold index
        $this->initUint32($obj + 0x1c, fdec(0.0)); // placement A posX
        $this->initUint32($obj + 0x28, fdec(0.0)); // placement B posX

        $slots = $this->alloc(0x0c);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $slots);

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 100);
        $this->shouldWriteLong($obj + 0x04, 0);
        $this->shouldWriteLong($obj + 0x0c, 2);
        $this->shouldWriteLong($slots + 2 * 4, 2);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
    }

    public function test_placementA_within_range_pushes_fade_call_and_marks_active()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_RenderQueueDraw2_8c022420', 4);

        $obj = $this->makeObject();
        $this->initUint32($obj + 0x1c, fdec(1.0)); // placement A posX
        $this->initUint32($obj + 0x24, fdec(0.0)); // placement A posZ
        $this->initUint32($obj + 0x28, fdec(0.0)); // placement B posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, fdec(1.0)); // posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, fdec(0.0)); // posZ

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 1);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(0.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_BusDrawSignal_8c0281ac'), $obj, $obj + 0x34);
        $this->shouldWriteLong($obj + 0xc8, 1);
    }

    public function test_placementA_out_of_range_does_not_push_fade_call()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $this->setSize('_njSqrt', 4);

        $obj = $this->makeObject();
        $this->initUint32($obj + 0x1c, fdec(1.0)); // placement A posX
        $this->initUint32($obj + 0x24, fdec(0.0)); // placement A posZ
        $this->initUint32($obj + 0x28, fdec(0.0)); // placement B posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, fdec(1.0)); // posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, fdec(0.0)); // posZ

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 1);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(200.0);
    }

    public function test_placementA_just_under_range_pushes_fade_call()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_RenderQueueDraw2_8c022420', 4);

        $obj = $this->makeObject();
        $this->initUint32($obj + 0x1c, fdec(1.0)); // placement A posX
        $this->initUint32($obj + 0x24, fdec(0.0)); // placement A posZ
        $this->initUint32($obj + 0x28, fdec(0.0)); // placement B posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, fdec(1.0)); // posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, fdec(0.0)); // posZ

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 1);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
        // Just under the 200.0 boundary: still pinned as "in range".
        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(199.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_BusDrawSignal_8c0281ac'), $obj, $obj + 0x34);
        $this->shouldWriteLong($obj + 0xc8, 1);
    }

    public function test_placementB_within_range_pushes_fade_call_and_marks_active()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_RenderQueueDraw2_8c022420', 4);

        $obj = $this->makeObject();
        $this->initUint32($obj + 0x1c, fdec(0.0)); // placement A posX
        $this->initUint32($obj + 0x28, fdec(1.0)); // placement B posX
        $this->initUint32($obj + 0x30, fdec(0.0)); // placement B posZ
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, fdec(1.0)); // posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, fdec(0.0)); // posZ

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 1);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(0.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_BusDrawSignal_8c0281ac'), $obj, $obj + 0x74);
        $this->shouldWriteLong($obj + 0xcc, 1);
    }

    public function test_placementB_out_of_range_does_not_push_fade_call()
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $this->setSize('_njSqrt', 4);

        $obj = $this->makeObject();
        $this->initUint32($obj + 0x1c, fdec(0.0)); // placement A posX
        $this->initUint32($obj + 0x28, fdec(1.0)); // placement B posX
        $this->initUint32($obj + 0x30, fdec(0.0)); // placement B posZ
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2fc, fdec(1.0)); // posX
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x304, fdec(0.0)); // posZ

        $this->call('_headTask_8c028258')->with(0, $obj);

        $this->shouldWriteLong($obj + 0x04, 1);
        $this->shouldWriteLong($obj + 0xcc, 0);
        $this->shouldWriteLong($obj + 0xc8, 0);
        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(200.0);
    }

    private function makeObject(array $thresholds = [100, 100, 100]): int
    {
        $obj = $this->alloc(0xd0);

        $this->initUint32($obj + 0x00, 0); // type/slot id
        $this->initUint32($obj + 0x04, 0); // frame counter
        $table = $this->alloc(count($thresholds) * 4);
        foreach ($thresholds as $i => $value) {
            $this->initUint32($table + $i * 4, $value);
        }
        $this->initUint32($obj + 0x08, $table);
        $this->initUint32($obj + 0x0c, 0); // threshold index

        return $obj;
    }
};
