<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_handlesCharCodes()
    {
        $this->resolveSymbols();

        $GLYPH_COUNT = 0x28 + (0x240 / 0x18) * (0x40 / 0x20);

        $glyphIndexes = $this->alloc($GLYPH_COUNT * 2);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, 0x240); // Width
        $this->initUint32($box + 0x10, 0x40); // Height
        $this->initUint16($box + 0x20, 2);
        $this->initUint16($box + 0x22, 2);

        $this->initUint32($box + 0x2c, $glyphIndexes);
        $this->initUint16($glyphIndexes + 0x00, 0);
        $this->initUint16($glyphIndexes + 0x02, 0xffed);
        $this->initUint16($glyphIndexes + 0x04, 42);
        $this->initUint16($glyphIndexes + 0x06, 69);

        $text = $this->allocString('');

        $var_8c1bc7a0 = $this->alloc(0x200 * 2);
        $this->initUint32($this->addressOf('_var_8c1bc7a0'), $var_8c1bc7a0);
        $var_glyphTexlists_8c1bc790 = $this->alloc(0x200 * 8);
        $this->initUint32($this->addressOf('_var_glyphTexlists_8c1bc790'), $var_glyphTexlists_8c1bc790);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 0 * 4);
        $this->shouldWriteWord($var_8c1bc7a0 + 0 * 2, -1);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 42 * 8);
        $this->shouldWriteWord($var_8c1bc7a0 + 42 * 2, -1);

        $this->shouldCall('_njReleaseTexture')->with($var_glyphTexlists_8c1bc790 + 69 * 8);
        $this->shouldWriteWord($var_8c1bc7a0 + 69 * 2, -1);

        for ($i = 0; $i < $GLYPH_COUNT; $i++) {
            $this->shouldWriteWord($glyphIndexes + $i * 2, 0xffff);
        }

        $this->shouldWriteLong($box + 0x38, $text);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_handlesEmptyString()
    {
        $this->resolveSymbols();

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $this->initUint16($glyphIndexes, -1);

        $text = $this->allocString('');

        $this->shouldWriteLong($box + 0x38, $text);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_processBasic()
    {
        $this->resolveSymbols();

        $WIDTH = 0x240;
        $HEIGHT = 0x40;

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, $WIDTH);
        $this->initUint32($box + 0x10, $HEIGHT);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $lineOffsets = $this->alloc((int) ($HEIGHT * 4 / 32));
        $this->initUint32($box + 0x34, $lineOffsets);
        $this->initUint16($glyphIndexes, -1);

        // Note that these are wide characters
        $textStr = 'Ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ';
        $textLen = strlen($textStr);

        $text = $this->allocString($textStr);

        // Tag count
        $this->shouldWriteWord($box + 0x22, 0);

        $this->shouldCall('_strlen')->with($text)->do(function ($params) use ($text) {
            $this->registers[0] = U32::of(strlen($this->memory->readString($text)));
        });

        $this->shouldWriteLong($box + 0x38, $text);
        $this->shouldWriteWord($box + 0x1c, 0);
        $this->shouldWriteWord($box + 0x1e, 0);
        $this->shouldWriteWord($box + 0x20, (int) ($textLen / 2));

        $this->shouldWriteFloat($lineOffsets + 0, 0.0);
        $this->shouldWriteFloat($lineOffsets + 4, 0.0);

        for ($i = 1; $i <= 38; $i++) {
            $this->shouldWriteFloat($lineOffsets + 0 * 4, (float) $i);
        }

        $this->shouldWriteFloat($lineOffsets + 0 * 4, 2.5);
        $this->shouldWriteFloat($lineOffsets + 1 * 4, 12.0);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(19)
            ->run();
    }

    public function test_processLineBreak()
    {
        $this->resolveSymbols();

        $WIDTH = 0x240;
        $HEIGHT = 0x40;

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, $WIDTH);
        $this->initUint32($box + 0x10, $HEIGHT);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $lineOffsets = $this->alloc((int) ($HEIGHT * 4 / 32));
        $this->initUint32($box + 0x34, $lineOffsets);
        $this->initUint16($glyphIndexes, -1);

        // Note that these are wide characters
        $textStr = 'Ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　<E>　ｊｕｍｐｓ　ｏｖｅｒ　ｔｈｅ　ｌａｚｙ　ｄｏｇ';
        $textLen = strlen($textStr);

        $text = $this->allocString($textStr);

        // Tag count
        $this->shouldWriteWord($box + 0x22, 0);
        $this->shouldWriteWord($box + 0x22, 1);

        $this->shouldCall('_strlen')->with($text)->do(function ($params) use ($text) {
            $this->registers[0] = U32::of(strlen($this->memory->readString($text)));
        });

        $this->shouldWriteLong($box + 0x38, $text);
        $this->shouldWriteWord($box + 0x1c, 0);
        $this->shouldWriteWord($box + 0x1e, 0);
        $this->shouldWriteWord($box + 0x20, (int) (($textLen - 3) / 2));

        $this->shouldWriteFloat($lineOffsets + 0, 0.0);
        $this->shouldWriteFloat($lineOffsets + 4, 0.0);

        for ($i = 1; $i <= 40; $i++) {
            $this->shouldWriteFloat($lineOffsets + 0 * 4, (float) $i);
        }

        for ($i = 1; $i <= 48; $i++) {
            $this->shouldWriteFloat($lineOffsets + 1 * 4, (float) $i);
        }

        $this->shouldWriteFloat($lineOffsets + 0 * 4, 2.0);
        $this->shouldWriteFloat($lineOffsets + 1 * 4, 0.0);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(44)
            ->run();
    }

    public function test_processExceedingLineBreaks()
    {
        $this->resolveSymbols();

        $WIDTH = 0x240;
        $HEIGHT = 0x40;

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, $WIDTH);
        $this->initUint32($box + 0x10, $HEIGHT);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $lineOffsets = $this->alloc((int) ($HEIGHT * 4 / 32));
        $this->initUint32($box + 0x34, $lineOffsets);
        $this->initUint16($glyphIndexes, -1);

        // Note that these are wide characters
        $textStr = 'Ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　<E>　ｆｏｘ　ｊｕｍｐｓ　ｏｖｅｒ　<E>　ｔｈｅ　ｌａｚｙ　ｄｏｇ';
        $textLen = strlen($textStr);

        $text = $this->allocString($textStr);

        // Tag count
        $this->shouldWriteWord($box + 0x22, 0);
        $this->shouldWriteWord($box + 0x22, 1);
        $this->shouldWriteWord($box + 0x22, 2);

        $this->shouldCall('_strlen')->with($text)->do(function ($params) use ($text) {
            $this->registers[0] = U32::of(strlen($this->memory->readString($text)));
        });

        $this->shouldWriteLong($box + 0x38, $text);
        $this->shouldWriteWord($box + 0x1c, 0);
        $this->shouldWriteWord($box + 0x1e, 0);
        $this->shouldWriteWord($box + 0x20, (int) (($textLen - 6) / 2));

        $this->shouldWriteFloat($lineOffsets + 0, 0.0);
        $this->shouldWriteFloat($lineOffsets + 4, 0.0);

        for ($i = 1; $i <= 32; $i++) {
            $this->shouldWriteFloat($lineOffsets + 0 * 4, (float) $i);
        }

        for ($i = 1; $i <= 32; $i++) {
            $this->shouldWriteFloat($lineOffsets + 1 * 4, (float) $i);
        }

        for ($i = 1; $i <= 26; $i++) {
            $this->shouldWriteFloat($lineOffsets + 2 * 4, (float) $i);
        }

        $this->shouldWriteFloat($lineOffsets + 0 * 4, 4.0);
        $this->shouldWriteFloat($lineOffsets + 1 * 4, 4.0);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(45)
            ->run();
    }

    public function test_processReallyExceedingLineBreaks()
    {
        $this->resolveSymbols();

        $WIDTH = 0x240;
        $HEIGHT = 0x40;

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, $WIDTH);
        $this->initUint32($box + 0x10, $HEIGHT);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $lineOffsets = $this->alloc((int) ($HEIGHT * 4 / 32));
        $this->initUint32($box + 0x34, $lineOffsets);
        $this->initUint16($glyphIndexes, -1);

        // Note that these are wide characters
        $textStr = 'Ｔｈｅ　ｑｕｉｃｋ　<E>　ｂｒｏｗｎ　ｆｏｘ　<E>　ｊｕｍｐｓ　ｏｖｅｒ　<E>　ｔｈｅ　ｌａｚｙ　ｄｏｇ';
        $textLen = strlen($textStr);

        $text = $this->allocString($textStr);

        // Tag count
        $this->shouldWriteWord($box + 0x22, 0);
        $this->shouldWriteWord($box + 0x22, 1);
        $this->shouldWriteWord($box + 0x22, 2);
        $this->shouldWriteWord($box + 0x22, 3);

        $this->shouldCall('_strlen')->with($text)->do(function ($params) use ($text) {
            $this->registers[0] = U32::of(strlen($this->memory->readString($text)));
        });

        $this->shouldWriteLong($box + 0x38, $text);
        $this->shouldWriteWord($box + 0x1c, 0);
        $this->shouldWriteWord($box + 0x1e, 0);
        $this->shouldWriteWord($box + 0x20, (int) (($textLen - 9) / 2));

        $this->shouldWriteFloat($lineOffsets + 0, 0.0);
        $this->shouldWriteFloat($lineOffsets + 4, 0.0);

        for ($i = 1; $i <= 20; $i++) {
            $this->shouldWriteFloat($lineOffsets + 0 * 4, (float) $i);
        }

        for ($i = 1; $i <= 22; $i++) {
            $this->shouldWriteFloat($lineOffsets + 1 * 4, (float) $i);
        }

        for ($i = 1; $i <= 24; $i++) {
            $this->shouldWriteFloat($lineOffsets + 2 * 4, (float) $i);
        }

        $this->shouldWriteFloat($lineOffsets + 0 * 4, 7.0);
        $this->shouldWriteFloat($lineOffsets + 1 * 4, 6.5);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(46)
            ->run();
    }

    public function test_processLineWrap()
    {
        $this->resolveSymbols();

        $WIDTH = 0x240;
        $HEIGHT = 0x40;

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, $WIDTH);
        $this->initUint32($box + 0x10, $HEIGHT);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $lineOffsets = $this->alloc((int) ($HEIGHT * 4 / 32));
        $this->initUint32($box + 0x34, $lineOffsets);
        $this->initUint16($glyphIndexes, -1);

        // Note that these are wide characters
        $textStr = 'Ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐｓ　ｏｖｅｒ　ｔｈｅ　ｌａｚｙ　ｄｏｇ';
        $textLen = strlen($textStr);

        $text = $this->allocString($textStr);

        // Tag count
        $this->shouldWriteWord($box + 0x22, 0);

        $this->shouldCall('_strlen')->with($text)->do(function ($params) use ($text) {
            $this->registers[0] = U32::of(strlen($this->memory->readString($text)));
        });

        $this->shouldWriteLong($box + 0x38, $text);
        $this->shouldWriteWord($box + 0x1c, 0);
        $this->shouldWriteWord($box + 0x1e, 0);
        $this->shouldWriteWord($box + 0x20, (int) ($textLen / 2));

        $this->shouldWriteFloat($lineOffsets + 0, 0.0);
        $this->shouldWriteFloat($lineOffsets + 4, 0.0);

        for ($i = 1; $i <= 48; $i++) {
            $this->shouldWriteFloat($lineOffsets + 0 * 4, (float) $i);
        }

        for ($i = 1; $i <= 38; $i++) {
            $this->shouldWriteFloat($lineOffsets + 1 * 4, (float) $i);
        }

        $this->shouldWriteFloat($lineOffsets + 0 * 4, 0);
        $this->shouldWriteFloat($lineOffsets + 1 * 4, 2.5);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(43)
            ->run();
    }

    public function test_processExceedingLineWraps()
    {
        $this->resolveSymbols();

        $WIDTH = 0x240;
        $HEIGHT = 0x40;

        $glyphIndexes = $this->alloc(4);
        $box = $this->alloc(0x3c);
        $this->initUint32($box + 0x0c, $WIDTH);
        $this->initUint32($box + 0x10, $HEIGHT);
        $this->initUint32($box + 0x2c, $glyphIndexes);
        $lineOffsets = $this->alloc((int) ($HEIGHT * 4 / 32));
        $this->initUint32($box + 0x34, $lineOffsets);
        $this->initUint16($glyphIndexes, -1);

        // Note that these are wide characters
        $textStr = 'Ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐＴｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐＴｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐ';
        $textLen = strlen($textStr);

        $text = $this->allocString($textStr);

        // Tag count
        $this->shouldWriteWord($box + 0x22, 0);

        $this->shouldCall('_strlen')->with($text)->do(function ($params) use ($text) {
            $this->registers[0] = U32::of(strlen($this->memory->readString($text)));
        });

        $this->shouldWriteLong($box + 0x38, $text);
        $this->shouldWriteWord($box + 0x1c, 0);
        $this->shouldWriteWord($box + 0x1e, 0);
        $this->shouldWriteWord($box + 0x20, (int) ($textLen / 2));

        $this->shouldWriteFloat($lineOffsets + 0, 0.0);
        $this->shouldWriteFloat($lineOffsets + 4, 0.0);

        for ($i = 1; $i <= 48; $i++) {
            $this->shouldWriteFloat($lineOffsets + 0 * 4, (float) $i);
        }

        for ($i = 1; $i <= 48; $i++) {
            $this->shouldWriteFloat($lineOffsets + 1 * 4, (float) $i);
        }

        for ($i = 1; $i <= 48; $i++) {
            $this->shouldWriteFloat($lineOffsets + 2 * 4, (float) $i);
        }

        $this->shouldWriteFloat($lineOffsets + 0 * 4, 0.0);
        $this->shouldWriteFloat($lineOffsets + 1 * 4, 0.0);

        $this->singleCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->with($box, $text)
            ->singleShouldReturn(72)
            ->run();
    }

    protected function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_njReleaseTexture', 4);
        $this->setSize('__divls', 4);
        $this->setSize('_strlen', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    protected function initUint16Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint16($address + $i * 2, $value);
        }
    }
};
