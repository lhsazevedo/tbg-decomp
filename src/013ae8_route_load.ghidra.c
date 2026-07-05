
void request_some_files_8c013ae8(void)

{
  request_nj_8c011492(&DAT_8c18ad6c,"front.njd",&DAT_8c1bc434,0);
  requestPvm_8c011ac0(&DAT_8c18ad6c,"front.pvm",(uint)&DAT_8c1bc430,0xf,0);
  request_nj_8c011492(&DAT_8c18ad6c,"syanai.njd",&DAT_8c1bc43c,0);
  requestPvm_8c011ac0(&DAT_8c18ad6c,"syanai.pvm",(uint)&DAT_8c1bc438,0x40,0);
  requestPvm_8c011ac0(&DAT_8c18ad6c,"mark.pvm",(uint)&DAT_8c1bc418,3,0);
  requestPvm_8c011ac0(&DAT_8c18ad6c,"busstop.pvm",(uint)&DAT_8c1bc424,1,0);
  DAT_8c1bc3f4 = _requestNjPvmPairs_8c012030(&DAT_8c18ad6c,&PTR_s_3s_2do_x.njd_8c043d64,0);
  return;
}


void freeVehicleAssets_8c013b5a(void)

{
  uint uVar1;
  
  if (DAT_8c1bc438 != (NJS_TEXLIST *)0xffffffff) {
    _njSetTexture((int)DAT_8c1bc438);
    for (uVar1 = 0; uVar1 < DAT_8c1bc438->nbTextures; uVar1 = uVar1 + 1) {
      _njReleaseCacheTextureNum(uVar1);
    }
    _releaseAndFreeTexlist_8c011e3c(DAT_8c1bc438);
    _syFree(DAT_8c1bc43c);
    _releaseAndFreeTexlist_8c011e3c(DAT_8c1bc430);
    _syFree(DAT_8c1bc434);
    _releaseAndFreeTexlist_8c011e3c(DAT_8c1bc418);
    _releaseAndFreeTexlist_8c011e3c(DAT_8c1bc424);
    DAT_8c1bc438 = (NJS_TEXLIST *)0xffffffff;
  }
  return;
}


void FUN_8c013bbc(uint param_1,int param_2)

{
  uint uVar1;
  
  for (uVar1 = (param_2 + -1) * 0x10 + param_1; param_1 <= uVar1; uVar1 = uVar1 - 0x10) {
    *(undefined4 *)(uVar1 + 8) = 0xffffffff;
  }
  return;
}


void FUN_8c013c34(char *param_1)

{
  int iVar1;
  undefined **ppuVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  int *local_24;
  
  puVar3 = &DAT_8c1bbddc;
  do {
    *puVar3 = 0;
    puVar3[1] = 0;
    puVar3 = puVar3 + 4;
  } while (puVar3 < &DAT_8c1bbfdc);
  while (iVar1 = (int)*param_1, iVar1 != -1) {
    (&DAT_8c1bbddc)[iVar1 * 4] = 1;
    param_1 = param_1 + 1;
    if ((&DAT_8c1bbde4)[iVar1 * 4] == -1) {
      (&DAT_8c1bbde0)[iVar1 * 4] = 1;
    }
  }
  local_24 = &DAT_8c1bbddc;
  puVar3 = &DAT_8c1bbddc;
  iVar1 = 0;
  puVar4 = &DAT_8c1bbddc;
  do {
    if ((&DAT_8c1bbde0)[iVar1 * 4] == 0) {
      if ((*local_24 == 0) && (puVar3[2] != -1)) {
        _releaseAndFreeTexlist_8c011e3c((NJS_TEXLIST *)puVar4[2]);
        _syFree((void *)puVar4[3]);
        puVar4[2] = 0xffffffff;
      }
    }
    else {
      if (DAT_8c18ad20 == 2) {
        request_nj_8c011492(&DAT_8c18ad6c,(&PTR_s_3s_2do0_l.njd_8c043ecc)[iVar1 * 2],(void *)0x0,
                            (int)(&DAT_8c1bbde8 + iVar1 * 4));
        ppuVar2 = &PTR_s_3s_2do0_l.njd_8c043ecc;
      }
      else {
        request_nj_8c011492(&DAT_8c18ad6c,(&PTR_s_3s_2do0_l.njd_8c043dc4)[iVar1 * 2],(void *)0x0,
                            (int)(&DAT_8c1bbde8 + iVar1 * 4));
        ppuVar2 = &PTR_s_3s_2do0_l.njd_8c043dc4;
      }
      requestPvm_8c011ac0(&DAT_8c18ad6c,ppuVar2[iVar1 * 2 + 1],(uint)(&DAT_8c1bbde4 + iVar1 * 4),
                          0x60,0);
    }
    iVar1 = iVar1 + 1;
    puVar4 = puVar4 + 4;
    puVar3 = puVar3 + 4;
    local_24 = local_24 + 4;
  } while (iVar1 < 0x20);
  return;
}


