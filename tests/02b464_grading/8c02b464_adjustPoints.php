<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20); // 8 x SDMIDI
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_driveMsgQueue_8c228564', 0x60); // 4 x DriveMsgSlot
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
    }

    private function pointsAddr(): int
    {
        return $this->addressOf('_var_runState_8c2285c4') + 0xc;
    }

    private function maxAddr(): int
    {
        return $this->addressOf('_var_runState_8c2285c4') + 0x10;
    }

    private function difficultyAddr(): int
    {
        return $this->addressOf('_var_progress_8c1ba1cc') + 0xc4;
    }

    public function test_no_op_while_scored_demo_playing(): void
    {
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2);

        $this->call('_adjustPoints_8c02b464')->with(5, 0xfffffff6); // delta = -10
    }

    public function test_penalty_counts_and_tracks_worst_offense_silently(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_penaltyCount_8c1bb8f4'), 3);
        $this->initUint32($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0xfffffffb); // -5, current worst
        $this->initUint32($this->addressOf('_var_worstPenaltyMsgSet_8c1bb8ec'), 7); // current worst's msgSet
        $this->initUint32($this->pointsAddr(), 50);
        $this->initUint32($this->maxAddr(), 100);

        // msgSet = -1: no banner queued, no sound.
        $this->call('_adjustPoints_8c02b464')->with(0xffffffff, 0xfffffff6); // -10, worse than -5

        $this->shouldWriteLongTo('_var_penaltyCount_8c1bb8f4', 4);
        $this->shouldWriteLong($this->pointsAddr(), 40);
        $this->shouldWriteLongTo('_var_worstPenaltyDelta_8c1bb8f0', 0xfffffff6);
        $this->shouldWriteLongTo('_var_worstPenaltyMsgSet_8c1bb8ec', 0xffffffff);
    }

    public function test_penalty_not_worse_than_current_leaves_worst_offense_alone(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_penaltyCount_8c1bb8f4'), 3);
        $this->initUint32($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0xfffffff6); // -10, current worst
        $this->initUint32($this->addressOf('_var_worstPenaltyMsgSet_8c1bb8ec'), 7);
        $this->initUint32($this->pointsAddr(), 50);
        $this->initUint32($this->maxAddr(), 100);

        $this->call('_adjustPoints_8c02b464')->with(0xffffffff, 0xfffffffb); // -5, not worse

        $this->shouldWriteLongTo('_var_penaltyCount_8c1bb8f4', 4);
        $this->shouldWriteLong($this->pointsAddr(), 45);
    }

    public function test_bonus_clamps_to_max_and_skips_worst_offense_tracking(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint8($this->difficultyAddr(), 0); // DIFFICULTY != 2, so no gate
        $this->initUint32($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0xfffffff6);
        $this->initUint32($this->pointsAddr(), 95);
        $this->initUint32($this->maxAddr(), 100);

        $this->call('_adjustPoints_8c02b464')->with(0xffffffff, 10); // +10, over the max

        $this->shouldWriteLong($this->pointsAddr(), 105);
        $this->shouldWriteLong($this->pointsAddr(), 100); // re-clamped
    }

    public function test_deficit_clamps_to_zero(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_penaltyCount_8c1bb8f4'), 0);
        $this->initUint32($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0);
        $this->initUint32($this->addressOf('_var_worstPenaltyMsgSet_8c1bb8ec'), 0);
        $this->initUint32($this->pointsAddr(), 3);
        $this->initUint32($this->maxAddr(), 100);

        $this->call('_adjustPoints_8c02b464')->with(0xffffffff, 0xfffffff6); // -10

        $this->shouldWriteLongTo('_var_penaltyCount_8c1bb8f4', 1);
        $this->shouldWriteLong($this->pointsAddr(), 0xfffffff9); // 3 - 10
        $this->shouldWriteLongTo('_var_worstPenaltyDelta_8c1bb8f0', 0xfffffff6);
        $this->shouldWriteLongTo('_var_worstPenaltyMsgSet_8c1bb8ec', 0xffffffff);
        $this->shouldWriteLong($this->pointsAddr(), 0); // re-clamped
    }

    public function test_bonus_gated_at_difficulty_2_outside_scored_run(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint8($this->difficultyAddr(), 2); // DIFFICULTY == 2

        // Positive delta while gated: no writes at all, function returns immediately.
        $this->call('_adjustPoints_8c02b464')->with(0xffffffff, 10);
    }

    public function test_bonus_not_gated_at_difficulty_2_during_scored_run(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // scored run
        $this->initUint8($this->difficultyAddr(), 2); // DIFFICULTY == 2
        $this->initUint32($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0);
        $this->initUint32($this->pointsAddr(), 50);
        $this->initUint32($this->maxAddr(), 100);

        $this->call('_adjustPoints_8c02b464')->with(0xffffffff, 10);

        $this->shouldWriteLong($this->pointsAddr(), 60);
    }

    public function test_bonus_queues_praise_banner_and_shifts_queue(): void
    {
        $this->resolveSymbols();
        $this->doNotRandomizeMemory();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint8($this->difficultyAddr(), 0); // DIFFICULTY != 2
        $this->initUint32($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0);
        $this->initUint32($this->pointsAddr(), 50);
        $this->initUint32($this->maxAddr(), 100);

        $queue = $this->addressOf('_var_driveMsgQueue_8c228564');
        for ($i = 0; $i < 6 * 4; $i += 4) {
            $this->initUint32($queue + $i, 0x11111111);      // slot 0
            $this->initUint32($queue + 0x18 + $i, 0x22222222); // slot 1
            $this->initUint32($queue + 0x30 + $i, 0x33333333); // slot 2
            $this->initUint32($queue + 0x48 + $i, 0x44444444); // slot 3
        }

        // init_8c04bef0 = { count = 6, ids = 32, 33, 34, 35, 36, 37 }
        $msgSet = 0;

        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0x14, 0x1234);

        $this->call('_adjustPoints_8c02b464')->with($msgSet, 10); // +10, praise jingle

        $this->shouldWriteLong($this->pointsAddr(), 60);

        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 4, 0);

        $evnMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];
            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };
        // slot[3] = slot[2]; slot[2] = slot[1]; slot[1] = slot[0] (slot[3]'s old contents are dropped).
        $this->shouldCall('__quick_evn_mvn')->do($evnMvn);
        $this->shouldCall('__quick_evn_mvn')->do($evnMvn);
        $this->shouldCall('__quick_evn_mvn')->do($evnMvn);

        // slot[0] <- new banner: count=6, duration = ((6*-32+640) + 0) >> 1 = 224
        $this->shouldWriteLong($queue + 0x00, 6);
        $this->shouldWriteLong($queue + 0x04, $this->addressOf('_init_8c04bef0') + 4);
        $this->shouldWriteFloat($queue + 0x08, 224.0);
        $this->shouldWriteLong($queue + 0x0c, 0);
        $this->shouldWriteLong($queue + 0x10, 0);
        $this->shouldWriteLong($queue + 0x14, 0x3c);
    }
};
