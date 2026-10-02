<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * updateDialogScene_8c01f42c(void): per-frame update and draw of the ending
 * dialog scene, before the credits. Sprite 4 of the ending resource group
 * bounces from MenuState.pos.title (busX_0x20/flagY_0x24) with
 * cursorVelocity_0x30 as its velocity: subState_0x1c 0 falls under gravity
 * until y passes 300, 1 rises until the velocity flips, and x turns around at
 * 0 and 90. Also redraws the instructor portrait and resets the background
 * color every frame.
 */
return new class extends TestCase {
    const MENU_STATE_SIZE = 0x7c;
    const RESOURCE_GROUP_B_0X0C = 0x0c;
    const FIELD_0X1C = 0x1c;
    const BUS_X_0X20 = 0x20;
    const FLAG_Y_0X24 = 0x24;
    const VELOCITY_X_0X30 = 0x30;
    const VELOCITY_Y_0X34 = 0x34;
    const INSTRUCTOR_SPRITE_0X60 = 0x60;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', self::MENU_STATE_SIZE);
        $this->setSize('_var_menuTextboxCharLimit_8c225fb8', 4);
    }

    private function initFloat(int $addr, float $value): void
    {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    // Truncates to float32 precision, mirroring the CPU's single-precision
    // arithmetic so expected values don't drift from double-precision PHP math.
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function seed(int $phase, float $busX, float $flagY, float $velX, float $velY, int $instructorSprite): int
    {
        $base = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($base + self::FIELD_0X1C, $phase);
        $this->initFloat($base + self::BUS_X_0X20, $busX);
        $this->initFloat($base + self::FLAG_Y_0X24, $flagY);
        $this->initFloat($base + self::VELOCITY_X_0X30, $velX);
        $this->initFloat($base + self::VELOCITY_Y_0X34, $velY);
        $this->initUint32($base + self::INSTRUCTOR_SPRITE_0X60, $instructorSprite);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 0x20);
        return $base;
    }

    public function test_phase_growing_below_threshold(): void
    {
        $this->resolveSymbols();
        $base = $this->seed(0, 45.0, 100.0, 1.0, 0.0, 7);

        $velY = $this->f32(0.0 + 0.1);
        $flagY = $this->f32(100.0 + $velY);
        $busX = $this->f32(45.0 + 1.0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::VELOCITY_Y_0X34, $velY);
        $this->shouldWriteFloat($base + self::FLAG_Y_0X24, $flagY);
        $this->shouldWriteFloat($base + self::BUS_X_0X20, $busX);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, $busX, $flagY, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 7, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }

    public function test_phase_growing_crosses_threshold_transitions_to_shrinking(): void
    {
        $this->resolveSymbols();
        $base = $this->seed(0, 45.0, 299.95, 1.0, 0.0, 0);

        $velY = $this->f32(0.0 + 0.1);
        $flagY = $this->f32($this->f32(299.95) + $velY);
        $busX = $this->f32(45.0 + 1.0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::VELOCITY_Y_0X34, $velY);
        $this->shouldWriteFloat($base + self::FLAG_Y_0X24, $flagY);
        $this->shouldWriteLong($base + self::FIELD_0X1C, 1);
        $this->shouldWriteFloat($base + self::BUS_X_0X20, $busX);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, $busX, $flagY, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 0, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }

    public function test_phase_shrinking_above_zero(): void
    {
        $this->resolveSymbols();
        $base = $this->seed(1, 45.0, 300.0, 1.0, 5.0, 0);

        $flagY = $this->f32(300.0 - 5.0);
        $velY = $this->f32(5.0 - 0.1);
        $busX = $this->f32(45.0 + 1.0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::FLAG_Y_0X24, $flagY);
        $this->shouldWriteFloat($base + self::VELOCITY_Y_0X34, $velY);
        $this->shouldWriteFloat($base + self::BUS_X_0X20, $busX);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, $busX, $flagY, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 0, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }

    public function test_phase_shrinking_crosses_zero_transitions_to_growing(): void
    {
        $this->resolveSymbols();
        $base = $this->seed(1, 45.0, 300.0, 1.0, 0.05, 0);

        $vel0 = $this->f32(0.05);
        $flagY = $this->f32(300.0 - $vel0);
        $velY = $this->f32($vel0 - 0.1);
        $busX = $this->f32(45.0 + 1.0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::FLAG_Y_0X24, $flagY);
        $this->shouldWriteFloat($base + self::VELOCITY_Y_0X34, $velY);
        $this->shouldWriteLong($base + self::FIELD_0X1C, 0);
        $this->shouldWriteFloat($base + self::BUS_X_0X20, $busX);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, $busX, $flagY, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 0, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }

    public function test_x_above_limit_negates_velocity(): void
    {
        $this->resolveSymbols();
        // No phase-0/1 vertical work: subState_0x1c holds a value outside 0/1.
        $base = $this->seed(2, 89.5, 150.0, 1.0, 0.0, 0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::BUS_X_0X20, 90.5);
        $this->shouldWriteFloat($base + self::VELOCITY_X_0X30, -1.0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, 90.5, 150.0, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 0, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }

    public function test_x_below_zero_negates_velocity(): void
    {
        $this->resolveSymbols();
        $base = $this->seed(2, 0.5, 150.0, -1.0, 0.0, 0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::BUS_X_0X20, -0.5);
        $this->shouldWriteFloat($base + self::VELOCITY_X_0X30, 1.0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, -0.5, 150.0, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 0, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }

    public function test_menu_textbox_active_draws_extra_sprite(): void
    {
        $this->resolveSymbols();
        $base = $this->seed(2, 45.0, 150.0, 0.0, 0.0, 0);

        $this->call('_updateDialogScene_8c01f42c');

        $this->shouldWriteFloat($base + self::BUS_X_0X20, 45.0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 4, 45.0, 150.0, -8.5);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0x20)->andReturn(1);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base, 1, 0.0, 0.0, -7.0);
        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($base + self::RESOURCE_GROUP_B_0X0C, 0, 0.0, 0.0, -8.0);
        $this->shouldCall('_njSetBackColor')->with(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
    }
};
