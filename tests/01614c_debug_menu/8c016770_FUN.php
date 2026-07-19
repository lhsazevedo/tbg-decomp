<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resets_demo_recording_state()
    {
        $this->setSize('_var_demoCursor_8c225fa8', 4);
        $this->setSize('_var_demoPrevOn_8c225fac', 4);
        $this->setSize('_var_demoBuffer_8c1bc828', 4);

        $this->call('_FUN_8c016770');

        $this->shouldWriteLongTo('_var_demoCursor_8c225fa8', $this->addressOf('_var_demoBuffer_8c1bc828'));
        $this->shouldWriteLongTo('_var_demoPrevOn_8c225fac', 0);
    }
};