void FUN_8c013d42(void)

{
  _setUknPvmBool_8c014330();
  _AsqFreeQueues_11f7e();
  return;
}



void FUN_8c013d78(void)

{
  _AsqInitQueues_11f36(0,0x40,0,0x40);
  _AsqResetQueues_11f6c();
  _resetUknPvmBool_8c014322();
  FUN_8c013c34(DAT_8c18adb0);
  _AsqProcessQueues_11fe0(_nop_8c011120,0,0,0,FUN_8c013d42);
  return;
}



void FUN_8c013dae(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = &DAT_8c1bbddc;
  puVar2 = &DAT_8c1bbddc;
  do {
    if (puVar1[2] != -1) {
      _releaseAndFreeTexlist_8c011e3c((NJS_TEXLIST *)puVar2[2]);
      _syFree((void *)puVar2[3]);
      puVar2[2] = 0xffffffff;
    }
    puVar1 = puVar1 + 4;
    puVar2 = puVar2 + 4;
  } while (puVar1 < &DAT_8c1bbfdc);
  return;
}


void FUN_8c013df6(char *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int *piVar3;
  undefined4 *puVar4;
  
  puVar2 = &DAT_8c1bbfdc;
  do {
    *puVar2 = 0;
    puVar2[1] = 0;
    puVar2 = puVar2 + 4;
  } while (puVar2 < &DAT_8c1bc3ec);
  while (iVar1 = (int)*param_1, iVar1 != -1) {
    (&DAT_8c1bbfdc)[iVar1 * 4] = 1;
    param_1 = param_1 + 1;
    if ((&DAT_8c1bbfe4)[iVar1 * 4] == -1) {
      (&DAT_8c1bbfe0)[iVar1 * 4] = 1;
    }
  }
  puVar2 = &DAT_8c1bbfdc;
  piVar3 = &DAT_8c1bbfdc;
  iVar1 = 0;
  puVar4 = &DAT_8c1bbfdc;
  do {
    if ((&DAT_8c1bbfe0)[iVar1 * 4] == 0) {
      if ((*piVar3 == 0) && (puVar2[2] != -1)) {
        _releaseAndFreeTexlist_8c011e3c((NJS_TEXLIST *)puVar4[2]);
        puVar4[2] = 0xffffffff;
      }
    }
    else {
      requestPvm_8c011ac0(&DAT_8c18ad6c,(&PTR_s_MOV01.pvm_8c043fd8)[iVar1],
                          (uint)(&DAT_8c1bbfe4 + iVar1 * 4),2,0);
    }
    iVar1 = iVar1 + 1;
    puVar2 = puVar2 + 4;
    puVar4 = puVar4 + 4;
    piVar3 = piVar3 + 4;
  } while (iVar1 < 0x41);
  return;
}


void FUN_8c013ee4(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = &DAT_8c1bbfdc;
  puVar2 = &DAT_8c1bbfdc;
  do {
    if (puVar1[2] != -1) {
      _releaseAndFreeTexlist_8c011e3c((NJS_TEXLIST *)puVar2[2]);
      puVar2[2] = 0xffffffff;
    }
    puVar1 = puVar1 + 4;
    puVar2 = puVar2 + 4;
  } while (puVar1 < &DAT_8c1bc3ec);
  return;
}


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8c013f22(void)

{
  int iVar1;
  
  iVar1 = *(int *)(DAT_8c18ad18 + 8) + _DAT_8c228708 * 0x2c;
  if (*(int *)(iVar1 + 0x28) != 0) {
    FUN_8c0120fe(&DAT_8c1bc3f0);
  }
  if (*(int *)(iVar1 + 0xc) != 0) {
    FUN_8c021a24();
    return;
  }
  return;
}


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8c013f78(void)

