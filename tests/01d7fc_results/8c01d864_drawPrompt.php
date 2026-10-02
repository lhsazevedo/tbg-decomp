<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_draws_only_sprite_when_textbox_has_no_text(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 3);

        $this->call('_drawPrompt_8c01d864');

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            5,
            224.0,
            300.0,
            -5.0,
        );

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff)->andReturn(0);
    }

    public function test_also_draws_textbox_sprite_when_textbox_has_text(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 3);

        $this->call('_drawPrompt_8c01d864');

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            5,
            224.0,
            300.0,
            -5.0,
        );

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff)->andReturn(1);

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8'),
            1,
            0.0,
            0.0,
            -4.3,
        );
    }
};
