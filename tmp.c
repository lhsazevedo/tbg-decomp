
void FUN_8c028de8(int *param_1)

{
  int *piVar1;
  
  for (piVar1 = param_1; *piVar1 != -1; piVar1 = piVar1 + 3) {
    piVar1[2] = piVar1[2] + (int)param_1;
  }
  return;
}


void FUN_8c029ad4(int *param_1)

{
  int iVar1;
  char *pcVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  int iVar6;
  
  if (param_1 == (int *)0x0) {
    FUN_8c029cfe();
    return;
  }
  if (DAT_8c228408 != param_1) {
    FUN_8c029cfe();
    iVar4 = 0;
    DAT_8c228408 = param_1;
    for (; *param_1 != -1; param_1 = param_1 + 2) {
      iVar1 = *param_1;
      iVar6 = iVar4 * 0x18;
      puVar3 = &DAT_8c228288 + iVar4 * 6;
      if (iVar1 == 0) {
        puVar3 = &DAT_8c228290 + iVar4 * 6;
        pcVar2 = *(char **)(param_1[1] + 0xc);
LAB_8c029cc4:
        request_nj_8c011492(&DAT_8c18ad8c,pcVar2,puVar3,0);
      }
      else if (iVar1 == 1) {
        puVar5 = (undefined4 *)param_1[1];
        request_nj_8c011492(&DAT_8c18ad8c,(char *)*puVar5,&DAT_8c22828c + iVar4 * 6,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,(char *)puVar5[1],(uint)puVar3,(char)puVar5[2],0);
        request_dat_8c011182(&DAT_8c18ad8c,(char *)puVar5[3],&DAT_8c228290 + iVar4 * 6);
      }
      else if (iVar1 == 2) {
        puVar5 = (undefined4 *)param_1[1];
        request_nj_8c011492(&DAT_8c18ad8c,(char *)*puVar5,&DAT_8c22828c + iVar4 * 6,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,(char *)puVar5[1],(uint)puVar3,(char)puVar5[2],0);
        (&DAT_8c22829c)[iVar6] = *(undefined1 *)(puVar5 + 3);
        (&DAT_8c22829d)[iVar6] = *(undefined1 *)((int)puVar5 + 0xd);
        (&DAT_8c22829e)[iVar6] = *(undefined1 *)((int)puVar5 + 0xe);
        (&DAT_8c22829f)[iVar6] = *(undefined1 *)((int)puVar5 + 0xf);
      }
      else if (iVar1 == 3) {
        puVar5 = (undefined4 *)param_1[1];
        request_nj_8c011492(&DAT_8c18ad8c,(char *)*puVar5,&DAT_8c22828c + iVar4 * 6,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,(char *)puVar5[1],(uint)puVar3,(char)puVar5[2],0);
        request_nj_8c011492(&DAT_8c18ad8c,(char *)puVar5[3],&DAT_8c228290 + iVar4 * 6,0);
        (&DAT_8c22829c)[iVar6] = *(undefined1 *)(puVar5 + 4);
        (&DAT_8c22829d)[iVar6] = *(undefined1 *)((int)puVar5 + 0x11);
        (&DAT_8c22829e)[iVar6] = *(undefined1 *)((int)puVar5 + 0x12);
        (&DAT_8c22829f)[iVar6] = *(undefined1 *)((int)puVar5 + 0x13);
      }
      else if (iVar1 == 4) {
        puVar5 = (undefined4 *)param_1[1];
        request_nj_8c011492(&DAT_8c18ad8c,(char *)*puVar5,&DAT_8c22828c + iVar4 * 6,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,(char *)puVar5[1],(uint)puVar3,(char)puVar5[2],0);
        (&DAT_8c22829d)[iVar6] = (char)puVar5[3];
      }
      else if (iVar1 == 5) {
        puVar5 = (undefined4 *)param_1[1];
        request_nj_8c011492(&DAT_8c18ad8c,(char *)*puVar5,&DAT_8c22828c + iVar4 * 6,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,(char *)puVar5[1],(uint)puVar3,(char)puVar5[2],0);
        __quick_evn_mvn(&DAT_8c228294 + iVar6,puVar5 + 3,8);
      }
      else if (iVar1 == 6) {
        request_nj_8c011492(&DAT_8c18ad8c,"O_FUMI_00.njd",&DAT_8c22840c,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,"O_FUMI.pvm",0x8c228410,2,0);
        request_nj_8c011492(&DAT_8c18ad8c,"O_FUMI_00.njm",&DAT_8c228414,0);
        request_nj_8c011492(&DAT_8c18ad8c,"O_FUMI_01.njm",&DAT_8c228418,0);
        request_nj_8c011492(&DAT_8c18ad8c,"O_FUMI_LAMP.njd",&DAT_8c22841c,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,"O_FUMI_LAMP.pvm",0x8c228420,0x10,0);
        request_nj_8c011492(&DAT_8c18ad8c,"od_chu00.njd",&DAT_8c228424,0);
        requestPvm_8c011ac0(&DAT_8c18ad8c,"od_chu00.pvm",0x8c228428,0x10,0);
        request_nj_8c011492(&DAT_8c18ad8c,"o01_tra0.njm",&DAT_8c22842c,0);
        pcVar2 = "o01_tra1.njm";
        puVar3 = &DAT_8c228430;
        goto LAB_8c029cc4;
      }
      iVar4 = iVar4 + 1;
    }
  }
  return;
}


