
void inputTaskAlt_8c012718(void)

{
  int iVar1;
  undefined1 *puVar2;
  uint *puVar3;
  undefined **ppuVar4;
  undefined4 *puVar5;
  
  _var_resetRequested_8c157a78 = 0;
  _var_peripheral_8c1ba358 = (PDS_PERIPHERAL *)_pdGetPeripheral(0);
                    /* WARNING: Load size is inaccurate */
  puVar2 = (undefined1 *)(_var_peripheral_8c1ba358->support_0x04 & (uint)&DAT_000f06fe);
  if (((*_var_peripheral_8c1ba358->info & 1) == 0) ||
     ((puVar2 != &DAT_000f06fe && (puVar2 != &DAT_000700fe)))) {
    __quick_odd_mvn(&_const_peripheral_8c033318,_var_peripherals_8c1ba35c,0x34);
    _var_vibport_8c1ba354 = 0xffffffff;
    DAT_8c157a70 = (undefined1 *)0xffffffff;
    goto LAB_8c0128b8;
  }
  _var_peripherals_8c1ba35c[0].r = _var_peripheral_8c1ba358->field6_0x18;
  _var_peripherals_8c1ba35c[0].l = _var_peripheral_8c1ba358->field7_0x1a;
  _var_peripherals_8c1ba35c[0].x1 = _var_peripheral_8c1ba358->field8_0x1c;
  _var_peripherals_8c1ba35c[0].on = 0;
  _var_peripherals_8c1ba35c[0].press = 0;
  if (puVar2 == &DAT_000f06fe) {
    puVar5 = &DAT_8c03beb8;
    for (puVar3 = &DAT_8c03beb8; puVar3 < &DAT_8c03bef0; puVar3 = puVar3 + 2) {
      if ((_var_peripheral_8c1ba358->field2_0x8 & *puVar3) != 0) {
        _var_peripherals_8c1ba35c[0].on = _var_peripherals_8c1ba35c[0].on | puVar5[1];
      }
      puVar5 = puVar5 + 2;
    }
    puVar5 = &DAT_8c03beb8;
    for (puVar3 = &DAT_8c03beb8; puVar3 < &DAT_8c03bef0; puVar3 = puVar3 + 2) {
      if ((_var_peripheral_8c1ba358->field4_0x10 & *puVar3) != 0) {
        _var_peripherals_8c1ba35c[0].press = _var_peripherals_8c1ba35c[0].press | puVar5[1];
      }
      puVar5 = puVar5 + 2;
    }
    if (((_var_peripheral_8c1ba358->field4_0x10 & 8U) != 0) &&
       ((_var_peripheral_8c1ba358->field2_0x8 & 0x606U) == 0x606)) {
LAB_8c012876:
      _var_resetRequested_8c157a78 = 1;
    }
  }
  else if (puVar2 == &DAT_000700fe) {
    puVar5 = &DAT_8c03bf18;
    for (ppuVar4 = (undefined **)&DAT_8c03bf18; ppuVar4 < &PTR_s_FortyFive_8c03bf40;
        ppuVar4 = ppuVar4 + 2) {
      if ((_var_peripheral_8c1ba358->field2_0x8 & (uint)*ppuVar4) != 0) {
        _var_peripherals_8c1ba35c[0].on = _var_peripherals_8c1ba35c[0].on | puVar5[1];
      }
      puVar5 = puVar5 + 2;
    }
    puVar5 = &DAT_8c03bf18;
    for (ppuVar4 = (undefined **)&DAT_8c03bf18; ppuVar4 < &PTR_s_FortyFive_8c03bf40;
        ppuVar4 = ppuVar4 + 2) {
      if ((_var_peripheral_8c1ba358->field4_0x10 & (uint)*ppuVar4) != 0) {
        _var_peripherals_8c1ba35c[0].press = _var_peripherals_8c1ba35c[0].press | puVar5[1];
      }
      puVar5 = puVar5 + 2;
    }
    if (((_var_peripheral_8c1ba358->field4_0x10 & 8U) != 0) &&
       ((_var_peripheral_8c1ba358->field2_0x8 & 6U) == 6)) goto LAB_8c012876;
  }
  DAT_8c157a70 = puVar2;
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
LAB_8c0128b8:
  _vmsLcd_8c01c910();
  return;
}

