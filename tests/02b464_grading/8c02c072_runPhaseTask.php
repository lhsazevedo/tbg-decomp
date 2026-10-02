<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): int
    {
        $base = $this->addressOf('_var_runState_8c2285c4');

        $this->setSize('_var_driveMsgQueue_8c228564', 0x60);
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_messageBoxActive_8c22847c', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_fadeCompleteCallback_8c22656c', 4);
        $this->setSize('_init_adxPlaying_8c03bd80', 4);
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_var_fadeRequest_8c226564', 4);
        $this->setSize('_DriveMsgDraw_8c02b388', 4);

        return $base;
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    private function baselineMsgQueue(): void
    {
        $q = $this->addressOf('_var_driveMsgQueue_8c228564');
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($q + $i * 0x18 + 0x14, 0); // holdFrames = 0 -> ticker no-op
        }
    }

    public function test_phase_0_is_idle_and_only_ticks_message_queue(): void
    {
        $base = $this->resolveSymbols();
        $this->baselineMsgQueue();
        $this->initUint32($base + 0x00, 0); // phase 0

        $this->call('_runPhaseTask_8c02c072');

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_DriveMsgDraw_8c02b388'), 0);
    }

    public function test_phase_2_active_driving_ticks_offense_graders(): void
    {
        $base = $this->resolveSymbols();
        $this->baselineMsgQueue();
        $this->initUint32($base + 0x00, 2); // phase 2

        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busState + 0x27c, 0.0); // speed
        $this->initUint32($busState + 0x368, 0);
        $this->initUint32($busState + 0x34c, 0);
        $this->initUint32($busState + 0x36c, 0x80); // skip the steering-angle offense-code branch
        $this->initUint32($busState + 0x358, 0);
        $this->initUint32($busState + 0x374, 1); // != junctionARoadFlags2_0x358 alias -> skip the lane-alias re-latch
        $this->initUint32($busState + 0x390, 0);

        $this->initUint32($base + 0x6c, 0); // var_runState_8c2285c4.lane_0x6c
        $this->initUint32($base + 0x70, 0); // var_runState_8c2285c4.field_0x70[0]

        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 0); // no bump flags -> no vibration
        $this->initUint32($this->addressOf('_var_cooldownCollision_8c228690'), 5); // cooldowns all still active -> no graders fire
        $this->initUint32($this->addressOf('_var_cooldownOffCourse_8c228694'), 5);
        $this->initUint32($this->addressOf('_var_cooldownSignal_8c228698'), 5);
        $this->initUint32($this->addressOf('_var_cooldownLane_8c22869c'), 5);
        $this->initUint32($this->addressOf('_var_cooldownIntersection_8c2286a0'), 5);

        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 50); // still > 0 -> stays in phase 2

        $this->call('_runPhaseTask_8c02c072');

        $this->shouldWriteFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);
        $this->shouldWriteLong($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->shouldWriteLong($this->addressOf('_var_headingVsRoad_8c22868c'), 0); // junctionBAttr1_0x36c high bit set -> skip grading
        $this->shouldWriteLong($this->addressOf('_var_prevLane_8c228684'), 0); // var_runState_8c2285c4.lane_0x6c
        $this->shouldWriteLong($this->addressOf('_var_prevLaneFlags_8c228688'), 0); // var_runState_8c2285c4.field_0x70[0]
        $this->shouldWriteLong($this->addressOf('_var_laneA_8c228674'), 0);
        $this->shouldWriteLong($this->addressOf('_var_laneB_8c228678'), 1);
        $this->shouldWriteLong($this->addressOf('_var_laneC_8c22867c'), 0);

        // Same-object callees: mock rather than let them really execute
        // (their own dependencies aren't set up in this test).
        $this->shouldCall('_handleBump_8c02b6d4')->andReturn(0);
        $this->shouldCall('_gradeOffCourseSevere_8c02b864')->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownCollision_8c228690'), 4);
        $this->shouldCall('_gradeFrame_8c02bcd8')->andReturn(0);

        $this->shouldWriteLong($base + 0x74, 0); // var_runState_8c2285c4.field_0x70[1]
        $this->shouldWriteLong($base + 0x78, 0); // var_runState_8c2285c4.field_0x70[2]

        // var_runState_8c2285c4.driverPoints_0x0c stays > 0 here, so the phase-4 handoff
        // (and its SndStartAdxFadeOut pair) does NOT fire -- goes straight
        // to the tail's message-queue tick and fade-command push.
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_DriveMsgDraw_8c02b388'), 0);
    }

    public function test_phase_3_end_of_stop_grades_arrival_and_starts_fadeout(): void
    {
        $base = $this->resolveSymbols();
        $this->baselineMsgQueue();
        $this->initUint32($base + 0x00, 3); // phase 3
        $this->initUint32($base + 0x24, 0); // var_runState_8c2285c4.stopArrivalGrade_0x24

        $this->setSize('_var_nextStopSegment_8c228710', 4);
        $this->setSize('_var_nextStopHeading_8c228714', 4);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);

        // var_nextStopHeading_8c228714 is the upcoming stop's heading angle, not the segment
        // index var_nextStopSegment_8c228710 -- set them apart so a test
        // reading the wrong symbol fails.
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0);
        $this->initUint32($this->addressOf('_var_nextStopHeading_8c228714'), 0x1000);

        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busState + 0x250, 0); // ang_0x250
        $this->initUint32($busState + 0x25c, 1); // skip the -8 stop-precision adjust
        $this->initUint32($base + 0x58, 0x3c); // var_runState_8c2285c4.signalHeldFrames_0x58

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0); // skip GradeRunComplete_8c02c586 gate

        $this->call('_runPhaseTask_8c02c072');

        // diff = var_nextStopHeading_8c228714(0x1000) - ang_0x250(0) = 0x1000, inside
        // (0x71c, 0xf8e3) -> heading-mismatch penalty fires.
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x1b, 0xfffffffd); // -3

        $this->shouldWriteLong($base + 0x00, 4);
        $this->shouldWriteLong($base + 0x08, 0x1e);
        $this->shouldWriteLong($this->addressOf('_var_messageBoxActive_8c22847c'), 1);
        $this->shouldWriteLongTo('_var_fadeCompleteCallback_8c22656c', $this->addressOf('_GradeOnFadePracticeEnd_8c02c784'));

        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_DriveMsgDraw_8c02b388'), 0);
    }
};