{
  char *filename;
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = *(int *)(DAT_8c18ad18 + 8) + _DAT_8c228708 * 0x2c;
  if (*(int *)(iVar3 + 0x24) != 0) {
    DAT_8c18ad28 = *(int **)(iVar3 + 0x24);
    DAT_8c226504 = *DAT_8c18ad28 + -1;
    DAT_8c226508 = (int)(DAT_8c226504 + (uint)(DAT_8c226504 < 0)) >> 1;
    DAT_8c227dd0 = DAT_8c18ad28[1];
  }
  if (*(int *)(iVar3 + 0x28) != 0) {
    DAT_8c1bc3f0 = _requestNjPvmPairs_8c012030(&DAT_8c18ad6c,*(undefined4 *)(iVar3 + 0x28),0x10);
  }
  if ((DAT_8c1bb900 == 0) || (_var_demo_8c1bb8d0 != 0)) {
    if (*(int *)(iVar3 + 0x10) != 0) {
      DAT_8c18adb0 = *(undefined4 *)(iVar3 + 0x10);
      FUN_8c013c34(*(char **)(iVar3 + 0x10));
    }
  }
  else {
    if (*(int *)(iVar3 + 0x10) != 0) {
      DAT_8c18adb0 = *(undefined4 *)(iVar3 + 0x10);
    }
    FUN_8c013c34(&DAT_8c043fd4);
  }
  if (*(int *)(iVar3 + 0x18) != 0) {
    FUN_8c013df6(*(char **)(iVar3 + 0x18));
  }
  FUN_8c029ad4(*(undefined4 *)(iVar3 + 0x1c));
  if (*(int *)(iVar3 + 0xc) == 0) {
    DAT_8c226534 = 0xffffffff;
  }
  else {
    DAT_8c226534 = *(undefined4 *)(iVar3 + 0xc);
    iVar2 = 0;
    puVar1 = *(undefined4 **)(iVar3 + 0x20);
    do {
      filename = (char *)*puVar1;
      puVar1 = puVar1 + 1;
      request_dat_8c011182(&DAT_8c18ad2c,filename,&DAT_8c18adb4 + iVar2);
      iVar2 = iVar2 + 1;
    } while (iVar2 < 4);
  }
  if ((DAT_8c1bb900 != 0) && (_var_demo_8c1bb8d0 == 0)) {
    FUN_8c02aa36();
    return;
  }
  return;
}


void load_route_models_8c014088(void)

{
  char *pcVar1;
  char *puVar12;
  
  DAT_8c18ad18 = (int *)(&PTR_INT_8c043ca4)[DAT_8c1bb868];
  DAT_8c18ad1c = *DAT_8c18ad18;
  DAT_8c18ad20 = DAT_8c18ad18[1];
  DAT_8c18ad24 = DAT_8c18ad18[4];
  if (DAT_8c18ad1c == 0) {
    if ((DAT_8c18ad20 == 0) || (DAT_8c18ad20 == 1)) {
      _strcpy(&DAT_8c18ad6c,"\\SD_COMMON");
      pcVar1 = "\\SD_PVR";
    }
    else {
      if (DAT_8c18ad20 != 2) goto LAB_8c0141e4;
      _strcpy(&DAT_8c18ad6c,"\\SN_COMMON");
      pcVar1 = "\\SN_PVR";
    }
    _strcpy(&DAT_8c18ad4c,pcVar1);
  }
  else if (DAT_8c18ad1c == 1) {
    if ((DAT_8c18ad20 == 0) || (DAT_8c18ad20 == 1)) {
      _strcpy(&DAT_8c18ad6c,"\\WD_COMMON");
      pcVar1 = "\\WD_PVR";
    }
    else {
      if (DAT_8c18ad20 != 2) goto LAB_8c0141e4;
      _strcpy(&DAT_8c18ad6c,"\\WN_COMMON");
      pcVar1 = "\\WN_PVR";
    }
    _strcpy(&DAT_8c18ad4c,pcVar1);
  }
  else if (DAT_8c18ad1c == 2) {
    if ((DAT_8c18ad20 == 0) || (DAT_8c18ad20 == 1)) {
      _strcpy(&DAT_8c18ad6c,"\\OD_COMMON");
      pcVar1 = "\\OD_PVR";
    }
    else {
      if (DAT_8c18ad20 != 2) goto LAB_8c0141e4;
      _strcpy(&DAT_8c18ad6c,"\\ON_COMMON");
      pcVar1 = "\\ON_PVR";
    }
    _strcpy(&DAT_8c18ad4c,pcVar1);
  }
LAB_8c0141e4:
  _strcpy(&DAT_8c18ad8c,&DAT_8c18ad6c);
  _strcpy(&DAT_8c18ad2c,&DAT_8c18ad6c);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[7],&DAT_8c1bb86c,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[8],&DAT_8c1bb870,0);
  DAT_8c1bb874 = DAT_8c18ad18[9];
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[10],&DAT_8c1bb878,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0xb],&DAT_8c1bb87c,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0xc],&DAT_8c1bb880,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0xd],&DAT_8c1bb884,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0xe],&DAT_8c1bb888,0);
  request_dat_8c011182(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0xf],&DAT_8c1bb88c);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x10],&DAT_8c1bb890,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x11],(void *)0x0,-0x73e4476c);
  request_dat_8c011182(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x12],&DAT_8c1bb898);
  request_dat_8c011182(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x13],&DAT_8c1bb89c);
  request_dat_8c011182(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x14],&DAT_8c1bb8a0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x15],&DAT_8c1bb8a4,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x16],&DAT_8c1bb8a8,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x17],&DAT_8c1bb8ac,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x18],&DAT_8c1bb8b0,0);
  request_nj_8c011492(&DAT_8c18ad6c,(char *)DAT_8c18ad18[0x19],&DAT_8c1bb8b4,0);
  request_some_files_8c013ae8();
  DAT_8c1bc3ec = _requestNjPvmPairs_8c012030(&DAT_8c18ad6c,&PTR_s_signal00.njd_8c0440dc,0x10);
  return;
}

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _resetUknPvmBool_8c014322(void)

