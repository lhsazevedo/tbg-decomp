<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _StopAdvance_8c02ccc6(void): locks in the current stop's
 * heading angle (var_currentStopHeading_8c2288fc, njArcTan2 of the stop-area record var_nextStopSegment_8c228710
 * was still pointing at, masked to an unsigned 16-bit angle), then advances
 * var_nextStopSegment_8c228710 to the next segment with an active-stop flag
 * (var_segmentHasStop_8c2286a4) and primes that upcoming stop's position
 * (var_nextStopPoint_8c228900.x/.z) and heading angle (var_nextStopHeading_8c228714, stored as the raw
 * sum then re-stored sign-extended from its low 16 bits when negative)
 * from its stop-area record (StopGetArea_8c02cd7a).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_atan2f', 4);

        // _StopGetArea_8c02cd7a is same-object -- mock with shouldCall() directly,
        // no setSize().
    }

    // stop-area record: +4/+8 (x, z) origin, +0xc/+0x10 (dx, dz) direction.
    // dx/dz aren't read by the C body directly (njArcTan2/atan2f is mocked
    // below) but are still initialized so the reads aren't garbage.
    private function allocStopAreaRecord(float $x, float $z, float $dx, float $dz): int
    {
        $rec = $this->alloc(0x14);
        $this->initUint32($rec + 4, unpack('L', pack('f', $x))[1]);
        $this->initUint32($rec + 8, unpack('L', pack('f', $z))[1]);
        $this->initUint32($rec + 0xc, unpack('L', pack('f', $dx))[1]);
        $this->initUint32($rec + 0x10, unpack('L', pack('f', $dz))[1]);
        return $rec;
    }

    private function initFlags(array $activeIndices): void
    {
        $base = $this->addressOf('_var_segmentHasStop_8c2286a4');
        for ($i = 0; $i < 24; $i++) {
            $this->initUint32($base + $i * 4, in_array($i, $activeIndices, true) ? 1 : 0);
        }
    }

    public function test_1(): void
    {
        $this->resolveSymbols();

        $this->initFlags([6]);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 5);

        // rec1: dx=0.0, dz=1.0 -> atan2f(0,1)=0 -> angle=0 -> +0x8000 -> 0x8000 (32768)
        $rec1 = $this->allocStopAreaRecord(0.0, 0.0, 0.0, 1.0);

        // rec2: x=10.0, z=20.0, dx=1.0, dz=0.0 -> atan2f(1,0)=pi/2 -> angle=16384
        // -> +0x8000 -> 0xc000 (49152)
        $rec2 = $this->allocStopAreaRecord(10.0, 20.0, 1.0, 0.0);

        $this->call('_StopAdvance_8c02ccc6')->with();

        $this->shouldWriteLongTo('_var_currentSegment_8c228708', 5);
        $this->shouldCall('_StopGetArea_8c02cd7a')->with(5)->andReturn($rec1);
        $this->shouldCall('_atan2f')->with(0.0, 1.0)->andReturn(0.0); // atan2f(dx, dz)
        $this->shouldWriteLongTo('_var_currentStopHeading_8c2288fc', 32768);

        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 6);

        $this->shouldCall('_StopGetArea_8c02cd7a')->with(6)->andReturn($rec2);

        $base = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->shouldWriteFloat($base + 0, 10.0);
        $this->shouldWriteFloat($base + 8, 20.0);

        $this->shouldCall('_atan2f')->with(1.0, 0.0)->andReturn(M_PI_2); // atan2f(dx, dz)
        $this->shouldWriteLongTo('_var_nextStopHeading_8c228714', 49152);
        $this->shouldWriteLongTo('_var_nextStopHeading_8c228714', -16384); // 0x8000 bit set -> sign-extended
    }

    // Multi-iteration search loop (skips two inactive segments), and a
    // heading angle whose sum doesn't set the 0x8000 bit -- only one write
    // to var_nextStopHeading_8c228714, no sign-extend re-store.
    public function test_2_multi_iteration_no_sign_extend(): void
    {
        $this->resolveSymbols();

        $this->initFlags([5]);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 2);

        $rec1 = $this->allocStopAreaRecord(0.0, 0.0, 0.0, 1.0);

        // rec2: dx=-1.0, dz=0.0 -> atan2f(-1,0)=-pi/2 -> angle=-16384
        // -> +0x8000 -> 0x4000 (16384); bit 0x8000 clear -> no second store.
        $rec2 = $this->allocStopAreaRecord(1.0, 2.0, -1.0, 0.0);

        $this->call('_StopAdvance_8c02ccc6')->with();

        $this->shouldWriteLongTo('_var_currentSegment_8c228708', 2);
        $this->shouldCall('_StopGetArea_8c02cd7a')->with(2)->andReturn($rec1);
        $this->shouldCall('_atan2f')->with(0.0, 1.0)->andReturn(0.0);
        $this->shouldWriteLongTo('_var_currentStopHeading_8c2288fc', 32768);

        // segments 3, 4 skipped (flag clear); the global is written back on
        // every increment, not just the final match.
        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 3);
        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 4);
        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 5);

        $this->shouldCall('_StopGetArea_8c02cd7a')->with(5)->andReturn($rec2);

        $base = $this->addressOf('_var_nextStopPoint_8c228900');
        $this->shouldWriteFloat($base + 0, 1.0);
        $this->shouldWriteFloat($base + 8, 2.0);

        $this->shouldCall('_atan2f')->with(-1.0, 0.0)->andReturn(-M_PI_2);
        $this->shouldWriteLongTo('_var_nextStopHeading_8c228714', 16384);
    }
};
