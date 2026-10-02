<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    private function f(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    // Task offsets used by pedGroupTask_8c029078.
    const OFF_GROUP_INDEX = 0x08;
    const OFF_RADIUS = 0x10;
    const OFF_SUBTASKS = 0x18;
    const OFF_SPEC = 0x1c;

    // PedGroupEntry offsets.
    const OFF_GRP_ACTIVE = 0x00;
    const OFF_GRP_WANTED = 0x04;

    // PedGroupSpawnSpec offsets (0xc bytes per entry).
    const OFF_SPEC_KINDID = 0x00;
    const OFF_SPEC_KIND = 0x02;
    const OFF_SPEC_REVERSE = 0x03;
    const OFF_SPEC_X = 0x04;
    const OFF_SPEC_Z = 0x08;
    const SPEC_SIZE = 0x0c;

    private function makeTask(int $groupIndex, int $subTasks, int $spec, float $radius = 0.0): int
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + self::OFF_GROUP_INDEX, $groupIndex);
        $this->initUint32($task + self::OFF_RADIUS, $this->f($radius));
        $this->initUint32($task + self::OFF_SUBTASKS, $subTasks);
        $this->initUint32($task + self::OFF_SPEC, $spec);
        return $task;
    }

    /** Allocates a single-entry group table at index 0 and seeds `wanted`. */
    private function makeGroup(int $wanted): int
    {
        $groups = $this->alloc(0xc);
        $this->initUint32($groups + self::OFF_GRP_ACTIVE, 1);
        $this->initUint32($groups + self::OFF_GRP_WANTED, $wanted);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);
        return $groups;
    }

    private function makePath(float $length): int
    {
        $paths = $this->alloc(0xc);
        $this->initUint32($paths + 0x00, 0);
        $this->initUint32($paths + 0x04, 0);
        $this->initUint32($paths + 0x08, $this->f($length));
        $this->initUint32($this->addressOf('_var_pedPaths_8c228238'), $paths);
        return $paths;
    }

    private function makeSpec(int $kindId, int $kind, int $reverse, float $x, float $z): int
    {
        $spec = $this->alloc(self::SPEC_SIZE);
        $this->initUint16($spec + self::OFF_SPEC_KINDID, $kindId);
        $this->initUint8($spec + self::OFF_SPEC_KIND, $kind);
        $this->initUint8($spec + self::OFF_SPEC_REVERSE, $reverse);
        $this->initUint32($spec + self::OFF_SPEC_X, $this->f($x));
        $this->initUint32($spec + self::OFF_SPEC_Z, $this->f($z));
        return $spec;
    }

    public function test_group_no_longer_wanted_tears_down()
    {
        $groups = $this->makeGroup(0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, 0);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->shouldWriteLong($groups + self::OFF_GRP_ACTIVE, 0);
        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($subTasks);
        $this->shouldCall('_syFree')->with($subTasks);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_sentinel_spec_skips_straight_to_exec_group()
    {
        $this->makeGroup(1);
        $spec = $this->makeSpec(0xffff, 0, 0, 0.0, 0.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_distance_beyond_path_length_skips_and_advances()
    {
        $this->makeGroup(1);
        $this->makePath(5.0);
        $spec = $this->makeSpec(1, 0, 0, 10.0, 0.0); // 10.0 > path length 5.0
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_task_install_failure_advances_without_spawning()
    {
        $this->makeGroup(1);
        $this->makePath(100.0);
        $spec = $this->makeSpec(1, 0, 0, 0.0, 0.0); // distance 0.0 -> always processed
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->shouldCall('_TaskInstall_8c014ae8')->andReturn(0);
        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_spawns_static_sprite_object()
    {
        $this->makeGroup(1);
        $this->makePath(100.0);
        $spec = $this->makeSpec(5, 2, 0, 12.0, 34.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 5 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->do(function () use ($subTask, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($subTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);

        $this->shouldWriteFloat($state + 0x20, 0.0); // flBaseX
        $this->shouldWriteFloat($state + 0x24, 0.0); // flBaseZ
        $this->shouldWriteFloat($state + 0x00, 12.0); // sprite.p.x
        $this->shouldWriteFloat($state + 0x08, 34.0); // sprite.p.z

        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(12.0, 0.0, 34.0, $state + 0x28);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($state + 0x28, $state);

        $this->shouldWriteLong($state + 0x38, 5); // nKindId
        $this->shouldWriteLong($state + 0x3c, 2); // nKind
        $this->shouldWriteLong($state + 0x40, 0); // nReverse

        $this->shouldWriteFloat($state + 0x0c, 0.015); // sprite.flSx
        $this->shouldWriteFloat($state + 0x10, 0.015); // sprite.flSy
        $this->shouldWriteLong($state + 0x14, 0); // sprite.nAng

        $this->shouldWriteLong($state + 0x18, $texlist); // sprite.pTlist
        $this->shouldWriteLong($state + 0x1c, $this->addressOf('_init_pedestrianTexAnims_8c04623c')); // sprite.pTanim

        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    /** Mocks TaskInstall and returns the installed subTask/state pair. */
    private function pushWalkingPedestrian(int $subTask, int $state): void
    {
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->do(function () use ($subTask, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($subTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);
    }

    /** Common prologue writes/calls before the path-position search, radius fixed at 0.0. */
    private function assertPedestrianPrologue(int $state, int $pathFirst, int $pathLast, float $pathLength): void
    {
        $this->shouldCall('_rand')->andReturn(0); // flBaseX
        $this->shouldWriteFloat($state + 0x20, 0.0);
        $this->shouldCall('_rand')->andReturn(0); // flBaseZ
        $this->shouldWriteFloat($state + 0x24, 0.0);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(0); // flSpeed
        $this->shouldWriteFloat($state + 0x58, 0.04);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(0); // nAnimPhase
        $this->shouldWriteLong($state + 0x5c, 0);
        $this->shouldWriteLong($state + 0x64, 0); // nState
        $this->shouldWriteLong($state + 0x44, $pathFirst); // pPathFirst
        $this->shouldWriteLong($state + 0x48, $pathLast); // pPathLast
        $this->shouldWriteFloat($state + 0x50, $pathLength); // flPathLength
    }

    /** Common epilogue from advancePedPathPos_8c0289ac through the sprite/texlist wiring. */
    private function assertPedestrianEpilogue(int $state, int $nKindId, int $texlist): void
    {
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($state)->andReturn(0);
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $state + 0x28);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($state + 0x28, $state);
        $this->shouldWriteFloat($state + 0x0c, 0.015);
        $this->shouldWriteFloat($state + 0x10, 0.015);
        $this->shouldWriteLong($state + 0x14, 0);
        $this->shouldWriteLong($state + 0x18, $texlist);
        $this->shouldWriteLong($state + 0x1c, $this->addressOf('_init_pedestrianTexAnims_8c04623c'));
    }

    public function test_texlist_unavailable_frees_subtask_instead_of_wiring_sprite()
    {
        $this->makeGroup(1);

        $node = $this->alloc(0x18 * 2);
        $this->initUint32($node + 0x00, $this->f(20.0));
        $this->initUint32($node + 0x14, 0);
        $this->initUint32($node + 0x18 + 0x00, $this->f(0.0));

        $path = $this->makePath(20.0);
        $this->initUint32($path + 0x00, $node);
        $this->initUint32($path + 0x04, $node);

        $spec = $this->makeSpec(3, 0, 0, 5.0, 0.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 0.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        // texlist_0x08 == -1: asset unavailable.
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, 0xffffffff);
        // sentinels the "wire up sprite" branch would overwrite if it wrongly ran.
        $this->initUint32($state + 0x18, 0xdeadbeef);
        $this->initUint32($state + 0x1c, 0xdeadbeef);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->pushWalkingPedestrian($subTask, $state);
        $this->assertPedestrianPrologue($state, $node, $node, 20.0);
        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags
        $this->shouldWriteFloat($state + 0x54, 5.0); // flPathPos
        $this->shouldWriteLong($state + 0x4c, $node); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3); // nKindId
        $this->shouldWriteLong($state + 0x3c, 0); // nKind
        $this->shouldWriteLong($state + 0x40, 0); // nReverse
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($state)->andReturn(0);
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $state + 0x28);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($state + 0x28, $state);
        $this->shouldWriteFloat($state + 0x0c, 0.015);
        $this->shouldWriteFloat($state + 0x10, 0.015);
        $this->shouldWriteLong($state + 0x14, 0);
        $this->shouldCall('_TaskUninstall_8c014b66')
            ->with($subTask)
            ->do(function () use ($state) {
                $tlist = $this->memory->readUInt32($state + 0x18)->value;
                $tanim = $this->memory->readUInt32($state + 0x1c)->value;
                if ($tlist !== 0xdeadbeef || $tanim !== 0xdeadbeef) {
                    throw new RuntimeException(sprintf(
                        'sprite.tlist/tanim were overwritten (%08x/%08x); the texlist-unavailable branch must skip them',
                        $tlist, $tanim,
                    ));
                }
            });

        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_position_search_spans_multiple_segments_and_wraps()
    {
        $this->makeGroup(1);

        // 3 real segments (5, 5, 10) plus a sentinel (field 0 == 0.0) marking
        // wraparound back to pPathFirst.
        $node = $this->alloc(0x18 * 4);
        $node0 = $node;
        $node1 = $node + 0x18;
        $node2 = $node + 0x18 * 2;
        $sentinel = $node + 0x18 * 3;
        $this->initUint32($node0 + 0x00, $this->f(5.0));
        $this->initUint32($node0 + 0x14, 0);
        $this->initUint32($node1 + 0x00, $this->f(5.0));
        $this->initUint32($node1 + 0x14, 0);
        $this->initUint32($node2 + 0x00, $this->f(10.0));
        $this->initUint32($node2 + 0x14, 0);
        $this->initUint32($sentinel + 0x00, $this->f(0.0));

        $path = $this->makePath(30.0);
        $this->initUint32($path + 0x00, $node0);
        $this->initUint32($path + 0x04, $node2);

        // distance 25.0: consumes all 3 segments (5+5+10=20), wraps to node0,
        // then consumes node0 (5.0) too, landing on node1 with 0.0 left over.
        $spec = $this->makeSpec(3, 0, 0, 25.0, 0.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 0.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->pushWalkingPedestrian($subTask, $state);
        $this->assertPedestrianPrologue($state, $node0, $node2, 30.0);

        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags (node1)
        $this->shouldWriteFloat($state + 0x54, 0.0); // flPathPos leftover
        $this->shouldWriteLong($state + 0x4c, $node1); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3);
        $this->shouldWriteLong($state + 0x3c, 0);
        $this->shouldWriteLong($state + 0x40, 0);

        $this->assertPedestrianEpilogue($state, 3, $texlist);
        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_backtrack_forward_skips_blocked_node_to_predecessor()
    {
        $this->makeGroup(1);

        $node = $this->alloc(0x18 * 2);
        $node0 = $node;
        $node1 = $node + 0x18;
        $this->initUint32($node0 + 0x00, $this->f(5.0));
        $this->initUint32($node0 + 0x14, 0); // clear
        $this->initUint32($node1 + 0x00, $this->f(5.0));
        $this->initUint32($node1 + 0x14, 1); // blocked (raw int, not float-encoded)

        $path = $this->makePath(20.0);
        $this->initUint32($path + 0x00, $node0); // pFirst
        $this->initUint32($path + 0x04, $node1); // pLast

        // distance 6.0: matches node1 (5.0 <= 6.0 consumes node0, then 5.0 <= 1.0 is false).
        $spec = $this->makeSpec(3, 0, 0, 6.0, 0.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 0.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->pushWalkingPedestrian($subTask, $state);
        $this->assertPedestrianPrologue($state, $node0, $node1, 20.0);

        // Backtrack (nReverse == 0): node1 is blocked, steps to node0 (node -= 6,
        // not the wrap-to-pPathLast case since node1 != pPathFirst), landing clear.
        $this->shouldWriteLong($state + 0x60, 1); // nNodeFlags (node1, before backtrack)
        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags (node0, after stepping back)
        $this->shouldWriteFloat($state + 0x54, 5.0); // flPathPos <- node0's own length
        $this->shouldWriteLong($state + 0x4c, $node0); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3);
        $this->shouldWriteLong($state + 0x3c, 0);
        $this->shouldWriteLong($state + 0x40, 0);

        $this->assertPedestrianEpilogue($state, 3, $texlist);
        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_backtrack_reverse_skips_blocked_node_to_successor()
    {
        $this->makeGroup(1);

        $node = $this->alloc(0x18 * 2);
        $node0 = $node;
        $node1 = $node + 0x18;
        $this->initUint32($node0 + 0x00, $this->f(5.0));
        $this->initUint32($node0 + 0x14, 1); // blocked (raw int, not float-encoded)
        $this->initUint32($node1 + 0x00, $this->f(5.0));
        $this->initUint32($node1 + 0x14, 0); // clear

        $path = $this->makePath(20.0);
        $this->initUint32($path + 0x00, $node0); // pFirst
        $this->initUint32($path + 0x04, $node1); // pLast

        // nReverse == 1: targetDist = flPathLength(20.0) - flX(18.0) = 2.0, which
        // is < node0's own length (5.0), so the search matches node0 immediately.
        $spec = $this->makeSpec(3, 0, 1, 18.0, 0.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 0.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->pushWalkingPedestrian($subTask, $state);
        $this->assertPedestrianPrologue($state, $node0, $node1, 20.0);

        // Backtrack (nReverse == 1): node0 is blocked, steps forward to node1
        // (node += 6, not the wrap-to-pPathFirst case since node0 != pPathLast).
        $this->shouldWriteLong($state + 0x60, 1); // nNodeFlags (node0, before backtrack)
        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags (node1, after stepping forward)
        $this->shouldWriteFloat($state + 0x54, 0.0); // flPathPos reset by the reverse backtrack
        $this->shouldWriteLong($state + 0x4c, $node1); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3);
        $this->shouldWriteLong($state + 0x3c, 0);
        $this->shouldWriteLong($state + 0x40, 1); // nReverse

        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($state)->andReturn(0);
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $state + 0x28);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($state + 0x28, $state);
        $this->shouldWriteFloat($state + 0x0c, 0.015);
        $this->shouldWriteFloat($state + 0x10, 0.015);
        $this->shouldWriteLong($state + 0x14, 0);
        $this->shouldWriteLong($state + 0x18, $texlist);
        $this->shouldWriteLong($state + 0x1c, $this->addressOf('_init_pedestrianTexAnims_8c04623c'));

        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_random_placement_when_flx_is_zero()
    {
        $this->makeGroup(1);

        $node = $this->alloc(0x18 * 2);
        $this->initUint32($node + 0x00, $this->f(20.0));
        $this->initUint32($node + 0x14, 0);
        $this->initUint32($node + 0x18 + 0x00, $this->f(0.0));

        $path = $this->makePath(20.0);
        $this->initUint32($path + 0x00, $node);
        $this->initUint32($path + 0x04, $node);

        $spec = $this->makeSpec(3, 0, 0, 0.0, 0.0); // flX == 0.0 -> random placement
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 0.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->pushWalkingPedestrian($subTask, $state);
        $this->assertPedestrianPrologue($state, $node, $node, 20.0);

        // rand()/32768 * flPathLength(20.0); rand() == 16384 -> 0.5 * 20.0 = 10.0.
        $this->shouldCall('_rand')->andReturn(16384);
        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags
        $this->shouldWriteFloat($state + 0x54, 10.0); // flPathPos
        $this->shouldWriteLong($state + 0x4c, $node); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3);
        $this->shouldWriteLong($state + 0x3c, 0);
        $this->shouldWriteLong($state + 0x40, 0);

        $this->assertPedestrianEpilogue($state, 3, $texlist);
        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    public function test_spawns_walking_pedestrian_single_segment_path()
    {
        $this->makeGroup(1);

        // Single-segment closed path: node[0] is the whole path (length 20),
        // node's own successor (index 6) wraps back to itself (0.0 marks the
        // wrap in the ghidra/asm sense: field 0 of the "next" node is read to
        // decide wraparound, so a single node's own start doubles as both).
        $node = $this->alloc(0x18 * 2);
        $this->initUint32($node + 0x00, $this->f(20.0)); // segment length
        $this->initUint32($node + 0x14, 0); // nNodeFlags (index 5): no signal
        // second slot: sentinel end-of-list (field 0 == 0.0) so the walk wraps
        $this->initUint32($node + 0x18 + 0x00, $this->f(0.0));

        $path = $this->makePath(20.0);
        $this->initUint32($path + 0x00, $node); // pFirst
        $this->initUint32($path + 0x04, $node); // pLast

        $spec = $this->makeSpec(3, 0, 0, 5.0, 0.0); // distance 5.0 along the path
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 2.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        // advancePedPathPos_8c0289ac (mocked below) is what would normally place the
        // sprite; pre-zero it so the later ground-query call args are known.
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->do(function () use ($subTask, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($subTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);

        $this->shouldCall('_rand')->andReturn(0); // flBaseX
        $this->shouldWriteFloat($state + 0x20, -2.0); // flBaseX: (0/32768)*2*2 - 2
        $this->shouldCall('_rand')->andReturn(0); // flBaseZ
        $this->shouldWriteFloat($state + 0x24, -2.0); // flBaseZ
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(0); // flSpeed
        $this->shouldWriteFloat($state + 0x58, 0.04); // flSpeed
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(7); // nAnimPhase
        $this->shouldWriteLong($state + 0x5c, 7); // nAnimPhase
        $this->shouldWriteLong($state + 0x64, 0); // nState
        $this->shouldWriteLong($state + 0x44, $node); // pPathFirst
        $this->shouldWriteLong($state + 0x48, $node); // pPathLast
        $this->shouldWriteFloat($state + 0x50, 20.0); // flPathLength

        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags
        $this->shouldWriteFloat($state + 0x54, 5.0); // flPathPos
        $this->shouldWriteLong($state + 0x4c, $node); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3); // nKindId
        $this->shouldWriteLong($state + 0x3c, 0); // nKind
        $this->shouldWriteLong($state + 0x40, 0); // nReverse

        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($state)->andReturn(0);
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(0.0, 0.0, 0.0, $state + 0x28);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($state + 0x28, $state);

        $this->shouldWriteFloat($state + 0x0c, 0.015);
        $this->shouldWriteFloat($state + 0x10, 0.015);
        $this->shouldWriteLong($state + 0x14, 0);
        $this->shouldWriteLong($state + 0x18, $texlist);
        $this->shouldWriteLong($state + 0x1c, $this->addressOf('_init_pedestrianTexAnims_8c04623c'));

        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }

    /** Rounds a PHP double to the single-precision value the SH4 FPU holds. */
    private function s(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    public function test_speed_uses_only_the_low_16_bits_of_the_random_seed()
    {
        // AsqGetRandomA_8c012166 returns the full 32-bit LCG seed; only its low
        // half feeds the speed, keeping every pedestrian between 0.04 and 0.06
        // units per frame. 0xc800 is picked so the divides stay exact.
        $seed = 0x03a5c800;
        $expectedSpeed = $this->s($this->s($this->s(0xc800 / 65536.0) / 50.0) + $this->s(0.04));

        $this->makeGroup(1);

        $node = $this->alloc(0x18 * 2);
        $this->initUint32($node + 0x00, $this->f(20.0));
        $this->initUint32($node + 0x14, 0);
        $this->initUint32($node + 0x18 + 0x00, $this->f(0.0));

        $path = $this->makePath(20.0);
        $this->initUint32($path + 0x00, $node);
        $this->initUint32($path + 0x04, $node);

        $spec = $this->makeSpec(3, 0, 0, 5.0, 0.0);
        $subTasks = $this->alloc(4);
        $task = $this->makeTask(0, $subTasks, $spec, 0.0);

        $subTask = $this->alloc(0x20);
        $state = $this->alloc(0x70);
        $this->initUint32($state + 0x00, $this->f(0.0));
        $this->initUint32($state + 0x04, $this->f(0.0));
        $this->initUint32($state + 0x08, $this->f(0.0));

        $texlist = $this->alloc(4);
        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 3 * 0x10 + 0x08, $texlist);

        $this->call('_pedGroupTask_8c029078')->with($task);

        $this->pushWalkingPedestrian($subTask, $state);

        $this->shouldCall('_rand')->andReturn(0); // flBaseX
        $this->shouldWriteFloat($state + 0x20, 0.0);
        $this->shouldCall('_rand')->andReturn(0); // flBaseZ
        $this->shouldWriteFloat($state + 0x24, 0.0);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn($seed);
        $this->shouldWriteFloat($state + 0x58, $expectedSpeed); // flSpeed
        // nAnimPhase keeps the whole seed, no masking.
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn($seed);
        $this->shouldWriteLong($state + 0x5c, $seed);
        $this->shouldWriteLong($state + 0x64, 0);
        $this->shouldWriteLong($state + 0x44, $node);
        $this->shouldWriteLong($state + 0x48, $node);
        $this->shouldWriteFloat($state + 0x50, 20.0);

        $this->shouldWriteLong($state + 0x60, 0); // nNodeFlags
        $this->shouldWriteFloat($state + 0x54, 5.0); // flPathPos
        $this->shouldWriteLong($state + 0x4c, $node); // pPathNode
        $this->shouldWriteLong($state + 0x38, 3);
        $this->shouldWriteLong($state + 0x3c, 0);
        $this->shouldWriteLong($state + 0x40, 0);

        $this->assertPedestrianEpilogue($state, 3, $texlist);
        $this->shouldWriteLong($task + self::OFF_SPEC, $spec + self::SPEC_SIZE);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($subTasks);
    }
};
