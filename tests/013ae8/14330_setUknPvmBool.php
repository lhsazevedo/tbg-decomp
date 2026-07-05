<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_sets_flag()
    {
        $this->call('_setUknPvmBool_8c014330');

        $this->shouldWriteLong($this->addressOf('_var_8c18adac'), 1);
    }
};
