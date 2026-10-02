<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // State offsets, matching ObjectsInstall_8c02a6ac's test.
    const ST_12 = 0x48;

    public function test_advances_dat_and_pushes_draw_calls(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $dat = $this->alloc(0x20);
        $this->initUint32($state + self::ST_12, $dat);

        $this->call('_rowDatTask_8c029fcc')->with($task, $state);

        $this->shouldCall('_advanceDatBlob_8c029f54')->with($dat)->andReturn(1);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawDatModel_8c029f2a'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawDatModel_8c029f2a'), $state);
    }
};
