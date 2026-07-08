<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RoriCallingConvention;

return new class extends TestCase {
    /** filenames_0x1c[] slots requested as .dat (vs .nj); slot 2 is a raw store. */
    private const DAT_SLOTS = [8, 11, 12, 13];

    /**
     * One real route per area/time-of-day dispatch branch. `files` holds the
     * record's filenames_0x1c[]; slot 2 is a symbol ('@name') stored raw, the
     * rest are name strings passed to the asset requests.
     */
    private const ROUTES = [
        9 => [
            'record' => '_init_courseShinjukuDay0_8c0406cc', 'ad1c' => 0, 'ad20' => 0,
            'common' => "\\SD_COMMON", 'pvr' => "\\SD_PVR",
            'ukn10' => '_init_8c03d848',
            'files' => ["s_atari_bus.dat", "s_line_bus.dat", '@init_8c04ce10', "s_attr_bus.dat", "s_attr_mark.dat", "s_atari_cpu.dat", "s_line_cpu.dat", "s_attr_cpu.dat", "sd_mac_cpu1.dat", "s_atari_hum.dat", "s_line_hum.dat", "sd_mac_hum_g0.dat", "sd_mac_hum_m0.dat", "s_mac_signal.dat", "sd_road_x.dat", "sd_machi_x.dat", "sd_uv_x.dat", "sn_shadow_x.dat", "sn_shadow_x.dat"],
        ],
        15 => [
            'record' => '_init_courseShinjukuNight0_8c04093c', 'ad1c' => 0, 'ad20' => 2,
            'common' => "\\SN_COMMON", 'pvr' => "\\SN_PVR",
            'ukn10' => '_init_8c03d958',
            'files' => ["s_atari_bus.dat", "s_line_bus.dat", '@init_8c04ce10', "s_attr_bus.dat", "s_attr_mark.dat", "s_atari_cpu.dat", "s_line_cpu.dat", "s_attr_cpu.dat", "sn_mac_cpu1.dat", "s_atari_hum.dat", "s_line_hum.dat", "sn_mac_hum_g0.dat", "sn_mac_hum_m0.dat", "s_mac_signal.dat", "sd_road_x.dat", "sn_machi_x.dat", "sd_uv_x.dat", "sn_shadow_x.dat", "sn_shadow_x.dat"],
        ],
        3 => [
            'record' => '_init_courseWanganEvening0_8c041d80', 'ad1c' => 1, 'ad20' => 1,
            'common' => "\\WD_COMMON", 'pvr' => "\\WD_PVR",
            'ukn10' => '_init_8c040d28',
            'files' => ["w_atari_bus.dat", "w_line_bus.dat", '@init_8c04e988', "w_attr_bus.dat", "w_attr_mark.dat", "w_atari_cpu.dat", "w_line_cpu.dat", "w_attr_cpu.dat", "we_mac_cpu1.dat", "w_atari_hum.dat", "w_line_hum.dat", "we_mac_hum_g0.dat", "we_mac_hum_m0.dat", "w_mac_signal.dat", "wd_road_x.dat", "wd_machi_x.dat", "wd_uv_x.dat", "wd_shadow_x.dat", "wd_shadow_x.dat"],
        ],
        6 => [
            'record' => '_init_courseWanganNight0_8c041eb8', 'ad1c' => 1, 'ad20' => 2,
            'common' => "\\WN_COMMON", 'pvr' => "\\WN_PVR",
            'ukn10' => '_init_8c040db0',
            'files' => ["w_atari_bus.dat", "w_line_bus.dat", '@init_8c04e988', "w_attr_bus.dat", "w_attr_mark.dat", "w_atari_cpu.dat", "w_line_cpu.dat", "w_attr_cpu.dat", "wn_mac_cpu1.dat", "w_atari_hum.dat", "w_line_hum.dat", "wn_mac_hum_g0.dat", "wn_mac_hum_m0.dat", "w_mac_signal.dat", "wd_road_x.dat", "wn_machi_x.dat", "wd_uv_x.dat", "wn_shadow_x.dat", "wn_shadow_x.dat"],
        ],
        18 => [
            'record' => '_init_courseOmeDay0_8c03d1ec', 'ad1c' => 2, 'ad20' => 0,
            'common' => "\\OD_COMMON", 'pvr' => "\\OD_PVR",
            'ukn10' => '_init_8c03c2b4',
            'files' => ["o_atari_bus.dat", "o_line_bus.dat", '@init_8c04df38', "o_attr_bus.dat", "o_attr_mark.dat", "o_atari_cpu.dat", "o_line_cpu.dat", "o_attr_cpu.dat", "od_mac_cpu1.dat", "o_atari_hum.dat", "o_line_hum.dat", "od_mac_hum_g0.dat", "od_mac_hum_m0.dat", "o_mac_signal.dat", "od_road_x.dat", "od_machi_x.dat", "od_uv1_x.dat", "od_uv2_x.dat", "od_shadow_x.dat"],
        ],
        24 => [
            'record' => '_init_courseOmeNight0_8c03d45c', 'ad1c' => 2, 'ad20' => 2,
            'common' => "\\ON_COMMON", 'pvr' => "\\ON_PVR",
            'ukn10' => '_init_8c03c3c4',
            'files' => ["o_atari_bus.dat", "o_line_bus.dat", '@init_8c04df38', "o_attr_bus.dat", "o_attr_mark.dat", "o_atari_cpu.dat", "o_line_cpu.dat", "o_attr_cpu.dat", "on_mac_cpu1.dat", "o_atari_hum.dat", "o_line_hum.dat", "on_mac_hum_g0.dat", "on_mac_hum_m0.dat", "o_mac_signal.dat", "od_road_x.dat", "on_machi_x.dat", "od_uv1_x.dat", "od_uv2_x.dat", "on_shadow_x.dat"],
        ],
    ];

    public function test_route_summer_day(): void   { $this->assertRoute(9); }
    public function test_route_summer_night(): void  { $this->assertRoute(15); }
    public function test_route_winter_day(): void    { $this->assertRoute(3); }
    public function test_route_winter_night(): void  { $this->assertRoute(6); }
    public function test_route_autumn_day(): void    { $this->assertRoute(18); }
    public function test_route_autumn_night(): void  { $this->assertRoute(24); }

    /**
     * Area/time selectors out of range: no basedir/pvr strcpy runs, everything
     * else is unchanged. The record fields are forced past the real data, which
     * only ever holds 0..2. ad1c > 2 skips the outer switch; ad1c in range with
     * ad20 > 2 skips each inner switch.
     */
    public function test_area_out_of_range(): void      { $this->assertRoute(9, 3); }
    public function test_time_out_of_range_summer(): void { $this->assertRoute(9, null, 3); }
    public function test_time_out_of_range_winter(): void { $this->assertRoute(3, null, 3); }
    public function test_time_out_of_range_autumn(): void { $this->assertRoute(18, null, 3); }

    private function assertRoute(int $routeId, ?int $forceAd1c = null, ?int $forceAd20 = null): void
    {
        $cfg = self::ROUTES[$routeId];

        $this->setSizes();
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x00, $routeId);

        $record = $this->addressOf($cfg['record']);
        $ad1c = $cfg['ad1c'];
        $ad20 = $cfg['ad20'];
        if ($forceAd1c !== null) { $this->initUint32($record + 0x00, $forceAd1c); $ad1c = $forceAd1c; }
        if ($forceAd20 !== null) { $this->initUint32($record + 0x04, $forceAd20); $ad20 = $forceAd20; }

        $this->call('_loadRouteModels_8c014088');

        $this->shouldWriteLong($this->addressOf('_var_currentCourseConfig_8c18ad18'), $record);
        $this->shouldWriteLong($this->addressOf('_var_route_8c18ad1c'), $ad1c);
        $this->shouldWriteLong($this->addressOf('_var_timeOfDay_8c18ad20'), $ad20);
        $this->shouldWriteLong($this->addressOf('_var_sceneParams_8c18ad24'), $this->addressOf($cfg['ukn10']));

        $basedir = $this->addressOf('_var_commonDir_8c18ad6c');
        if ($ad1c <= 2 && $ad20 <= 2) {
            $this->expectStrcpy($basedir, $cfg['common']);
            $this->expectStrcpy($this->addressOf('_var_pvrDir_8c18ad4c'), $cfg['pvr']);
        }
        $this->expectStrcpy($this->addressOf('_var_commonDirCopy_8c18ad8c'), $basedir);
        $this->expectStrcpy($this->addressOf('_var_datDir_8c18ad2c'), $basedir);

        $base = $this->addressOf('_var_currentCourse_8c1bb868');
        foreach ($cfg['files'] as $k => $file) {
            $dest = $base + 0x04 + 4 * $k;

            if ($file[0] === '@') {
                $this->shouldWriteLong($dest, $this->addressOf('_' . substr($file, 1)));
            } elseif (in_array($k, self::DAT_SLOTS, true)) {
                $this->shouldCall('_AsqRequestDat_11182')->with($basedir, $file, $dest);
            } elseif ($k === 10) {
                $this->shouldCall('_AsqRequestNj_11492')->with($basedir, $file, 0, $dest);
            } else {
                $this->shouldCall('_AsqRequestNj_11492')->with($basedir, $file, $dest, 0);
            }
        }

        $this->shouldCall('_requestVehicleAssets_8c013ae8');
        $this->shouldCall('_AsqRequestModels_12030')
            ->with($basedir, $this->addressOf('_init_8c0440dc'), 0x10)
            ->andReturn(0x8cff0000);
        $this->shouldWriteLong($this->addressOf('_var_routeModels_8c1bc3ec'), 0x8cff0000);
    }

    private function setSizes(): void
    {
        foreach ([
            '_var_currentCourse_8c1bb868' => 0x50,
            '_var_routeModels_8c1bc3ec' => 4,
            '_init_8c04e988' => 4,
            '_init_8c04ce10' => 4,
            '_init_8c04df38' => 4,
            '_AsqRequestNj_11492' => 4,
            '_AsqRequestDat_11182' => 4,
            '_AsqRequestModels_12030' => 4,
        ] as $sym => $size) {
            $this->setSize($sym, $size);
        }
    }

    /**
     * SHC inlines strcpy to __slow_strcpy, which passes dst/src in R0/R1 (Rori).
     */
    private function expectStrcpy(int $dst, $src): void
    {
        if ($this->isAsmObject()) {
            $this->shouldCall('_strcpy')->with($dst, $src);
        } else {
            $this->shouldCall('__slow_strcpy')->with($dst, $src)
                ->using(new RoriCallingConvention());
        }
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