void FUN_8c029cfe(void)

{
  int iVar1;
  void *ptr;
  int iVar2;
  int *piVar3;
  
  if (DAT_8c228408 != (int *)0xffffffff) {
    iVar2 = 0;
    for (piVar3 = DAT_8c228408; *piVar3 != -1; piVar3 = piVar3 + 2) {
      iVar1 = *piVar3;
      if (iVar1 == 0) {
LAB_8c029dd8:
        ptr = (void *)(&DAT_8c228290)[iVar2 * 6];
LAB_8c029e20:
        _syFree(ptr);
      }
      else {
        if ((iVar1 == 1) || (iVar1 == 3)) {
          _syFree((void *)(&DAT_8c22828c)[iVar2 * 6]);
          _releaseAndFreeTexlist_8c011e3c((NJS_TEXLIST *)(&DAT_8c228288)[iVar2 * 6]);
          goto LAB_8c029dd8;
        }
        if (((iVar1 != 2) && (iVar1 != 4)) && (iVar1 != 5)) {
          if (iVar1 != 6) goto LAB_8c029e24;
          _syFree(DAT_8c22840c);
          _releaseAndFreeTexlist_8c011e3c(DAT_8c228410);
          _syFree(DAT_8c228414);
          _syFree(DAT_8c228418);
          _syFree(DAT_8c22841c);
          _releaseAndFreeTexlist_8c011e3c(DAT_8c228420);
          _syFree(DAT_8c228424);
          _releaseAndFreeTexlist_8c011e3c(DAT_8c228428);
          _syFree(DAT_8c22842c);
          ptr = DAT_8c228430;
          goto LAB_8c029e20;
        }
        _syFree((void *)(&DAT_8c22828c)[iVar2 * 6]);
        _releaseAndFreeTexlist_8c011e3c((NJS_TEXLIST *)(&DAT_8c228288)[iVar2 * 6]);
      }
LAB_8c029e24:
      iVar2 = iVar2 + 1;
    }
    DAT_8c228408 = (int *)0xffffffff;
  }
  return;
}


void FUN_8c028dd0(int *param_1)

{
  int *piVar1;
  
  for (piVar1 = param_1; *piVar1 != 0; piVar1 = piVar1 + 1) {
    *piVar1 = *piVar1 + (int)param_1;
  }
  return;
}


void FUN_8c02190a(void)

