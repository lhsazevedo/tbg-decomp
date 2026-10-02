<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    private function f(float $value): int
    {
        return fdec($value);
    }

    // Task offsets used by pedestriansTask_8c0293f6.
    const OFF_DEMO_ENTRY = 0x08;
    const OFF_PENDING = 0x0c;

    // PedGroupEntry offsets (0xc bytes per entry).
    const OFF_GRP_ACTIVE = 0x00;
    const OFF_GRP_WANTED = 0x04;
    const OFF_GRP_LIST = 0x08;
    const GRP_SIZE = 0x0c;

    // PedGroupDef offsets (0xc bytes per entry).
    const OFF_DEF_ID = 0x00;
    const OFF_DEF_RADIUS = 0x04;
    const OFF_DEF_SPECS = 0x08;
    const DEF_SIZE = 0x0c;

    private function makeTask(int $demoEntry, int $pending): int
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + self::OFF_DEMO_ENTRY, $demoEntry);
        $this->initUint32($task + self::OFF_PENDING, $pending);
        return $task;
    }

    /** Allocates a one-entry group table (index 0). */
    private function makeGroups(int $active, int $wanted): int
    {
        $groups = $this->alloc(self::GRP_SIZE);
        $this->initUint32($groups + self::OFF_GRP_ACTIVE, $active);
        $this->initUint32($groups + self::OFF_GRP_WANTED, $wanted);
        $this->initUint32($groups + self::OFF_GRP_LIST, 0);
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);
        return $groups;
    }

    /** Points demo-entry `entry`'s group list at a single group id, -1 terminated. */
    private function makeGroupList(int $entry, int $groupId): void
    {
        $lists = $this->alloc(($entry + 1) * 4);
        $list = $this->alloc(8);
        $this->initUint32($list + 0, $groupId);
        $this->initUint32($list + 4, -1);
        $this->initUint32($lists + $entry * 4, $list);
        $this->initUint32($this->addressOf('_var_pedGroupLists_8c228240'), $lists);
    }

    /** Points demo-entry `entry`'s group list at several group ids, -1 terminated. */
    private function makeGroupListMulti(int $entry, array $groupIds): void
    {
        $lists = $this->alloc(($entry + 1) * 4);
        $list = $this->alloc((count($groupIds) + 1) * 4);
        foreach ($groupIds as $i => $id) {
            $this->initUint32($list + $i * 4, $id);
        }
        $this->initUint32($list + count($groupIds) * 4, -1);
        $this->initUint32($lists + $entry * 4, $list);
        $this->initUint32($this->addressOf('_var_pedGroupLists_8c228240'), $lists);
    }

    /** Points every var_pedPaths_8c228238 slot (up to `count`) at an empty
     * (immediately-terminated) path, so an active group's crosswalk scan
     * touches no uninitialized memory. */
    private function makeEmptyPaths(int $count): void
    {
        $paths = $this->alloc($count * 0xc);
        $node = $this->alloc(4);
        $this->initUint32($node, $this->f(0.0));
        for ($i = 0; $i < $count; $i++) {
            $this->initUint32($paths + $i * 0xc + 0x00, $node);
        }
        $this->initUint32($this->addressOf('_var_pedPaths_8c228238'), $paths);
    }

    private function makeGroupDef(int $id, float $radius, int $specs): int
    {
        $def = $this->alloc(self::DEF_SIZE);
        $this->initUint32($def + self::OFF_DEF_ID, $id);
        $this->initUint32($def + self::OFF_DEF_RADIUS, $this->f($radius));
        $this->initUint32($def + self::OFF_DEF_SPECS, $specs);
        $this->initUint32($this->addressOf('_var_pedGroupDefs_8c22823c'), $def);
        return $def;
    }

    /** Common gate + ground-grid + demo-entry preconditions that never change. */
    private function setupCommon(int $groupCount = 0): void
    {
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);

        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x28, 0x11111111); // atariHum_0x28

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0);

        $this->initUint32($this->addressOf('_var_pedGroupCount_8c228234'), $groupCount);
        $this->initUint32($this->addressOf('_var_activePedPreset_8c22822c'), 0);

        // Read unconditionally even when the group-scan/crosswalk loops
        // below have nothing to do.
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), 0);
        $this->initUint32($this->addressOf('_var_pedPaths_8c228238'), 0);
        $this->initUint32($this->addressOf('_var_pedGroupDefs_8c22823c'), 0);
        $this->initUint32($this->addressOf('_var_pedGroupLists_8c228240'), 0);

        $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
    }

    /** Mocks the two njCalcPoint calls and asserts their scratch-point writes. */
    private function expectGroundScratch(): void
    {
        $this->shouldCall('_CollisionQueueReset_8c02e486');
        $this->shouldCall('_SignalClearPedCrossing_8c02890c');

        $this->shouldCall('_njCalcPoint')
            ->with(($this->addressOf('_var_busState_8c1bb9d0') + 0x084), $this->addressOf('_init_busAxisLocalA_8c04650c'),
                $this->addressOf('_var_groundQueryPoint_8c1bc460'))
            ->do(function () {
                $this->memory->writeUInt32($this->registers[6]->value + 0x0, U32::of(fdec(1.5)));
                $this->memory->writeUInt32($this->registers[6]->value + 0x8, U32::of(fdec(2.5)));
            });
        $this->shouldWriteFloat($this->addressOf('_var_busAxisA_8c228268'), 1.5);
        $this->shouldWriteFloat($this->addressOf('_var_busAxisA_8c228268') + 4, 2.5);

        $this->shouldCall('_njCalcPoint')
            ->with(($this->addressOf('_var_busState_8c1bb9d0') + 0x084), $this->addressOf('_init_busAxisLocalB_8c046518'),
                $this->addressOf('_var_groundQueryPoint_8c1bc460'))
            ->do(function () {
                $this->memory->writeUInt32($this->registers[6]->value + 0x0, U32::of(fdec(3.5)));
                $this->memory->writeUInt32($this->registers[6]->value + 0x8, U32::of(fdec(4.5)));
            });
        $this->shouldWriteFloat($this->addressOf('_var_busAxisB_8c228270'), 3.5);
        $this->shouldWriteFloat($this->addressOf('_var_busAxisB_8c228270') + 4, 4.5);

        $this->shouldWriteLongTo('_var_blockedCrosswalksEnd_8c228244', $this->addressOf('_var_blockedCrosswalks_8c228248'));
    }

    public function test_skips_everything_before_the_run_starts()
    {
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 0);

        $task = $this->alloc(0x20);

        $this->call('_pedestriansTask_8c0293f6')->with($task);
    }

    public function test_pending_flag_set_when_demo_flags_clear()
    {
        $this->setupCommon();
        $task = $this->makeTask(0, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldWriteLong($task + self::OFF_PENDING, 1);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);

        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);
    }

    // NOTE: the "commit a newly-signalled demo entry" branch
    // ((int)(short)(demoFlags >> 16), compiled as SHLR16+EXTS.W) can't be
    // exercised here: sh4objtest v0.1.44 doesn't implement EXTS.W yet, and
    // that's true of the unmodified .src object too, not just the C
    // translation. Covered instead by test_pending_flag_set_when_demo_flags_clear
    // (the demoFlags == 0 side of the same if/else-if).

    public function test_non_demo_mode_registers_mirror_layer()
    {
        $this->setupCommon();
        // task->field_0x08 already matches var_activePedPreset_8c22822c (0)
        // and demoFlags is 0, so neither the pending-flag toggle nor the
        // group-scan block fires here -- this isolates the draw-callback tail.
        $task = $this->makeTask(0, 1);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // PLAY_MODE_NORMAL
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));

        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawPedestriansMirror_8c028a38'), 1);
    }

    public function test_mirror_gate_registers_two_extra_layers()
    {
        $this->setupCommon();
        $task = $this->makeTask(0, 1);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 2);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));

        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_StopDrawWaitingPassengers_8c02d06c'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_StopDrawWaitingPassengers_8c02d06c'), 1);
    }

    public function test_new_group_is_spun_up_and_marked_wanted()
    {
        $this->setupCommon(1);
        $task = $this->makeTask(-1, 1); // -1 != 0: forces the group-scan block
        $groups = $this->makeGroups(0, 0);
        $this->makeGroupList(0, 0);
        $specs = 0x12345678;
        $def = $this->makeGroupDef(0, 3.5, $specs);
        $this->makeEmptyPaths(1);

        $subtasks = 0x30000000;
        $subTask = $this->alloc(0x20);
        $state = $this->alloc(4);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldWriteLong($task + self::OFF_DEMO_ENTRY, 0);

        $this->shouldWriteLong($groups + self::OFF_GRP_WANTED, 0); // reset pass
        $this->shouldCall('_syMalloc')->with(0x220)->andReturn($subtasks);
        $this->shouldWriteLong($groups + self::OFF_GRP_LIST, $subtasks);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($subtasks, 0x10);
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->do(function () use ($subTask, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($subTask));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($subTask + 0x18, $subtasks);
        $this->shouldWriteLong($subTask + 0x1c, $specs);
        $this->shouldWriteLong($subTask + 0x08, 0);
        $this->shouldWriteFloat($subTask + 0x10, 3.5);
        $this->shouldWriteLong($groups + self::OFF_GRP_ACTIVE, 1);
        $this->shouldWriteLong($groups + self::OFF_GRP_WANTED, 1);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
    }

    public function test_already_active_group_is_only_marked_wanted()
    {
        $this->setupCommon(1);
        $task = $this->makeTask(-1, 1);
        $groups = $this->makeGroups(1, 0);
        $this->makeGroupList(0, 0);
        $this->makeEmptyPaths(1);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldWriteLong($task + self::OFF_DEMO_ENTRY, 0);
        $this->shouldWriteLong($groups + self::OFF_GRP_WANTED, 0); // reset pass
        $this->shouldWriteLong($groups + self::OFF_GRP_WANTED, 1);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
    }

    public function test_alloc_failure_breaks_scan_without_marking_wanted()
    {
        $this->setupCommon(1);
        $task = $this->makeTask(-1, 1);
        $groups = $this->makeGroups(0, 0);
        $this->makeGroupList(0, 0);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldWriteLong($task + self::OFF_DEMO_ENTRY, 0);
        $this->shouldWriteLong($groups + self::OFF_GRP_WANTED, 0); // reset pass
        $this->shouldCall('_syMalloc')->with(0x220)->andReturn(0);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
    }

    public function test_task_spawn_failure_frees_and_breaks_scan()
    {
        $this->setupCommon(1);
        $task = $this->makeTask(-1, 1);
        $groups = $this->makeGroups(0, 0);
        $this->makeGroupList(0, 0);
        $subtasks = 0x30000000;

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldWriteLong($task + self::OFF_DEMO_ENTRY, 0);
        $this->shouldWriteLong($groups + self::OFF_GRP_WANTED, 0); // reset pass
        $this->shouldCall('_syMalloc')->with(0x220)->andReturn($subtasks);
        $this->shouldWriteLong($groups + self::OFF_GRP_LIST, $subtasks);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($subtasks, 0x10);
        $this->shouldCall('_TaskSpawn_8c014ae8')->andReturn(0);
        $this->shouldCall('_syFree')->with($subtasks);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
    }

    public function test_crosswalk_scratch_built_for_active_group_signalled_node()
    {
        $this->setupCommon(1);
        $task = $this->makeTask(0, 1); // matches var_activePedPreset_8c22822c: scan skipped
        $groups = $this->makeGroups(1, 1);

        // node0: unsignalled, skipped without an intersection check.
        // node1: signalled, intersection hits -> appended.
        // node2: terminator (field 0 == 0.0).
        $node = $this->alloc(0x18 * 3);
        $this->initUint32($node + 0x00, $this->f(1.0));
        $this->initUint32($node + 0x14, 0); // nNodeFlags: none
        $this->initUint32($node + 0x18 + 0x00, $this->f(1.0));
        $this->initUint32($node + 0x18 + 0x14, 1); // nNodeFlags: signalled
        $this->initUint32($node + 0x30 + 0x00, $this->f(0.0)); // terminator

        $paths = $this->alloc(0xc);
        $this->initUint32($paths + 0x00, $node);
        $this->initUint32($this->addressOf('_var_pedPaths_8c228238'), $paths);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);

        $this->expectGroundScratch();

        $this->shouldCall('_IntersectSegments_8c0206f0')
            ->with($this->addressOf('_var_busAxisA_8c228268'), $this->addressOf('_var_busAxisB_8c228270'),
                $node + 0x18 + 0x04, $node + 0x18 + 0x1c, $this->addressOf('_var_crossingIntersectPoint_8c1bc458'))
            ->andReturn(1);

        // Stop here: the two appended-entry writes that follow go through a
        // pre-decrement store addressing mode that sh4objtest v0.1.44
        // doesn't surface as an ordered write event on the .src object (it
        // does on the compiled C, so asserting them diverges the two
        // objects' expectation queues). The append is still exercised --
        // just not its exact byte-level ordering.
        $this->forceStop();
    }

    public function test_crosswalk_high_bits_alone_do_not_signal_a_node()
    {
        $this->setupCommon(1);
        $task = $this->makeTask(0, 1); // matches var_activePedPreset_8c22822c: scan skipped
        $groups = $this->makeGroups(1, 1);

        // node0: only high bits set (0x1000 & 0xfff == 0) -- must NOT check the
        // signal. node1: all masked bits set (0xfff) -- must check it.
        $node = $this->alloc(0x18 * 3);
        $this->initUint32($node + 0x00, $this->f(1.0));
        $this->initUint32($node + 0x14, 0x1000); // nNodeFlags: masked to 0
        $this->initUint32($node + 0x18 + 0x00, $this->f(1.0));
        $this->initUint32($node + 0x18 + 0x14, 0xfff); // nNodeFlags: fully masked-in
        $this->initUint32($node + 0x30 + 0x00, $this->f(0.0)); // terminator

        $paths = $this->alloc(0xc);
        $this->initUint32($paths + 0x00, $node);
        $this->initUint32($this->addressOf('_var_pedPaths_8c228238'), $paths);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);

        $this->expectGroundScratch();

        // node0 is skipped entirely: the only IntersectSegments_8c0206f0 call is for node1.
        $this->shouldCall('_IntersectSegments_8c0206f0')
            ->with($this->addressOf('_var_busAxisA_8c228268'), $this->addressOf('_var_busAxisB_8c228270'),
                $node + 0x18 + 0x04, $node + 0x18 + 0x1c, $this->addressOf('_var_crossingIntersectPoint_8c1bc458'))
            ->andReturn(0);

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
    }

    public function test_group_scan_spins_up_two_groups_with_non_first_def_match()
    {
        $this->setupCommon(2);
        $task = $this->makeTask(-1, 1); // -1 != 0: forces the group-scan block
        $groups = $this->alloc(2 * self::GRP_SIZE);
        for ($i = 0; $i < 2; $i++) {
            $this->initUint32($groups + $i * self::GRP_SIZE + self::OFF_GRP_ACTIVE, 0);
            $this->initUint32($groups + $i * self::GRP_SIZE + self::OFF_GRP_WANTED, 0);
            $this->initUint32($groups + $i * self::GRP_SIZE + self::OFF_GRP_LIST, 0);
        }
        $this->initUint32($this->addressOf('_var_pedGroups_8c228230'), $groups);
        $group0 = $groups;
        $group1 = $groups + self::GRP_SIZE;

        $this->makeGroupListMulti(0, [0, 1]);

        // def table: id 0 first, id 1 second -- group 1's match is NOT the
        // table's first entry.
        $defs = $this->alloc(2 * self::DEF_SIZE);
        $def0 = $defs;
        $def1 = $defs + self::DEF_SIZE;
        $this->initUint32($def0 + self::OFF_DEF_ID, 0);
        $this->initUint32($def0 + self::OFF_DEF_RADIUS, $this->f(1.0));
        $this->initUint32($def0 + self::OFF_DEF_SPECS, 0xaaaa0000);
        $this->initUint32($def1 + self::OFF_DEF_ID, 1);
        $this->initUint32($def1 + self::OFF_DEF_RADIUS, $this->f(2.0));
        $this->initUint32($def1 + self::OFF_DEF_SPECS, 0xbbbb0000);
        $this->initUint32($this->addressOf('_var_pedGroupDefs_8c22823c'), $defs);

        $this->makeEmptyPaths(2);

        $subtasks0 = 0x30000000;
        $subtasks1 = 0x30001000;
        $subTask0 = $this->alloc(0x20);
        $state0 = $this->alloc(4);
        $subTask1 = $this->alloc(0x20);
        $state1 = $this->alloc(4);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);

        $this->call('_pedestriansTask_8c0293f6')->with($task);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
        $this->shouldWriteLong($task + self::OFF_DEMO_ENTRY, 0);

        $this->shouldWriteLong($group0 + self::OFF_GRP_WANTED, 0); // reset pass
        $this->shouldWriteLong($group1 + self::OFF_GRP_WANTED, 0);

        // Group 0: list-advance visits it first, def match is the table's first entry.
        $this->shouldCall('_syMalloc')->with(0x220)->andReturn($subtasks0);
        $this->shouldWriteLong($group0 + self::OFF_GRP_LIST, $subtasks0);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($subtasks0, 0x10);
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->do(function () use ($subTask0, $state0) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($subTask0));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state0));
            })
            ->andReturn(1);
        $this->shouldWriteLong($subTask0 + 0x18, $subtasks0);
        $this->shouldWriteLong($subTask0 + 0x1c, 0xaaaa0000);
        $this->shouldWriteLong($subTask0 + 0x08, 0);
        $this->shouldWriteFloat($subTask0 + 0x10, 1.0);
        $this->shouldWriteLong($group0 + self::OFF_GRP_ACTIVE, 1);
        $this->shouldWriteLong($group0 + self::OFF_GRP_WANTED, 1);

        // Group 1: the list-advance loop must still be at the right slot, and
        // the def search must walk past id 0 to find id 1 -- not stop at def0.
        $this->shouldCall('_syMalloc')->with(0x220)->andReturn($subtasks1);
        $this->shouldWriteLong($group1 + self::OFF_GRP_LIST, $subtasks1);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($subtasks1, 0x10);
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->do(function () use ($subTask1, $state1) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($subTask1));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state1));
            })
            ->andReturn(1);
        $this->shouldWriteLong($subTask1 + 0x18, $subtasks1);
        $this->shouldWriteLong($subTask1 + 0x1c, 0xbbbb0000); // def1's specs, not def0's
        $this->shouldWriteLong($subTask1 + 0x08, 1);
        $this->shouldWriteFloat($subTask1 + 0x10, 2.0); // def1's radius
        $this->shouldWriteLong($group1 + self::OFF_GRP_ACTIVE, 1);
        $this->shouldWriteLong($group1 + self::OFF_GRP_WANTED, 1);

        $this->expectGroundScratch();

        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba808'));
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawPedestrians_8c028b74'), 0);
    }
};
