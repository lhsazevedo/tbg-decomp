
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void task_8c012504(void)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  int *piVar4;
  undefined4 *puVar5;
  uint *puVar6;
  
  _var_resetRequested_8c157a78 = 0;
  _var_peripheral_8c1ba358 = (PDS_PERIPHERAL *)_pdGetPeripheral(0);
                    /* WARNING: Load size is inaccurate */
  puVar6 = &DAT_8c157a70;
  uVar2 = _var_peripheral_8c1ba358->support_0x04 & 0xf06fe;
  if (((*_var_peripheral_8c1ba358->info & 1) == 0) || ((uVar2 != 0xf06fe && (uVar2 != 0x700fe)))) {
    __quick_odd_mvn(&_const_peripheral_8c033318,_var_peripherals_8c1ba35c,0x34);
    _var_vibport_8c1ba354 = 0xffffffff;
    *puVar6 = 0xffffffff;
    goto LAB_8c0126e0;
  }
  _var_peripherals_8c1ba35c[0].r = _var_peripheral_8c1ba358->field6_0x18;
  _var_peripherals_8c1ba35c[0].l = _var_peripheral_8c1ba358->field7_0x1a;
  _var_peripherals_8c1ba35c[0].x1 = _var_peripheral_8c1ba358->field8_0x1c;
  _var_peripherals_8c1ba35c[0].on = 0;
  _var_peripherals_8c1ba35c[0].press = 0;
  if (uVar2 == 0xf06fe) {
    piVar4 = &INT_8c03be80;
    for (piVar3 = &INT_8c03be80; piVar3 < &DAT_8c03beb8; piVar3 = (int *)((uint *)piVar3 + 2)) {
      if ((_var_peripheral_8c1ba358->field2_0x8 & *piVar3) != 0) {
        _var_peripherals_8c1ba35c[0].on = _var_peripherals_8c1ba35c[0].on | piVar4[1];
      }
      piVar4 = piVar4 + 2;
    }
    piVar4 = &INT_8c03be80;
    for (piVar3 = &INT_8c03be80; piVar3 < &DAT_8c03beb8; piVar3 = (int *)((uint *)piVar3 + 2)) {
      if ((_var_peripheral_8c1ba358->field4_0x10 & *piVar3) != 0) {
        _var_peripherals_8c1ba35c[0].press = _var_peripherals_8c1ba35c[0].press | piVar4[1];
      }
      piVar4 = piVar4 + 2;
    }
    if (((_var_peripheral_8c1ba358->field4_0x10 & 8U) != 0) &&
       ((_var_peripheral_8c1ba358->field2_0x8 & 0x606U) == 0x606)) {
LAB_8c0126a0:
      _var_resetRequested_8c157a78 = 1;
    }
  }
  else if (uVar2 == 0x700fe) {
    puVar5 = &DAT_8c03bef0;
    for (puVar6 = &DAT_8c03bef0; puVar6 < &DAT_8c03bf18; puVar6 = puVar6 + 2) {
      if ((_var_peripheral_8c1ba358->field2_0x8 & *puVar6) != 0) {
        _var_peripherals_8c1ba35c[0].on = _var_peripherals_8c1ba35c[0].on | puVar5[1];
      }
      puVar5 = puVar5 + 2;
    }
    puVar5 = &DAT_8c03bef0;
    for (puVar6 = &DAT_8c03bef0; puVar6 < &DAT_8c03bf18; puVar6 = puVar6 + 2) {
      if ((_var_peripheral_8c1ba358->field4_0x10 & *puVar6) != 0) {
        _var_peripherals_8c1ba35c[0].press = _var_peripherals_8c1ba35c[0].press | puVar5[1];
      }
      puVar5 = puVar5 + 2;
    }
    if ((((_var_peripherals_8c1ba35c[0].press & 0x200) == 0) || (DAT_8c1bbc4c != 0.0)) ||
       (_var_peripherals_8c1ba35c[0].l < 0x81)) {
      if (((_var_peripheral_8c1ba358->field4_0x10 & 8U) != 0) &&
         ((_var_peripheral_8c1ba358->field2_0x8 & 6U) == 6)) goto LAB_8c0126a0;
    }
    else if (DAT_8c1bbcc4 == 5) {
      _var_peripherals_8c1ba35c[0].press = _var_peripherals_8c1ba35c[0].press ^ 0x210;
    }
    else if (DAT_8c1bbcc4 == 0) {
      _var_peripherals_8c1ba35c[0].press = _var_peripherals_8c1ba35c[0].press ^ 0x220;
    }
  }
  DAT_8c157a70 = uVar2;
  iVar1 = _pdGetPeripheral(1);
  if ((**(uint **)(iVar1 + 0x30) & 0x100) == 0) {
    iVar1 = _pdGetPeripheral(2);
    if ((**(uint **)(iVar1 + 0x30) & 0x100) == 0) {
      _var_vibport_8c1ba354 = 0xffffffff;
    }
    else {
      _var_vibport_8c1ba354 = 2;
    }
  }
  else {
    _var_vibport_8c1ba354 = 1;
  }
LAB_8c0126e0:
  _vmsLcd_8c01c910();
  return;
}