{
  undefined1 *ptr;
  undefined1 *puVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  char *pcVar5;
  int iVar6;
  int *piVar7;
  size_t size;
  int iStack_30;
  
  if (DAT_8c226534 == (char *)0xffffffff) {
    return;
  }
  size = *DAT_8c22650c * DAT_8c22650c[1];
  ptr = (undefined1 *)_syMalloc(size);
  iVar3 = 0;
  puVar1 = ptr;
  pcVar5 = DAT_8c226534;
  if (0 < (int)size) {
    do {
      iVar3 = iVar3 + 1;
      *puVar1 = 0;
      puVar1 = puVar1 + 1;
      pcVar5 = DAT_8c226534;
    } while (iVar3 < (int)size);
  }
  for (; (pcVar5[2] != '\0' || (pcVar5[3] != '\0')); pcVar5 = pcVar5 + 4) {
    for (iStack_30 = (int)pcVar5[1]; iStack_30 <= pcVar5[3]; iStack_30 = iStack_30 + 1) {
      for (iVar3 = (int)*pcVar5; iVar3 <= pcVar5[2]; iVar3 = iVar3 + 1) {
        iVar2 = iStack_30 * *DAT_8c22650c + iVar3;
        if (ptr[iVar2] == '\0') {
          piVar4 = &DAT_8c226520;
          iVar6 = iVar2 * 8;
          for (piVar7 = &DAT_8c226520; piVar7 < &DAT_8c226530; piVar7 = piVar7 + 1) {
            if (*(int *)(*piVar7 + iVar6) != 0) {
              _requestUnknownStruct_8c01181c(&DAT_8c18ad4c,*(undefined4 *)(*piVar4 + iVar6));
            }
            piVar4 = piVar4 + 1;
          }
          ptr[iVar2] = '\x01';
        }
      }
    }
  }
  _syFree(ptr);
  return;
}


void FUN_8c021a24(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int *piVar5;
  int iVar6;
  int local_38;
  int *local_28;
  
  iVar2 = *DAT_8c1bb8a4;
  iVar1 = DAT_8c1bb8a4[1];
  local_28 = &DAT_8c226520;
  for (local_38 = 0; local_38 < iVar1; local_38 = local_38 + 1) {
    for (iVar3 = 0; iVar3 < iVar2; iVar3 = iVar3 + 1) {
      piVar5 = local_28;
      for (uVar4 = 0; uVar4 < 0x10; uVar4 = uVar4 + 4) {
        iVar6 = local_38 * **(int **)((int)&DAT_8c22650c + uVar4) + iVar3;
        if (*(int *)(*(int *)((int)&DAT_8c226520 + uVar4) + iVar6 * 8) != 0) {
          iVar6 = iVar6 * 8;
          _njReleaseTexture(*(NJS_TEXLIST **)(*piVar5 + iVar6));
          _syFree(*(void **)(*piVar5 + iVar6));
          _syFree(*(void **)(*piVar5 + iVar6 + 4));
          *(undefined4 *)(*piVar5 + iVar6) = 0;
          *(undefined4 *)(iVar6 + *piVar5 + 4) = 0;
        }
        piVar5 = piVar5 + 1;
      }
      iVar6 = (local_38 * *DAT_8c22651c + iVar3) * 8;
      if (*(int *)(DAT_8c226530 + iVar6 + 4) != 0) {
        _syFree(*(void **)(DAT_8c226530 + iVar6 + 4));
        *(undefined4 *)(DAT_8c226530 + iVar6 + 4) = 0;
      }
    }
  }
  return;
}


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void task_pedestrians_8c0293f6(Task *task,void *state)

