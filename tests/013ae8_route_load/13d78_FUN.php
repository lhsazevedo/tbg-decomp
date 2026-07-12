<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_inits_queues_syncs_models_and_starts_pass()
    {
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_AsqNop_8c011120', 4);

        // LoadedModel list read from the global and forwarded to the sync pass.
        $models = 0x8cabc000;
        $this->initUint32($this->addressOf('_var_routeModelIndexes_8c18adb0'), $models);

        $this->call('_startRouteModelLoadPass_8c013d78');

        $this->shouldCall('_AsqInitQueues_8c011f36')->with(0, 0x40, 0, 0x40);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_syncRouteModelAssets_8c013c34')->with($models);
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0, 0, 0,
            $this->addressOf('_finishAssetLoad_8c013d42'),
        );
    }
};
