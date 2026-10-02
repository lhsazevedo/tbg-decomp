<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_sets_simple_light_from_xyz_global()
    {
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_var_busSimpleLightDir_8c227db8', 0xc);
        $this->initUint32($this->addressOf('_var_busSimpleLightDir_8c227db8') + 0, fdec(1.5));
        $this->initUint32($this->addressOf('_var_busSimpleLightDir_8c227db8') + 4, fdec(-2.25));
        $this->initUint32($this->addressOf('_var_busSimpleLightDir_8c227db8') + 8, fdec(3.0));

        $this->call('_setLightCallback_8c0283d4')->with(0);

        $this->shouldCall('_njCnkSetSimpleLight')->with(1.5, -2.25, 3.0);
    }
};
