<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficMarkSignalIdsInUse_8c026dcc rebuilds var_trafficSignalFrames_8c227e24 (in-use flags for
// signal ids 0..maxId) for the current segment. With no scene-object-type
// list at all (CourseSegment.sceneObjectTypeIds_0x14 == NULL), every id is
// conservatively marked in-use. Otherwise every id starts free, then every
// already-placed script's opcode-5 (fixed id) instruction, across
// every scene-object type listed for the segment, marks its id in-use.
return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_var_trafficSignalFrames_8c227e24', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
    }

    public function test_noTypeList_marksEveryIdUpToMaxIdInUse(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 3);

        // CourseSegment record; only sceneObjectTypeIds_0x14 matters.
        $seg = $this->alloc(0x18);
        $this->initUint32($seg + 0x14, 0);

        $frames = $this->alloc(4 * 4);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $frames);

        $this->call('_TrafficMarkSignalIdsInUse_8c026dcc')->with(2);

        $this->shouldCall('_StopGetSegment_8c02cd6a')->with(3)->andReturn($seg);
        $this->shouldWriteLong($frames + 0, 1);
        $this->shouldWriteLong($frames + 4, 1);
        $this->shouldWriteLong($frames + 8, 1);
    }

    // Negative maxId: the loop must reject every index and write nothing.
    public function test_noTypeList_negativeMaxIdWritesNothing(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 0);

        $seg = $this->alloc(0x18);
        $this->initUint32($seg + 0x14, 0);

        $frames = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $frames);

        $this->call('_TrafficMarkSignalIdsInUse_8c026dcc')->with(0xffffffff); // maxId = -1

        $this->shouldCall('_StopGetSegment_8c02cd6a')->with(0)->andReturn($seg);
    }

    public function test_emptyTypeList_zeroFillsWithNoMarks(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 5);

        $typeIds = $this->alloc(1);
        $this->initUint8($typeIds, 0xff); // terminator right away

        $seg = $this->alloc(0x18);
        $this->initUint32($seg + 0x14, $typeIds);

        $frames = $this->alloc(4 * 3);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $frames);

        $this->call('_TrafficMarkSignalIdsInUse_8c026dcc')->with(2);

        $this->shouldCall('_StopGetSegment_8c02cd6a')->with(5)->andReturn($seg);
        $this->shouldWriteLong($frames + 0, 0);
        $this->shouldWriteLong($frames + 4, 0);
    }

    // One scene-object type, whose placed-object list has one record with a
    // two-instruction script: opcode 5 (fixed id) claiming id 7, then the
    // opcode-9 terminator. Only id 7 should end up marked.
    public function test_oneType_oneRecord_marksOnlyItsScriptedId(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 0);

        $typeIds = $this->alloc(2);
        $this->initUint8($typeIds + 0, 0); // one scene-object type: id 0
        $this->initUint8($typeIds + 1, 0xff);

        $seg = $this->alloc(0x18);
        $this->initUint32($seg + 0x14, $typeIds);

        // Placed-object record chain for type 0: one record, then a
        // terminator record (its own +4 field is 0).
        $script = $this->alloc(6); // { opcode=5, id=7 }, then opcode 9
        $this->initUint16($script + 0, 5);
        $this->initUint16($script + 2, 7);
        $this->initUint16($script + 4, 9);

        $rec = $this->alloc(0x18);
        $this->initUint32($rec + 0x04, $script);
        $this->initUint32($rec + 0x0c + 0x04, 0); // next record's +4 == terminator

        $table = $this->alloc(4);
        $this->initUint32($table, $rec);
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x24, $table);

        $frames = $this->alloc(4 * 8);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $frames);

        $this->call('_TrafficMarkSignalIdsInUse_8c026dcc')->with(7);

        $this->shouldCall('_StopGetSegment_8c02cd6a')->with(0)->andReturn($seg);
        for ($i = 0; $i < 7; $i++) {
            $this->shouldWriteLong($frames + $i * 4, 0);
        }
        $this->shouldWriteLong($frames + 7 * 4, 1);
    }
};
