<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_clears_flag()
    {
        $this->call('_resetUknPvmBool_8c014322');

        $this->shouldWriteLong($this->addressOf('_var_8c18adac'), 0);
    }
};
