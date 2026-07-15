<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_inactive_without_activation_returns_1()
    {
        $this->setup(press: 0, ctrl: 0);

        $this->call('_Update_8c0129cc');

        // (press&8)==0 && ctrl!=-1 -> no activation, just returns 1.
        $this->shouldReturn(1);
    }

    public function test_activates_and_returns_1()
    {
        $this->setup(press: 8, ctrl: 0, vibport: -1);

        $this->call('_Update_8c0129cc');

        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_pauseSettle_8c18ad04'), 0);
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_controlAdxtWithOutVol_8c0107d2')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldReturn(1);
    }

    public function test_activates_and_stops_vibration()
    {
        $this->setup(press: 8, ctrl: 0, vibport: 3);

        $this->call('_Update_8c0129cc');

        // vibport != -1 -> also stop the vibration motor.
        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_pauseSettle_8c18ad04'), 0);
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_controlAdxtWithOutVol_8c0107d2')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_pdVibMxStop')->with(3);
        $this->shouldReturn(1);
    }

    public function test_ad04_pending_advances_to_2()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad04: 1);

        $this->call('_Update_8c0129cc');

        $this->shouldWriteLong($this->addressOf('_var_pauseSettle_8c18ad04'), 2);
        $this->shouldReturn(0);
    }

    public function test_ad04_settled_returns_0()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad04: 2);

        $this->call('_Update_8c0129cc');

        $this->shouldReturn(0);
    }

    public function test_press_start_deactivates_and_base_draws()
    {
        $this->setup(press: 8, ctrl: 0, bb8cc: 1, ad04: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 0);
        $this->shouldCall('_controlAdxtWithOutVol_8c0107d2')->with(0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad10_init_press4_deactivates_and_draws_75()
    {
        $this->setup(press: 4, ctrl: 0, bb8cc: 1, ad04: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10==0, press&4 -> stop clip, replay midi, draw steer sprite 0x75.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 0);
        $this->shouldCall('_controlAdxtWithOutVol_8c0107d2')->with(0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad10_init_steer_engages_and_draws_75()
    {
        $this->setup(press: 0x20, ctrl: 0, bb8cc: 1, ad04: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10==0, press&0x20 -> engage steering, draw sprite 0x75.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 1);
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad10_init_steer_engages_on_y1_tilt()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad04: 0, y1: 0x41);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10==0, y1 > 0x40 -> engage steering, draw sprite 0x75.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 1);
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad10_init_idle_just_draws_75()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad04: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10==0, no input -> no state change, just draw 0x75 then base.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_zero_press4_advances_to_1_and_draws_7a()
    {
        $this->setup(press: 4, ctrl: 0, bb8cc: 1, ad04: 0, ad10: 1, ad08: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10!=0, ad08==0, press&4 -> ad08=1, replay midi, draw 0x7a.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_zero_press10_disengages_and_draws_7a()
    {
        $this->setup(press: 0x10, ctrl: 0, bb8cc: 1, ad04: 0, ad10: 1, ad08: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10!=0, ad08==0, press&0x10 -> ad10=0, midi(1,3,0), draw 0x7a.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_zero_disengages_on_y1_tilt()
    {
        // y1 = -0x41 (0xffbf as u16) < -0x40.
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad04: 0, ad10: 1, ad08: 0, y1: 0xffbf);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_zero_idle_just_draws_7a()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad04: 0, ad10: 1, ad08: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad10!=0, ad08==0, no input -> no state change, just draw 0x7a then base.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_press2_resets_and_draws_7a()
    {
        $this->setup(press: 2, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad08==1, press&2 -> ad08=0, draw 0x7a, then midi(1,1,0).
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 1, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c0_press4_fades_to_2()
    {
        $this->setup(press: 4, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad08==1, !press&2, ad0c==0, press&4 -> ad08=2, fadeout(10), midi(1,0,0), no arrow draw.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 2);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c0_press80_toggles_right_draws_76()
    {
        $this->setup(press: 0x80, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad0c==0, press&0x80 -> ad0c=1, midi(1,3,0), draw 0x76.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c0_x1_toggles_right_draws_76()
    {
        // x1 = 0x41 > 0x40.
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 0, x1: 0x41);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c0_idle_draws_76()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad0c==0, no toggle input -> just draw 0x76.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c1_press4_resets_draws_77()
    {
        $this->setup(press: 4, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad0c!=0, press&4 -> ad08=0, midi(1,0,0), draw 0x77.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c1_press40_toggles_left_draws_77()
    {
        $this->setup(press: 0x40, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad0c!=0, !press&4, press&0x40 -> ad0c=0, midi(1,3,0), draw 0x77.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c1_x1_toggles_left_draws_77()
    {
        // x1 = -0x41 (0xffbf as u16) < -0x40.
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 1, x1: 0xffbf);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_one_ad0c1_idle_draws_77()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 1, ad0c: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad0c!=0, no toggle input -> just draw 0x77.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_two_fading_draws_76_only()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 2, isFading: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_Update_8c0129cc');

        // ad08==2, isFading -> draw 0x76 and njDrawPolygon, skip base 0x74 draw.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldCall('_drawSprite_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_ad08_two_practice_commits_menu_selection()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 2, isFading: 0, playMode: 1);
        $this->initUint32($this->addressOf('_var_8c22640c'), 0x2b);

        $this->call('_Update_8c0129cc');

        // isFading==0 -> reset flags + FUN_8c016182; playMode==PRACTICE -> menuState.selected = var_8c22640c, FUN_8c01f21c.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8dc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8bc'), 0);
        $this->shouldCall('_FUN_8c016182');
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 0x2b);
        $this->shouldCall('_FUN_8c01f21c');
        $this->shouldReturn(0);
    }

    public function test_ad08_two_normal_copies_progress_and_opens_course_menu()
    {
        $this->setup(press: 0, ctrl: 0, bb8cc: 1, ad10: 1, ad08: 2, isFading: 0, playMode: 0);
        $b8 = $this->addressOf('_var_8c1ba2b8');
        $cc = $this->addressOf('_var_8c1ba2cc');
        $prog = $this->addressOf('_var_progress_8c1ba1cc');
        for ($i = 0; $i < 5; $i++) {
            $this->initUint32($b8 + $i * 4, 0x100 + $i);
            $this->initUint32($cc + $i * 4, 0x200 + $i);
        }

        $this->call('_Update_8c0129cc');

        // isFading==0 -> reset flags + FUN_8c016182; playMode==NORMAL -> copy backups into progress, CourseMenuFUN.
        $this->shouldCall('_FUN_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8dc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8bc'), 0);
        $this->shouldCall('_FUN_8c016182');
        for ($i = 0; $i < 5; $i++) {
            // field_0x04[i] = var_8c1ba2b8[i]; field_0x18[i] = var_8c1ba2cc[i].
            $this->shouldWriteLong($prog + 0x04 + $i * 4, 0x100 + $i);
            $this->shouldWriteLong($prog + 0x18 + $i * 4, 0x200 + $i);
        }
        $this->shouldCall('_CourseMenuFUN_8c017ef2');
        $this->shouldReturn(0);
    }

    private function setup(int $press, int $ctrl, int $vibport = 0, int $bb8cc = 0, int $ad04 = 0, int $ad10 = 0, int $y1 = 0, int $ad08 = 0, int $ad0c = 0, int $x1 = 0, int $isFading = 0, int $playMode = 0): void
    {
        $this->setSize('_FUN_8c022560', 4);
        $this->setSize('_drawSprite_8c014f54', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('_init_8c03bf4c', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_controlAdxtWithOutVol_8c0107d2', 4);
        $this->setSize('_pdVibMxStop', 4);
        $this->setSize('_push_fadeout_8c022b60', 4);
        $this->setSize('_FUN_8c016182', 4);
        $this->setSize('_CourseMenuFUN_8c017ef2', 4);
        $this->setSize('_FUN_8c01f21c', 4);

        $this->setSize('_var_8c1bb8dc', 4);
        $this->setSize('_var_8c1bb8bc', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_8c22640c', 4);
        $this->setSize('_var_menuState_8c1bc7a8', 0x80);
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $this->setSize('_var_8c1ba2b8', 0x14);
        $this->setSize('_var_8c1ba2cc', 0x14);

        $this->setSize('_var_pauseActive_8c1bb8cc', 4);
        $this->setSize('_var_8c1bb8b8', 4);
        $this->setSize('_var_pauseSettle_8c18ad04', 4);
        $this->setSize('_var_retirePhase_8c18ad08', 4);
        $this->setSize('_var_confirmChoice_8c18ad0c', 4);
        $this->setSize('_var_onRetire_8c18ad10', 4);

        $periph = $this->alloc(0x34);
        $this->initUint32($periph + 0x10, $press);
        $this->initUint16($periph + 0x1c, $x1);
        $this->initUint16($periph + 0x1e, $y1);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba358'), $periph);

        $this->initUint32($this->addressOf('_var_pauseActive_8c1bb8cc'), $bb8cc);
        $this->initUint32($this->addressOf('_var_pauseSettle_8c18ad04'), $ad04);
        $this->initUint32($this->addressOf('_var_onRetire_8c18ad10'), $ad10);
        $this->initUint32($this->addressOf('_var_retirePhase_8c18ad08'), $ad08);
        $this->initUint32($this->addressOf('_var_confirmChoice_8c18ad0c'), $ad0c);
        $this->initUint32($this->addressOf('_var_activeCtrlType_8c157a70'), $ctrl);
        $this->initUint32($this->addressOf('_var_8c22847c'), 0);
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), $vibport);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), $playMode);
    }
};
