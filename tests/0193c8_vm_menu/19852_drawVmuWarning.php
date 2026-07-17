<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_drawFirstOption()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x3c, 0x0);

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c, 0x11,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x00,
            2,
            228.0, 304.0, -5.0
        );

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x00, 0,
            0.0, 0.0, -7.0
        );

        $this->singleCall('_drawVmWarning_8c019852')->run();
    }

    public function test_drawSecondOption()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x3c, 0x1);

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c, 0x11,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x00,
            3,
            228.0, 304.0, -5.0
        );

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x00, 0,
            0.0, 0.0, -7.0
        );

        $this->singleCall('_drawVmWarning_8c019852')->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        // Functions
        $this->setSize('_TxtDrawSprite_8c014f54', 0x4);
    }

    private function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    private function entryName(): string
    {
        return $this->isAsmObject()
            ? '_vmMenuTask_8c0198a0'
            : '_vmMenuTask_8c0198a0';
    }
};
