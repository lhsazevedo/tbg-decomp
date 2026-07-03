<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_state_5_skips_drawing()
    {
        $this->resolveSymbols();

        // State 5 hides the grid: no draws at all.
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);

        $this->singleCall('_AlbumDrawGrid_8c01d290')->run();
    }

    public function test_draws_received_letters_then_frame()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);

        // Letters received in slots 0, 2 and 4.
        $this->initLetters([1, 0, 1, 0, 1, 0]);

        // One sprite per received letter; spriteNo is slot + 1.
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1,
            0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3,
            0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5,
            0.0, 0.0, -4.0
        );

        // Grid frame.
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0,
            0.0, 0.0, -5.0
        );

        $this->singleCall('_AlbumDrawGrid_8c01d290')->run();
    }

    public function test_draws_all_letters_then_frame()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);

        $this->initLetters([1, 1, 1, 1, 1, 1]);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3, 0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0
        );

        $this->singleCall('_AlbumDrawGrid_8c01d290')->run();
    }

    public function test_no_letters_draws_only_frame()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);

        $this->initLetters([0, 0, 0, 0, 0, 0]);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0
        );

        $this->singleCall('_AlbumDrawGrid_8c01d290')->run();
    }

    private function initLetters(array $letters): void
    {
        // PlayerProgress.letters_0x2c lives at var_progress + 0x2c.
        $base = $this->addressOf('_var_progress_8c1ba1cc') + 0x2c;
        foreach ($letters as $i => $value) {
            $this->initUint32($base + $i * 4, $value);
        }
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);

        // Functions
        $this->setSize('_drawSprite_8c014f54', 0x4);
    }
};
