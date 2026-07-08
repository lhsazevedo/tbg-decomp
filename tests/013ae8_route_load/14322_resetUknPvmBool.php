<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_clears_flag()
    {
        $this->call('_resetPvmReady_8c014322');

        $this->shouldWriteLong($this->addressOf('_var_pvmReady_8c18adac'), 0);
    }
};