{
  int *piVar1;
  undefined4 *ptr;
  int iVar2;
  uint uVar3;
  code *pcVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  float *pfVar8;
  undefined4 in_fr5;
  ulonglong in_dr6;
  uint in_fr9;
  bool bVar9;
  int local_2c;
  int *local_28;
  undefined1 auStack_24 [8];
  
  if (DAT_8c2285c4 != 0) {
    DAT_8c2264d4 = DAT_8c1bb890;
    uVar3 = _DAT_8c1bbd8c & 0xff0000;
    if (task->gdfs_0x0c == 0) {
      if (uVar3 == 0) {
        task->gdfs_0x0c = 1;
      }
    }
    else if (uVar3 != 0) {
      DAT_8c22822c = (int)(short)(uVar3 >> 0x10);
      task->gdfs_0x0c = 0;
    }
    if (task->field2_0x8 != DAT_8c22822c) {
      task->field2_0x8 = DAT_8c22822c;
      iVar5 = 0;
      for (iVar6 = 0; iVar6 < DAT_8c228234; iVar6 = iVar6 + 1) {
        iVar7 = DAT_8c228230 + iVar5;
        iVar5 = iVar5 + 0xc;
        *(undefined4 *)(iVar7 + 4) = 0;
      }
      for (local_28 = *(int **)(DAT_8c228240 + DAT_8c22822c * 4); *local_28 != -1;
          local_28 = local_28 + 1) {
        iVar5 = *local_28;
        iVar6 = iVar5 * 0xc;
        if (*(int *)(DAT_8c228230 + iVar6) == 0) {
          ptr = (undefined4 *)_syMalloc(0x220);
          if (ptr == (undefined4 *)0x0) break;
          *(undefined4 **)(DAT_8c228230 + iVar6 + 8) = ptr;
          clearTasks_8c014a9c(ptr,0x10);
          iVar7 = _pushTask_8c014ae8((Task *)&DAT_8c1ba808,task_pedgroup_8c029078,&local_2c,
                                     auStack_24,0);
          if (iVar7 == 0) {
            _syFree(ptr);
            break;
          }
          for (iVar7 = 0; *(int *)(iVar7 * 0xc + DAT_8c22823c) != iVar5; iVar7 = iVar7 + 1) {
          }
          *(undefined4 **)(local_2c + 0x18) = ptr;
          *(undefined4 *)(local_2c + 0x1c) = *(undefined4 *)(DAT_8c22823c + iVar7 * 0xc + 8);
          *(int *)(local_2c + 8) = iVar5;
          *(undefined4 *)(local_2c + 0x10) = *(undefined4 *)(iVar7 * 0xc + DAT_8c22823c + 4);
          *(undefined4 *)(DAT_8c228230 + iVar6) = 1;
        }
        *(undefined4 *)(iVar6 + DAT_8c228230 + 4) = 1;
      }
    }
    FUN_8c02e486();
    FUN_8c02890c();
    _njCalcPoint((undefined8 *)&DAT_8c1bba54,(undefined8 *)&DAT_8c04650c,(undefined8 *)&DAT_8c1bc460
                );
    DAT_8c228268 = DAT_8c1bc460;
    DAT_8c22826c = DAT_8c1bc468;
    _njCalcPoint((undefined8 *)&DAT_8c1bba54,(undefined8 *)&DAT_8c046518,(undefined8 *)&DAT_8c1bc460
                );
    iVar7 = 0;
    DAT_8c228270 = DAT_8c1bc460;
    DAT_8c228274 = DAT_8c1bc468;
    DAT_8c228244 = &DAT_8c228248;
    iVar6 = 0;
    for (iVar5 = 0; iVar5 < DAT_8c228234; iVar5 = iVar5 + 1) {
      if (*(int *)(DAT_8c228230 + iVar6) != 0) {
        for (pfVar8 = *(float **)(DAT_8c228238 + iVar7); *pfVar8 != 0.0; pfVar8 = pfVar8 + 6) {
          if (((uint)pfVar8[5] & 0xfff) != 0) {
            iVar2 = FUN_8c0206f0(in_fr5,in_fr9,in_dr6,(double *)&DAT_8c228268,
                                 (double *)&DAT_8c228270,(double *)(pfVar8 + 1),
                                 (double *)(pfVar8 + 7),(double *)&DAT_8c1bc458);
            if (iVar2 != 0) {
              piVar1 = DAT_8c228244 + 1;
              *DAT_8c228244 = (int)pfVar8;
              DAT_8c228244 = piVar1;
              piVar1 = DAT_8c228244 + 1;
              *DAT_8c228244 = (int)(pfVar8 + 6);
              DAT_8c228244 = piVar1;
            }
          }
        }
      }
      iVar6 = iVar6 + 0xc;
      iVar7 = iVar7 + 0xc;
    }
    execTasks_8c014b42((Task *)&DAT_8c1ba808);
    bVar9 = _var_demo_8c1bb8d0 == 2;
    if (bVar9) {
      pcVar4 = FUN_8c028b74;
    }
    else {
      FUN_8c0223ea(0,FUN_8c028b74,0);
      pcVar4 = (code *)&LAB_8c028a38;
    }
    uVar3 = (uint)!bVar9;
    FUN_8c0223ea(uVar3,pcVar4,uVar3);
    if (DAT_8c2285e4 == 2) {
      FUN_8c0223ea(0,&LAB_8c02d06c,0);
      FUN_8c0223ea(1,&LAB_8c02d06c,1);
    }
  }
  return;
}

