<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_renders_value_with_padding(): void
    {
        $this->resolveSymbols();
        $x = 300.0;
        $y = 128.0;

        $this->call('_drawFixedInteger_8c01803e')->with($x, $y, 42, 5);

        $this->shouldDrawDigit($x, $y, 2);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 4);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
    }

    public function test_renders_value_longer_than_digits_param(): void
    {
        $this->resolveSymbols();
        $x = 256.0;
        $y = 64.0;

        $this->call('_drawFixedInteger_8c01803e')->with($x, $y, 12345, 3);

        $this->shouldDrawDigit($x, $y, 5);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 4);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 3);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 2);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 1);
    }

    public function test_renders_value_exact_digits(): void
    {
        $this->resolveSymbols();
        $x = 512.0;
        $y = 32.0;

        $this->call('_drawFixedInteger_8c01803e')->with($x, $y, 907, 3);

        $this->shouldDrawDigit($x, $y, 7);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 9);
    }

    public function test_renders_zero_with_padding(): void
    {
        $this->resolveSymbols();
        $x = 100.0;
        $y = 200.0;

        $this->call('_drawFixedInteger_8c01803e')->with($x, $y, 0, 4);

        $this->shouldDrawDigit($x, $y, 0);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
        $x -= 19.0;

        $this->shouldDrawDigit($x, $y, 0);
    }

    private function shouldDrawDigit(float $x, float $y, int $digit): void
    {
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            12 + $digit,
            $x,
            $y,
            -4.0,
        );
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('__modls', 4);
        $this->setSize('__divls', 4);
    }
};
