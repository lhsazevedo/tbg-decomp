<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // Buffer already equals "FortyFive": strcmp returns 0, no copy, return 0.
    public function test_returns_0_when_already_set()
    {
        $this->call('_setName_8c012984');

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($this->addressOf('_var_name_8c157aec'), "FortyFive")
            ->andReturn(0);
        $this->shouldReturn(0);
    }

    // Buffer differs: strcmp returns nonzero, strcpy writes the name, return 1.
    public function test_sets_name_and_returns_1_when_different()
    {
        $this->call('_setName_8c012984');

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($this->addressOf('_var_name_8c157aec'), "FortyFive")
            ->andReturn(1);

        $strCpy = $this->isAsmObject() ? '_strcpy' : '__slow_strcpy';
        $this->shouldCall($strCpy)
            ->with($this->addressOf('_var_name_8c157aec'), "FortyFive");
        $this->shouldReturn(1);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
