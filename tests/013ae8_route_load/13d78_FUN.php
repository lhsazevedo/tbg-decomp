<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_inits_queues_syncs_models_and_starts_pass()
    {
        $this->setSize('_AsqInitQueues_11f36', 4);
        $this->setSize('_AsqResetQueues_11f6c', 4);
        $this->setSize('_AsqProcessQueues_11fe0', 4);
        $this->setSize('_AsqNop_11120', 4);

        // LoadedModel list read from the global and forwarded to the sync pass.
        $models = 0x8cabc000;
        $this->initUint32($this->addressOf('_var_routeModelIndexes_8c18adb0'), $models);

        $this->call('_startRouteModelLoadPass_8c013d78');

        $this->shouldCall('_AsqInitQueues_11f36')->with(0, 0x40, 0, 0x40);
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_syncRouteModelAssets_8c013c34')->with($models);
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0, 0, 0,
            $this->addressOf('_finishAssetLoad_8c013d42'),
        );
    }
};
