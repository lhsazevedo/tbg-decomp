<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const STATE = 0x18;       // var_menuState_8c1bc7a8.state_0x18 offset
    const SELECTED = 0x38;    // var_menuState_8c1bc7a8.selected_0x38 offset

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    /*
     * Install the AUDIO task, reset the phase/cursor, clear the three sound-test
     * digit fields, and kick the fade-in. The clear zeroes 9 longs (each field
     * high digit first); var_voiceTestDigits_8c226088[0] is written twice -- once off the SFX base
     * (var_sfxTestDigits_8c226080 + 8) and once off the VOICE base; the unit lays the three
     * arrays out contiguously in BSS.
     */
    public function test_switch_to_audio()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_TaskReinstall_8c014b3e', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);

        $music = $this->addressOf('_var_musicTestDigits_8c226078');
        $sfx = $this->addressOf('_var_sfxTestDigits_8c226080');
        $voice = $this->addressOf('_var_voiceTestDigits_8c226088');

        $task = $this->alloc(0x20);
        $this->call('_enterAudio_8c01afd8')->with($task);

        $this->shouldCall('_TaskReinstall_8c014b3e')
            ->with($task, $this->addressOf('_audioTask_8c01ab08'));
        $this->shouldWriteLong($this->menu(self::STATE), 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), 0);
        $this->shouldWriteLong($music + 4, 0);   // MUSIC[1]
        $this->shouldWriteLong($music + 0, 0);   // MUSIC[0]
        $this->shouldWriteLong($sfx + 8, 0);     // SFX base + 8 == VOICE[0]
        $this->shouldWriteLong($sfx + 4, 0);     // SFX[1]
        $this->shouldWriteLong($sfx + 0, 0);     // SFX[0]
        $this->shouldWriteLong($voice + 12, 0);  // VOICE[3]
        $this->shouldWriteLong($voice + 8, 0);   // VOICE[2]
        $this->shouldWriteLong($voice + 4, 0);   // VOICE[1]
        $this->shouldWriteLong($voice + 0, 0);   // VOICE[0] (again)
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }
};
