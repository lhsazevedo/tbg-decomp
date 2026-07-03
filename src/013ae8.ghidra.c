
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

