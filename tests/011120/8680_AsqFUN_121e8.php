<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_itFollowsPath_0_0_0_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_1_0_0_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  0, 4);
        $this->shouldWriteOffset3be80(2 *  3, 0x400);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_2_0_0_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 2);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  1, 4);
        $this->shouldWriteOffset3be80(2 *  3, 2);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_1_0_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160); 

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 7, 0x4);
        $this->shouldWriteOffset3be80(2 * 10, 0x400);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_2_0_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 2);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160); 

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 8, 0x4);
        $this->shouldWriteOffset3be80(2 * 10, 0x2);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_0_1_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x02);
        $this->shouldWriteOffset3be80(2 * 15, 0x04);
        $this->shouldWriteOffset3be80(2 * 16, 0x20);
        $this->shouldWriteOffset3be80(2 * 17, 0x10);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_0_2_0()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 2);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 0);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_0_0_1()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 1);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x02);
        $this->shouldWriteOffset3be80(2 * 20, 0x04);

        $this->shouldWriteOffset3be80(2 * 21, 0x20);
        $this->shouldWriteOffset3be80(2 * 22, 0x10);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_0_0_2()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 2);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x40);
        $this->shouldWriteOffset3be80(2 * 20, 0x80);

        $this->shouldWriteOffset3be80(2 * 21, 0x20);
        $this->shouldWriteOffset3be80(2 * 22, 0x10);
        
        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    public function test_itFollowsPath_0_0_0_3()
    {
        $this->resolveImports();

        // Random values
        $this->init3be80(0, 175);
        $this->init3be80(1, 113);
        $this->init3be80(2, 40);
        $this->init3be80(3, 145);
        $this->init3be80(4, 16);
        $this->init3be80(5, 133);
        $this->init3be80(6, 208);
        $this->init3be80(7, 219);
        $this->init3be80(8, 151);
        $this->init3be80(9, 47);
        $this->init3be80(10, 112);
        $this->init3be80(11,  59);
        $this->init3be80(12, 186);
        $this->init3be80(13, 160);

        $this->init3be80(14, 196);
        $this->init3be80(15, 125);
        $this->init3be80(16, 88);
        $this->init3be80(17, 216);
        $this->init3be80(18, 213);
        $this->init3be80(19, 211);
        $this->init3be80(20, 154);
        $this->init3be80(21, 250);
        $this->init3be80(22, 228);
        $this->init3be80(23, 124);
        $this->init3be80(24, 136);
        $this->init3be80(25,  52);
        $this->init3be80(26, 55);
        $this->init3be80(27, 222);

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcc, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcd, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xce, 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xcf, 3);

        $this->shouldWriteOffset3be80(2 *  0, 113);
        $this->shouldWriteOffset3be80(2 *  1, 145);
        $this->shouldWriteOffset3be80(2 *  2, 133);
        $this->shouldWriteOffset3be80(2 *  3, 219);
        $this->shouldWriteOffset3be80(2 *  4,  47);
        $this->shouldWriteOffset3be80(2 *  5,  59);
        $this->shouldWriteOffset3be80(2 *  6, 160);

        $this->shouldWriteOffset3be80(2 *  7, 125);
        $this->shouldWriteOffset3be80(2 *  8, 216);
        $this->shouldWriteOffset3be80(2 *  9, 211);
        $this->shouldWriteOffset3be80(2 * 10, 250);
        $this->shouldWriteOffset3be80(2 * 11, 124);
        $this->shouldWriteOffset3be80(2 * 12,  52);
        $this->shouldWriteOffset3be80(2 * 13, 222);

        $this->shouldWriteOffset3be80(2 * 11, 0x40);
        $this->shouldWriteOffset3be80(2 * 12, 0x80);

        $this->shouldWriteOffset3be80(2 * 14, 0x20);
        $this->shouldWriteOffset3be80(2 * 15, 0x10);
        $this->shouldWriteOffset3be80(2 * 16, 0x02);
        $this->shouldWriteOffset3be80(2 * 17, 0x04);

        $this->shouldWriteOffset3be80(2 * 18, 8);
        
        $this->shouldWriteOffset3be80(2 * 19, 0x20);
        $this->shouldWriteOffset3be80(2 * 20, 0x10);
        $this->shouldWriteOffset3be80(2 * 21, 0x02);
        $this->shouldWriteOffset3be80(2 * 22, 0x04);

        $this->shouldWriteOffset3be80(2 * 23, 8);

        $this->singleCall('_AsqFUN_121e8')->run();
    }

    private function init3be80(int $offset, int $value): void
    {
        $this->initUint32($this->addressOf('_init_8c03be80') + $offset * 4, $value);
    }

    private function shouldWriteOffset3be80(int $offset, int $value): void
    {
        $this->shouldWrite($this->addressOf('_init_8c03be80') + $offset * 4, $value);
    }

    private function resolveImports(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd0);
    }
};
