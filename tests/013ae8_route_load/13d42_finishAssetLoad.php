<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_sets_ready_flag_then_frees_queues()
    {
        $this->setSize('_AsqFreeQueues_8c011f7e', 4);

        $this->call('_finishAssetLoad_8c013d42');

        $this->shouldCall('_setPvmReady_8c014330');
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
    }
};