{
  _DAT_8c18adac = 0;
  return;
}



/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 _getUknPvmBool_8c01432a(void)

{
  return _DAT_8c18adac;
}



/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _setUknPvmBool_8c014330(void)

{
  _DAT_8c18adac = 1;
  return;
}


void task_load_8c014338(Task *task,void *state)

{
  int iVar1;
  undefined1 *puVar2;
  uint *puVar3;
  undefined **ppuVar4;
  undefined4 *puVar5;
  undefined4 in_fr5;
  undefined4 in_fr7;
  float x;
  
  iVar1 = task->field2_0x8;
  if (iVar1 == 0) {
    _AsqResetQueues_11f6c();
    _njSetTexture((int)DAT_8c1bc3f8);
    _njLoadCacheTexture();
    load_route_models_8c014088();
    _resetUknPvmBool_8c014322();
    _AsqProcessQueues_11fe0(_nop_8c011120,0,0,0,_setUknPvmBool_8c014330);
LAB_8c0143ee:
    task->field2_0x8 = task->field2_0x8 + 1;
    return;
  }
  if (iVar1 == 1) {
    iVar1 = _getUknPvmBool_8c01432a();
    if (iVar1 != 0) {
      FUN_8c02175a();
      FUN_8c026da4(DAT_8c1bb88c);
      FUN_8c028de8(DAT_8c1bb898);
      FUN_8c028dd0(DAT_8c1bb89c);
      FUN_8c02caba(in_fr5,in_fr7);
      FUN_8c02b170();
      _AsqResetQueues_11f6c();
      FUN_8c013f78();
      _resetUknPvmBool_8c014322();
      _AsqProcessQueues_11fe0(_nop_8c011120,FUN_8c021810,FUN_8c02190a,0,_setUknPvmBool_8c014330);
      task->field2_0x8 = task->field2_0x8 + 1;
    }
LAB_8c0144b4:
    x = 0.0;
    drawSprite_8c014f54((ResourceGroup *)&DAT_8c1bc3f8,0,0.0,0.0,-5.0);
    iVar1 = task->gdfs_0x0c;
    task->gdfs_0x0c = iVar1 + 1;
    iVar1 = __modls(iVar1 >> 2,6);
    drawSprite_8c014f54((ResourceGroup *)&DAT_8c1bc3f8,iVar1 + 1,x,x,-4.0);
    return;
  }
  if (iVar1 == 2) {
    iVar1 = _getUknPvmBool_8c01432a();
    if (iVar1 != 0) goto LAB_8c0143ee;
    goto LAB_8c0144b4;
  }
  if (iVar1 == 3) {
    task->field2_0x8 = task->field2_0x8 + 1;
    return;
  }
  if (iVar1 != 4) goto LAB_8c0144b4;
  freeTask_8c014b66(task);
  _AsqFreeQueues_11f7e();
  DAT_8c157a6c = 0;
  _njReleaseTexture(DAT_8c1bc3f8);
  FUN_8c01306e();
  if (DAT_8c1bb8c8 == 0) {
    task_8c012504();
    return;
  }
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

void FUN_8c0144fc(undefined4 param_1)

{
  int iStack_10;
  undefined1 auStack_c [8];
  
  _njSetBackColor(param_1,0xff418dff,0xff418dff,0xff418dff);
  DAT_8c157a6c = 1;
  _pushTask_8c014ae8((Task *)&_var_tasks_8c1ba3c8,task_load_8c014338,&iStack_10,auStack_c,0);
  *(undefined4 *)(iStack_10 + 8) = 0;
  *(undefined4 *)(iStack_10 + 0xc) = 0;
  _njGarbageTexture((int *)&_var_tex_8c157af8,0xc00);
  _AsqInitQueues_11f36(0x20,0x800,0x800,0x40);
  return;
}


void FUN_8c01468e(undefined4 param_1)

{
  int iStack_10;
  undefined1 auStack_c [8];
  
  if (((DAT_8c1ba290 < '\x02') || (_var_demo_8c1bb8d0 == 1)) &&
     (DAT_8c2285d0 = DAT_8c2285d0 + 0x1e, DAT_8c2285d4 < DAT_8c2285d0)) {
    DAT_8c2285d0 = DAT_8c2285d4;
  }
  DAT_8c157a6c = 1;
  _pushTask_8c014ae8((Task *)&_var_tasks_8c1ba3c8,&LAB_8c014550,&iStack_10,auStack_c,0);
  *(undefined4 *)(iStack_10 + 8) = 0;
  *(undefined4 *)(iStack_10 + 0xc) = 0;
  FUN_8c013f22();
  _njGarbageTexture((int *)&_var_tex_8c157af8,0xc00);
  _AsqInitQueues_11f36(0x20,0x800,0x800,0x40);
  _njSetTexture(DAT_8c1bc3f8);
  _njLoadCacheTexture();
  _njSetBackColor(param_1,0xff418dff,0xff418dff,0xff418dff);
  return;
}


void prob_task_8c014784(undefined4 param_1,undefined4 param_2,Task *param_3)

{
  int iVar1;
  undefined1 *puVar2;
  uint *puVar3;
  undefined **ppuVar4;
  undefined4 *puVar5;
  float x;
  
  iVar1 = param_3->field2_0x8;
  if (iVar1 == 0) {
    _AsqResetQueues_11f6c();
    _njSetTexture((int)DAT_8c1bc3f8);
    _njLoadCacheTexture();
    load_route_models_8c014088();
    _resetUknPvmBool_8c014322();
    _AsqProcessQueues_11fe0(_nop_8c011120,0,0,0,_setUknPvmBool_8c014330);
  }
  else {
    if (iVar1 != 1) {
      if (iVar1 == 2) {
        iVar1 = _getUknPvmBool_8c01432a();
        if (iVar1 != 0) {
          param_3->field2_0x8 = param_3->field2_0x8 + 1;
          return;
        }
        goto LAB_8c014882;
      }
      if (iVar1 == 3) {
        param_3->field2_0x8 = param_3->field2_0x8 + 1;
        return;
      }
      if (iVar1 != 4) goto LAB_8c014882;
      freeTask_8c014b66(param_3);
      _AsqFreeQueues_11f7e();
      DAT_8c157a6c = 0;
      _njReleaseTexture(DAT_8c1bc3f8);
      _njSetTexture(DAT_8c1bc438);
      _njLoadCacheTexture();
      FUN_8c01306e();
      if (DAT_8c1bb8c8 == 0) {
        task_8c012504();
        return;
      }
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
    iVar1 = _getUknPvmBool_8c01432a();
    if (iVar1 == 0) goto LAB_8c014882;
    FUN_8c02175a();
    FUN_8c026da4(DAT_8c1bb88c);
    FUN_8c028de8(DAT_8c1bb898);
    FUN_8c028dd0(DAT_8c1bb89c);
    FUN_8c02caba(param_1,param_2);
    FUN_8c02b170();
    _AsqResetQueues_11f6c();
    FUN_8c013f78();
    _resetUknPvmBool_8c014322();
    _AsqProcessQueues_11fe0(_nop_8c011120,0,FUN_8c02190a,0,_setUknPvmBool_8c014330);
  }
  param_3->field2_0x8 = param_3->field2_0x8 + 1;
LAB_8c014882:
  x = 0.0;
  drawSprite_8c014f54((ResourceGroup *)&DAT_8c1bc3f8,0,0.0,0.0,-5.0);
  iVar1 = param_3->gdfs_0x0c;
  param_3->gdfs_0x0c = iVar1 + 1;
  iVar1 = __modls(iVar1 >> 2,6);
  drawSprite_8c014f54((ResourceGroup *)&DAT_8c1bc3f8,iVar1 + 1,x,x,-4.0);
  return;
}



