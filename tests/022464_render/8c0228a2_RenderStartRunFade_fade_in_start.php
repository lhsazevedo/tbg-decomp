<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
    }

    public function test_normal_play_mode_sets_mirror_flag(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // PLAY_MODE_NORMAL
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);

        $this->call('_RenderStartRunFade_8c0228a2');

        $this->shouldWriteLongTo('_var_viewLayout_8c22655c', 1);
        $this->shouldWriteLongTo('_var_fadePhase_8c227d7c', 0);
        $this->shouldWriteLongTo('_var_sceneVisible_8c226560', 1);
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 1);
        $this->shouldWriteLongTo('_var_fadeCompleteCallback_8c22656c', 0xffffffff);
        $this->shouldWriteLongTo('_var_isFading_8c226568', 1);
    }

    public function test_mirror_flag_bit_set_sets_mirror_flag(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 8);

        $this->call('_RenderStartRunFade_8c0228a2');

        $this->shouldWriteLongTo('_var_viewLayout_8c22655c', 1);
        $this->shouldWriteLongTo('_var_fadePhase_8c227d7c', 0);
        $this->shouldWriteLongTo('_var_sceneVisible_8c226560', 1);
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 1);
        $this->shouldWriteLongTo('_var_fadeCompleteCallback_8c22656c', 0xffffffff);
        $this->shouldWriteLongTo('_var_isFading_8c226568', 1);
    }

    public function test_neither_condition_clears_mirror_flag(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);

        $this->call('_RenderStartRunFade_8c0228a2');

        $this->shouldWriteLongTo('_var_viewLayout_8c22655c', 0);
        $this->shouldWriteLongTo('_var_fadePhase_8c227d7c', 0);
        $this->shouldWriteLongTo('_var_sceneVisible_8c226560', 1);
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 1);
        $this->shouldWriteLongTo('_var_fadeCompleteCallback_8c22656c', 0xffffffff);
        $this->shouldWriteLongTo('_var_isFading_8c226568', 1);
    }
};
