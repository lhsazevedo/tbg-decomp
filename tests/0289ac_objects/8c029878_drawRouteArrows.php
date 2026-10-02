<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_drawCamera_8c226558', 4);
        $this->setSize('__modls', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_njCnkSimpleDrawObject', 4);
    }

    public function test_phase0_draws_all_matrices(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 2); // count
        $this->initUint32($task + 0x0c, 0); // frame counter (>>2 == 0, phase 0)

        $camera = $this->addressOf('_var_drawCamera_8c226558');
        $this->initUint32($camera, 0x11111111);

        $root = $this->alloc(0x34);
        $node1 = $this->alloc(0x34);
        $node2 = $this->alloc(0x34);
        $node3 = $this->alloc(0x34);
        $this->initUint32($node1, 0); // evalflags
        $this->initUint32($node2, 0);
        $this->initUint32($node3, 0);

        $nodes = $this->addressOf('_var_routeArrowNodes_8c228278');
        $this->initUint32($nodes + 0, $root);
        $this->initUint32($nodes + 4, $node1);
        $this->initUint32($nodes + 8, $node2);
        $this->initUint32($nodes + 12, $node3);

        $matrix0 = $this->alloc(0x40);
        $matrix1 = $matrix0 + 0x40;

        $this->call('_drawRouteArrows_8c029878')->with($task, $matrix0);

        $this->shouldCall('__modls')->with(0, 3)->using(new RiroCallingConvention())->andReturn(0);
        $this->shouldWriteLong($node1, 0x3f);
        $this->shouldWriteLong($node2, 0x3f);
        $this->shouldWriteLong($node3, 0x37);
        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix0);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($root);
        $this->shouldCall('_njSetCamera')->with(0x11111111);
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix1);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($root);
    }

    public function test_phase1_and_zero_count(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0); // count
        $this->initUint32($task + 0x0c, 4); // frame counter (>>2 == 1, phase 1)

        $root = $this->alloc(0x34);
        $node1 = $this->alloc(0x34);
        $node2 = $this->alloc(0x34);
        $node3 = $this->alloc(0x34);
        $this->initUint32($node1, 0);
        $this->initUint32($node2, 0);
        $this->initUint32($node3, 0);

        $nodes = $this->addressOf('_var_routeArrowNodes_8c228278');
        $this->initUint32($nodes + 0, $root);
        $this->initUint32($nodes + 4, $node1);
        $this->initUint32($nodes + 8, $node2);
        $this->initUint32($nodes + 12, $node3);

        $matrices = $this->alloc(0x40);

        $this->call('_drawRouteArrows_8c029878')->with($task, $matrices);

        $this->shouldCall('__modls')->with(1, 3)->using(new RiroCallingConvention())->andReturn(1);
        $this->shouldWriteLong($node1, 0x3f);
        $this->shouldWriteLong($node2, 0x37);
        $this->shouldWriteLong($node3, 0x3f);
    }

    public function test_phase2(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0); // count
        $this->initUint32($task + 0x0c, 8); // frame counter (>>2 == 2, phase 2)

        $root = $this->alloc(0x34);
        $node1 = $this->alloc(0x34);
        $node2 = $this->alloc(0x34);
        $node3 = $this->alloc(0x34);
        $this->initUint32($node1, 0);
        $this->initUint32($node2, 0);
        $this->initUint32($node3, 0);

        $nodes = $this->addressOf('_var_routeArrowNodes_8c228278');
        $this->initUint32($nodes + 0, $root);
        $this->initUint32($nodes + 4, $node1);
        $this->initUint32($nodes + 8, $node2);
        $this->initUint32($nodes + 12, $node3);

        $matrices = $this->alloc(0x40);

        $this->call('_drawRouteArrows_8c029878')->with($task, $matrices);

        $this->shouldCall('__modls')->with(2, 3)->using(new RiroCallingConvention())->andReturn(2);
        $this->shouldWriteLong($node1, 0x37);
        $this->shouldWriteLong($node2, 0x3f);
        $this->shouldWriteLong($node3, 0x3f);
    }
};
