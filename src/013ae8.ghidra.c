
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

