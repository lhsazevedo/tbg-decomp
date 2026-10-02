<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // {int id, void *pvm, void *dat} entry, matching var_messageAssets_8c228484.
    const ENTRY_SIZE = 0x0c;

    private function resolveSymbols(): void
    {
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);
        $this->setSize('_var_commonDir_8c18ad6c', 0x20);
    }

    public function test_route_shinjuku_requests_dat_and_walks_event_groups(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0); // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), 0);

        $this->call('_MessageBoxRequestAssets_8c02aa36');

        $commonDir = $this->addressOf('_var_commonDir_8c18ad6c');
        $entries = $this->addressOf('_var_messageAssets_8c228484');

        $this->shouldWriteLongTo('_var_eventSlides_8c228480', $this->addressOf('_init_shinjukuEvents_8c049a6c'));

        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 's_text.dat', $this->addressOf('_var_messageTextDat_8c228518'),
        );

        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);

        // Event 0's groups reference ids 1, 1, 201, 2, 1, 1, 201 in order --
        // covers a repeat within one group, a repeat across groups, and a
        // large id whose filenames land past the table's first (padding) entry
        // in init_slideAssetFiles_8c046758.
        $this->shouldWriteLong($entries + 0 * self::ENTRY_SIZE, 1);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B001.pvm', $entries + 0 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B001.map', $entries + 0 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 1);

        $this->shouldWriteLong($entries + 1 * self::ENTRY_SIZE, 201);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'P001.pvm', $entries + 1 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'P001.map', $entries + 1 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 2);

        $this->shouldWriteLong($entries + 2 * self::ENTRY_SIZE, 2);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B002.pvm', $entries + 2 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B002.map', $entries + 2 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 3);
    }

    public function test_route_wangan_requests_dat_and_walks_event_groups(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), 0);

        $this->call('_MessageBoxRequestAssets_8c02aa36');

        $commonDir = $this->addressOf('_var_commonDir_8c18ad6c');
        $entries = $this->addressOf('_var_messageAssets_8c228484');

        $this->shouldWriteLongTo('_var_eventSlides_8c228480', $this->addressOf('_init_wanganEvents_8c04843c'));

        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'w_text.dat', $this->addressOf('_var_messageTextDat_8c228518'),
        );

        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);

        // Event 0's groups reference ids 64, 63, 65, 64, 63, 64, 63, 64.
        $this->shouldWriteLong($entries + 0 * self::ENTRY_SIZE, 64);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B064.pvm', $entries + 0 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B064.map', $entries + 0 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 1);

        $this->shouldWriteLong($entries + 1 * self::ENTRY_SIZE, 63);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B063.pvm', $entries + 1 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B063.map', $entries + 1 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 2);

        $this->shouldWriteLong($entries + 2 * self::ENTRY_SIZE, 65);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B065.pvm', $entries + 2 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B065.map', $entries + 2 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 3);
    }

    public function test_route_ome_requests_dat_and_walks_event_groups(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), 0);

        $this->call('_MessageBoxRequestAssets_8c02aa36');

        $commonDir = $this->addressOf('_var_commonDir_8c18ad6c');
        $entries = $this->addressOf('_var_messageAssets_8c228484');

        $this->shouldWriteLongTo('_var_eventSlides_8c228480', $this->addressOf('_init_omeEvents_8c04a9c8'));

        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'o_text.dat', $this->addressOf('_var_messageTextDat_8c228518'),
        );

        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);

        // Event 0's groups reference ids 101, 104, 102, 103, 101, 104.
        $this->shouldWriteLong($entries + 0 * self::ENTRY_SIZE, 101);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B101.pvm', $entries + 0 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B101.map', $entries + 0 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 1);

        $this->shouldWriteLong($entries + 1 * self::ENTRY_SIZE, 104);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B104.pvm', $entries + 1 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B104.map', $entries + 1 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 2);

        $this->shouldWriteLong($entries + 2 * self::ENTRY_SIZE, 102);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B102.pvm', $entries + 2 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B102.map', $entries + 2 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 3);

        $this->shouldWriteLong($entries + 3 * self::ENTRY_SIZE, 103);
        $this->shouldCall('_AsqRequestPvm_8c011ac0')->with(
            $commonDir, 'B103.pvm', $entries + 3 * self::ENTRY_SIZE + 0x04, 0xde, 0,
        );
        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            $commonDir, 'B103.map', $entries + 3 * self::ENTRY_SIZE + 0x08,
        );
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 4);
    }

    public function test_unrecognized_route_skips_dat_request_and_reuses_stale_table(): void
    {
        $this->resolveSymbols();

        // An empty group list: the outer walk's very first entry is already
        // the -1 terminator.
        $groupList = $this->alloc(0x08);
        $this->initUint32($groupList + 0x00, -1);
        $this->initUint32($groupList + 0x04, 0);

        $table = $this->alloc(0x04);
        $this->initUint32($table + 0x00, $groupList);

        $this->initUint32($this->addressOf('_var_eventSlides_8c228480'), $table);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 3); // not a real ROUTE value
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), 0);

        $this->call('_MessageBoxRequestAssets_8c02aa36');

        // No s_text/w_text/o_text.dat request, var_eventSlides_8c228480 left untouched,
        // and the (empty) group walk still runs and resets the count.
        $this->shouldWriteLongTo('_var_messageAssetCount_8c228514', 0);
    }
};
