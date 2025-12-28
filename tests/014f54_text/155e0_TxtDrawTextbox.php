<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_skips_when_no_text()
    {
        $this->resolveSymbols();

        [$box] = $this->createTextbox(
            '',
            characterCount: 0,
            tagCount: 0,
            x: 0,
            y: 500,
        );

        // Force NULL text pointer
        $this->initUint32($box + 0x38, 0);

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_skips_when_text_is_empty()
    {
        $this->resolveSymbols();

        [$box] = $this->createTextbox(
            '',
            characterCount: 0,
            tagCount: 0,
            x: 0,
            y: 500,
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_it_loads_and_draws_simple_text()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);

        $var_glyphTexnames_8c1bc78c = $this->alloc(0x200 * 0x0c);
        // $this->initUint16Array($var_glyphTexnames_8c1bc78c, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), $var_glyphTexnames_8c1bc78c);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 0x8);
        // $this->initUint16Array($var_glyphTexlists_8c1bc790, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_busFont_8c1ba1c8'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0002);

        [$box, $glyphIndexes] = $this->createTextbox(
            'ÇÅÇÇÇÉ',
            characterCount: 3,
            tagCount: 0,
            x: 0,
            y: 500,
        );

        $shouldLoadGlyphContext = [
            "box" => $box,
            "var_glyphTexnames_8c1bc78c" => $var_glyphTexnames_8c1bc78c,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
            "var_8c1bc7a0" => $var_8c1bc7a0,
            "glyphIndexes" => $glyphIndexes,
        ];

        $shouldDrawCharacterContext = [
            "box" => $box,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
        ];

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            0,
            tokenIndex:            0,
            newProcessedCharCount: 1,
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 0,
            x:          0.0,
            y:          500.0,
            priority:   42.0,
        );

        // Load and draw ÇÇ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8282,
            glyphIndex:            1,
            tokenIndex:            1,
            newProcessedCharCount: 2
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 1,
            x:          24.0,
            y:          500.0,
            priority:   42.0,
        );

        // Load and draw ÇÉ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8283,
            glyphIndex:            2,
            tokenIndex:            2,
            newProcessedCharCount: 3
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 2,
            x:          48.0,
            y:          500.0,
            priority:   42.0,
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box, 3)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_it_reloads_already_loaded_glyphs()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);

        $var_glyphTexnames_8c1bc78c = $this->alloc(0x200 * 0x0c);
        // $this->initUint16Array($var_glyphTexnames_8c1bc78c, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), $var_glyphTexnames_8c1bc78c);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 0x8);
        // $this->initUint16Array($var_glyphTexlists_8c1bc790, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_busFont_8c1ba1c8'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0002);

        [$box, $glyphIndexes] = $this->createTextbox(
            'ÇÅÇÇÇÅ',
            characterCount: 3,
            tagCount: 0,
            x: 0,
            y: 500,
        );

        $shouldLoadGlyphContext = [
            "box" => $box,
            "var_glyphTexnames_8c1bc78c" => $var_glyphTexnames_8c1bc78c,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
            "var_8c1bc7a0" => $var_8c1bc7a0,
            "glyphIndexes" => $glyphIndexes,
        ];

        $shouldDrawCharacterContext = [
            "box" => $box,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
        ];

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            0,
            tokenIndex:            0,
            newProcessedCharCount: 1
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 0,
            x:          0.0,
            y:          500.0,
            priority:   42.0
        );

        // Load and draw ÇÇ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8282,
            glyphIndex:            1,
            tokenIndex:            1,
            newProcessedCharCount: 2
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 1,
            x:          24.0,
            y:          500.0,
            priority:   42.0
        );

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            2,
            tokenIndex:            2,
            newProcessedCharCount: 3
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 2,
            x:          48.0,
            y:          500.0,
            priority:   42.0
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box, 3)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_it_allows_limiting_the_characters_to_draw()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);

        $var_glyphTexnames_8c1bc78c = $this->alloc(0x200 * 0x0c);
        // $this->initUint16Array($var_glyphTexnames_8c1bc78c, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), $var_glyphTexnames_8c1bc78c);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 0x8);
        // $this->initUint16Array($var_glyphTexlists_8c1bc790, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_busFont_8c1ba1c8'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0002);

        [$box, $glyphIndexes, $lineOffsets] = $this->createTextbox(
            'ÇÅÇÇÇÉ',
            characterCount: 3,
            tagCount: 0,
            x: 0,
            y: 500,
        );

        $shouldLoadGlyphContext = [
            "box" => $box,
            "var_glyphTexnames_8c1bc78c" => $var_glyphTexnames_8c1bc78c,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
            "var_8c1bc7a0" => $var_8c1bc7a0,
            "glyphIndexes" => $glyphIndexes,
        ];

        $shouldDrawCharacterContext = [
            "box" => $box,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
        ];

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            0,
            tokenIndex:            0,
            newProcessedCharCount: 1
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 0,
            x:          0.0,
            y:          500.0,
            priority:   42.0
        );

        // Load and draw ÇÇ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8282,
            glyphIndex:            1,
            tokenIndex:            1,
            newProcessedCharCount: 2
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 1,
            x:          24.0,
            y:          500.0,
            priority:   42.0
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box, 2)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_it_handles_line_break_tags()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);

        $var_glyphTexnames_8c1bc78c = $this->alloc(0x200 * 0x0c);
        // $this->initUint16Array($var_glyphTexnames_8c1bc78c, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), $var_glyphTexnames_8c1bc78c);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 0x8);
        // $this->initUint16Array($var_glyphTexlists_8c1bc790, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_busFont_8c1ba1c8'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0002);

        [$box, $glyphIndexes] = $this->createTextbox(
            'ÇÅ<E>ÇÇ',
            characterCount: 2,
            tagCount: 1,
            x: 0,
            y: 500,
        );

        $shouldLoadGlyphContext = [
            "box" => $box,
            "var_glyphTexnames_8c1bc78c" => $var_glyphTexnames_8c1bc78c,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
            "var_8c1bc7a0" => $var_8c1bc7a0,
            "glyphIndexes" => $glyphIndexes,
        ];

        $shouldDrawCharacterContext = [
            "box" => $box,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
        ];

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            0,
            tokenIndex:            0,
            newProcessedCharCount: 1,
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 0,
            x:          0.0,
            y:          500.0,
            priority:   42.0,
        );

        // Parse line break tag
        $this->shouldWriteWord($glyphIndexes + 1 * 2, 0xfffe);
        $this->shouldWriteWord($box + 0x1e, 1);

        // Load and draw ÇÇ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8282,
            glyphIndex:            1,
            tokenIndex:            2,
            newProcessedCharCount: 2
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 1,
            x:          0.0,
            y:          532.0,
            priority:   42.0,
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box, 3)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_it_does_not_overflow_the_box_height()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);

        $var_glyphTexnames_8c1bc78c = $this->alloc(0x200 * 0x0c);
        // $this->initUint16Array($var_glyphTexnames_8c1bc78c, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), $var_glyphTexnames_8c1bc78c);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 0x8);
        // $this->initUint16Array($var_glyphTexlists_8c1bc790, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_busFont_8c1ba1c8'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0002);

        [$box, $glyphIndexes] = $this->createTextbox(
            'ÇÅ<E>ÇÇ',
            characterCount: 2,
            tagCount: 1,
            x: 0,
            y: 500,
            height: 32,
        );

        $shouldLoadGlyphContext = [
            "box" => $box,
            "var_glyphTexnames_8c1bc78c" => $var_glyphTexnames_8c1bc78c,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
            "var_8c1bc7a0" => $var_8c1bc7a0,
            "glyphIndexes" => $glyphIndexes,
        ];

        $shouldDrawCharacterContext = [
            "box" => $box,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
        ];

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            0,
            tokenIndex:            0,
            newProcessedCharCount: 1,
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 0,
            x:          0.0,
            y:          500.0,
            priority:   42.0,
        );

        // Parse line break tag
        $this->shouldWriteWord($glyphIndexes + 1 * 2, 0xfffe);
        $this->shouldWriteWord($box + 0x1e, 1);

        // Load but does not draw ÇÇ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8282,
            glyphIndex:            1,
            tokenIndex:            2,
            newProcessedCharCount: 2
        );
        $this->shouldWriteLong(
            $this->addressOf('_var_fontResourceGroup_8c1bc794') + 0x0,
            $var_glyphTexlists_8c1bc790 + 1 * 0x8
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box, 3)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_it_limits_correctly_when_a_line_break_tag_is_present()
    {
        $this->resolveSymbols();

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint16Array($var_8c1bc7a0, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);

        $var_glyphTexnames_8c1bc78c = $this->alloc(0x200 * 0x0c);
        // $this->initUint16Array($var_glyphTexnames_8c1bc78c, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexnames_8c1bc78c'), $var_glyphTexnames_8c1bc78c);

        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 0x8);
        // $this->initUint16Array($var_glyphTexlists_8c1bc790, array_fill(0, 0x200, 0xffff));
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->initUint32($this->addressOf('_var_busFont_8c1ba1c8'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_glyphBuffer_8c1bc7a4'), 0xcafe0002);

        [$box, $glyphIndexes] = $this->createTextbox(
            'ÇÅ<E>ÇÇÇÉ',
            characterCount: 2,
            tagCount: 1,
            x: 0,
            y: 500,
        );

        $shouldLoadGlyphContext = [
            "box" => $box,
            "var_glyphTexnames_8c1bc78c" => $var_glyphTexnames_8c1bc78c,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
            "var_8c1bc7a0" => $var_8c1bc7a0,
            "glyphIndexes" => $glyphIndexes,
        ];

        $shouldDrawCharacterContext = [
            "box" => $box,
            "var_glyphTexlists_8c1bc790" => $var_glyphTexlists_8c1bc790,
        ];

        // Load and draw ÇÅ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8281,
            glyphIndex:            0,
            tokenIndex:            0,
            newProcessedCharCount: 1,
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 0,
            x:          0.0,
            y:          500.0,
            priority:   42.0,
        );

        // Parse line break tag
        $this->shouldWriteWord($glyphIndexes + 1 * 2, 0xfffe);
        $this->shouldWriteWord($box + 0x1e, 1);

        // Load and draw ÇÇ
        $this->shouldLoadGlyph(
            ...$shouldLoadGlyphContext,
            charCode:              0x8282,
            glyphIndex:            1,
            tokenIndex:            2,
            newProcessedCharCount: 2
        );
        $this->shouldDrawCharacter(
            ...$shouldDrawCharacterContext,
            glyphIndex: 1,
            x:          0.0,
            y:          532.0,
            priority:   42.0,
        );

        $this->singleCall('_TxtDrawTextbox_8c0155e0')
            ->with($box, 3)
            ->singleShouldReturn(1)
            ->run();
    }

    protected function shouldLoadGlyph (
        int $charCode,
        int $glyphIndex,
        int $tokenIndex,
        int $newProcessedCharCount,

        $box,
        $var_glyphTexnames_8c1bc78c,
        $var_glyphTexlists_8c1bc790,
        $var_8c1bc7a0,
        $glyphIndexes,
    ) {
        $texInfoLocal = $this->isAsmObject() ? 0xffffa8 : 0xffffac;

        $this->shouldCall('_unpackGlyph_8c015110')
            ->with(
                $charCode,
                $box + 0x24,
                0xcafe0001,
                0xcafe0002,
            );

        $this->shouldCall('_njSetTextureInfo')
            ->with($texInfoLocal, 0xcafe0002, 0x100, 32, 32);

        $this->shouldCall('_njSetTextureName')
            ->with(
                $var_glyphTexnames_8c1bc78c + $glyphIndex * 0xc,
                $texInfoLocal,
                $glyphIndex,
                0x40800000,
            );
        $this->shouldWriteLong(
            $var_glyphTexlists_8c1bc790 + $glyphIndex * 0x8 + 0x00,
            $var_glyphTexnames_8c1bc78c + $glyphIndex * 0xc,
        );
        $this->shouldWriteLong(
            $var_glyphTexlists_8c1bc790 + $glyphIndex * 0x8 + 0x04, 1
        );

        $this->shouldWriteWord(
            $var_8c1bc7a0 + $glyphIndex * 0x2, $glyphIndex
        );
        $this->shouldWriteWord(
            $glyphIndexes + $tokenIndex * 0x2, $glyphIndex
        );

        $this->shouldCall('_njLoadTexture')
            ->with($var_glyphTexlists_8c1bc790 + $glyphIndex * 0x8);

        $this->shouldWriteWord($box + 0x1c, $newProcessedCharCount);
    }

    protected function shouldDrawCharacter (
        int $glyphIndex,
        float $x,
        float $y,
        float $priority,

        $box,
        $var_glyphTexlists_8c1bc790,
    ) {
        $this->shouldWriteLong(
            $this->addressOf('_var_fontResourceGroup_8c1bc794') + 0x0,
            $var_glyphTexlists_8c1bc790 + $glyphIndex * 0x8,
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_var_fontResourceGroup_8c1bc794'),
            2000,
            $x,
            $y,
            $priority,
        );
    }

    protected function resolveSymbols(): void
    {
        $this->setSize('_var_busFont_8c1ba1c8', 4);
        // Functions
        $this->setSize('_njSetTextureInfo', 4);
        $this->setSize('_njSetTextureName', 4);
        $this->setSize('_njLoadTexture', 4);
    }

    private function createTextbox(
        $textContent,
        $characterCount,
        $tagCount,
        $x,
        $y,
        $width = 24 * 24,
        $height = 32 * 2,
        $x2 = 0,
        $y2 = 0,
        $priority = 42.0,
    ) {
        $text = $this->allocString($textContent);
        $box = $this->alloc(0x3c);

        // Initialize box properties
        $this->initUint32($box + 0x00, $x); // x
        $this->initUint32($box + 0x04, $y); // y
        $this->initUint32($box + 0x08, fdec($priority)); // priority
        $this->initUint32($box + 0x0c, $width); // width
        $this->initUint32($box + 0x10, $height); // height
        $this->initUint32($box + 0x14, $x2);
        $this->initUint32($box + 0x18, $y2);
        $this->initUint16($box + 0x1c, 0); // processed_char_count_0x1c
        $this->initUint16($box + 0x1e, 0); // processed_tag_count_0x1e
        $this->initUint16($box + 0x20, $characterCount);
        $this->initUint16($box + 0x22, $tagCount);

        // Allocate glyph indexes
        $glyphIndexes = $this->alloc($characterCount * 2);
        $this->initUint32($box + 0x2c, $glyphIndexes);

        // Allocate line offsets and initialize
        $this->initUint32($box + 0x30, -1); // Enable offsets
        $lineOffsets = $this->alloc((int) ($height * 4 / 32));
        $this->initUint32Array($lineOffsets, array_fill(0, (int) ($height * 4 / 32), fdec(0)));
        $this->initUint32($box + 0x34, $lineOffsets);

        // Set text reference
        $this->initUint32($box + 0x38, $text);

        return [$box, $glyphIndexes, $lineOffsets];
    }

    protected function initUint16Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint16($address + $i * 2, $value);
        }
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
