<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_requests_route_assets()
    {
        $this->resolveSymbols();

        $basedir = $this->addressOf('_var_basedir_8c18ad6c');

        $this->call('_requestVehicleAssets_8c013ae8');

        $this->shouldCall('_AsqRequestNj_11492')->with(
            $basedir, "front.njd", $this->addressOf('_var_frontNj_8c1bc434'), 0
        );
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            $basedir, "front.pvm", $this->addressOf('_var_frontTexlist_8c1bc430'), 0xf, 0
        );
        $this->shouldCall('_AsqRequestNj_11492')->with(
            $basedir, "syanai.njd", $this->addressOf('_var_interiorNj_8c1bc43c'), 0
        );
        // attr H'80000000 (Ghidra misreads this as 0)
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            $basedir, "syanai.pvm", $this->addressOf('_var_interiorTexlist_8c1bc438'), 0x40, 0x80000000
        );
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            $basedir, "mark.pvm", $this->addressOf('_var_markTexlist_8c1bc418'), 3, 0
        );
        $this->shouldCall('_AsqRequestPvm_11ac0')->with(
            $basedir, "busstop.pvm", $this->addressOf('_var_busStopTexlist_8c1bc424'), 1, 0
        );

        $this->shouldCall('_AsqRequestNjPvmPairs_12030')->with(
            $basedir, $this->addressOf('_init_routeModelFilenames_8c043d64'), 0
        )->andReturn(0x8c123456);
        $this->shouldWriteLongTo('_var_routeModelPairs_8c1bc3f4', 0x8c123456);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_frontNj_8c1bc434', 4);
        $this->setSize('_var_frontTexlist_8c1bc430', 4);
        $this->setSize('_var_interiorNj_8c1bc43c', 4);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_markTexlist_8c1bc418', 4);
        $this->setSize('_var_busStopTexlist_8c1bc424', 4);
        $this->setSize('_var_routeModelPairs_8c1bc3f4', 4);
    }
};
