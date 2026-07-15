# 0129cc decomp progress

## FUN_8c0129cc  (steering/prompt state machine, returns Bool)
- [x] branch: bb8cc==0 activation ((press&8 || ctrl==-1) && 8c22847c==0) -> set flags, return 1
- [x] branch: ad04==1 -> ad04=2, return 0 ; ad04==2 -> return 0
- [x] branch: press&8 -> deactivate, base draw
- [x] tail: base draw 0x74 @ -1.1 + njDrawPolygon (draw_base label)
- [x] branch: ad10==0 steer-init (sprite 0x75): press&4 deactivate | (press&0x20||y1>0x40) engage ad10=1/ad08=0/ad0c=1/midi(1,3,0) | idle
- [x] branch: press&8==0 && ad10!=0 dispatch on ad08 (below)
- [x] branch: ad08==0 (sprite 0x7a): press&4 -> ad08=1/midi(1,0,0) | (press&0x10||y1<-0x40) -> ad10=0/midi(1,3,0) | idle
- [x] branch: ad08==1 (sprite 0x76/0x77/0x7a): press&2 -> ad08=0/draw0x7a/midi(1,1,0) | ad0c==0{press&4 -> ad08=2/fadeout(10)/midi(1,0,0) no draw | (press&0x80||x1>0x40) toggle ad0c=1/draw0x76} | ad0c!=0{press&4 -> ad08=0/midi(1,0,0) | (press&0x40||x1<-0x40) toggle ad0c=0 | draw0x77}
- [x] branch: ad08==2 isFading==0, playMode!=PRACTICE: reset bb8dc/b8/bc=0, FUN_8c016182(), copy var_8c1ba2b8->progress.field_0x04[5] & var_8c1ba2cc->progress.field_0x18[5], CourseMenuFUN_8c017ef2(), return 0
- [x] branch: ad08==2 isFading==0, playMode==PRACTICE: ...FUN_8c016182(), menuState.selected_0x38 = var_8c22640c, FUN_8c01f21c(), return 0
- [x] branch: ad08==2 isFading!=0: draw 0x76 + njDrawPolygon, return 0 (skips base draw)

**FUN_8c0129cc DONE -- c.obj coverage 100%, 24 tests.**
**Refactored to switch(ad08)+guard clauses; same side effects/order, 24 tests still green. Backup: scratchpad 0129cc.c.pre-refactor.bak. Naming suggestions in function docblock.**
**IDENTIFIED: this is the in-drive PAUSE menu (CONTINUE / RETIRE, RETIRE has YES/NO confirm). njDrawPolygon = dimmed backdrop quad. press bits now use PDD_DGT_* macros (ST=8 A=4 B=2 KU/KD/KL/KR=0x10/20/40/80). Marks: 0x74 base, 0x75 CONTINUE, 0x7a RETIRE, 0x76 confirm YES, 0x77 confirm NO. State: ad10 0=CONTINUE/1=RETIRE, ad08 0=idle/1=confirm/2=fading, ad0c 0=YES/1=NO(default). ad08==2 -> CourseMenuFUN (leave drive) or FUN_8c01f21c (practice).**
**Renamed vars (kept 8c<addr> suffix; sed'd across src+tests, incl. asm .src owners 012f44.src/014f54_text_pre_data.src and PHP `_var_...` strings): var_8c1bb8cc->var_pauseActive_8c1bb8cc, var_8c18ad04->var_pauseSettle_8c18ad04, var_8c18ad10->var_onRetire_8c18ad10, var_8c18ad08->var_retirePhase_8c18ad08, var_8c18ad0c->var_confirmChoice_8c18ad0c. Added private #defines in 0129cc.c: MARK_BASE/CONTINUE/RETIRE/CONFIRM_YES/CONFIRM_NO, MARK_Z_ARROW/MARK_Z_BASE, STICK_THRESHOLD, RETIRE_PHASE_IDLE/CONFIRM/FADING, CONFIRM_YES/CONFIRM_NO. Full suite still 284/284.**

## task_8c012cbc  DONE
- pushTask_8c014ae8 action installed by routeLoadTask_8c014338 -> FUN_8c01306e -> pushTask_8c014ae8.
- reset-requested guard (var_resetRequested_8c157a78 && !var_8c157a7c && !var_queuesAreInitialized_8c157a60)
  -> FUN_8c016182() + pushTitle_8c015fd6(1), return. Same guard reused verbatim in
  task_8c012d06/task_8c012d5a below.
- else: call FUN_8c0129cc() (the pause menu); if it just opened (returns 1), reset the
  render-command list (FUN_8c02239c, unit 0222dc -- new minimal header) and run the rest
  of the frame's tasks (execTasks_8c014b42(var_tasks_8c1ba5e8)) so the paused frame still draws,
  then FUN_8c022560() (unowned no-arg fn, already in 022464.h).
- No params read (Ghidra shows void(void) even though it's called as a TaskAction);
  matches existing precedent (PspTask_8c012324, demoInputTask_8c016bf4) -- declared as
  `void task_8c012cbc()` (empty parens, not `(void)`) since pushTask_8c014ae8's `action`
  param is untyped `void *` so no prototype-matching is enforced at the call site.
- 0129cc.h previously declared this as `extern TaskAction task_8c012cbc;` (a function-pointer
  *variable* decl) -- that's fine only while undecompiled/opaque; once given a real body in
  this TU, SHC enforces type consistency against the header (same TU sees both), so it had
  to become a proper `void task_8c012cbc();` function declaration. task_8c012d06/d5a still
  use the TaskAction placeholder form until decompiled.
- Test note: FUN_8c0129cc is a real symbol defined in this same object -- no setSize() for
  it (that's only for external/stub symbols); shouldCall()->andReturn() works directly on it.
- Ghidra note: splitting the single "ram" memory-map block into bin/bss areas (and disabling
  Write on it) fixed most of the literal-pool PTR_FUN_* garbage across this whole unit --
  much more effective than manually retyping/renaming individual PTR_ slots. task_8c012cbc
  and task_8c012d06 now decompile cleanly; task_8c012d5a still shows FPSCR.SZ paired-float
  register noise around its drawSprite_8c014f54 calls (separate, known SH4 float-pair
  calling-convention artifact, unrelated to the memory-map fix).

## task_8c012d06  DONE
- Installed by FUN_8c01306e (unit 012f44) in place of task_8c012cbc when
  var_playMode_8c1bb8d0 == PLAY_MODE_DEMO (attract loop) and var_8c1bb8d4 == 0
  (the other branch installs task_8c012d5a instead). Different task list too:
  var_tasks_8c1ba3c8, not var_tasks_8c1ba5e8 (that one's still used as the arg
  to execTasks_8c014b42 inside the body though, same as task_8c012cbc).
- Same reset-requested guard verbatim, reused a third time.
- No full pause menu here: Start (PDD_DGT_ST) just toggles var_pauseActive_8c1bb8cc
  (XOR 1), no FUN_8c0129cc call at all. Frame's remaining tasks
  (FUN_8c02239c render-list reset + execTasks_8c014b42) only run while
  var_pauseActive_8c1bb8cc == 0 (i.e. right after toggling on, tasks are
  skipped for that frame; right after toggling off, they run immediately).
  FUN_8c022910 (owned by 022464, header already existed for FUN_8c022560 --
  added FUN_8c022910 alongside it) always runs regardless of pause state.
- var_peripheral_8c1ba358 is `PDS_PERIPHERAL *`; press field at offset 0x10,
  already declared in 014f54_text_pre_data.h. PDD_DGT_ST is the same Start-button
  bit used throughout FUN_8c0129cc.
- Test note: peripheral struct built with alloc(0x34) + initUint32 at +0x10,
  matching the existing pattern in 8c0129cc_FUN.php (not setSize -- it's a
  plain data blob, not an external symbol/relocation target). Toggle assertions
  use shouldWriteTo('_var_pauseActive_8c1bb8cc', ...) placed in the same
  ordered-expectation position as the real write (before the subsequent calls),
  per the single ordered expectations queue.

## task_8c012d5a  DONE
- Installed by FUN_8c01306e (unit 012f44) for PLAY_MODE_DEMO when var_8c1bb8d4 != 0
  (the ending sequence: attract loop plays out then returns to title). Unlike
  task_8c012d06 this one USES its task param -- takes `Task *task` (r4), reads
  task->field_0x08 (phase 0/1/2) and task->field_0x0c (frame counter). Ghidra
  mislabels the param as param_2 and buries the real signature under FPSCR.SZ
  paired-float noise; the asm (MOV R4,R14 then MOV.L @(8,R14)) is unambiguous.
- Same reset guard, but the reset action differs: FUN_8c016182() then
  init_8c03bd80 = 1 / init_8c03bd84 = 0 (both in 0100bc_sound.h), NOT pushTitle.
- Phase 0: Start (var_peripherals_8c1ba35c[0].press & PDD_DGT_ST -- the REMAPPED
  logical buttons array, not var_peripheral_8c1ba358) -> midi + phase=1; else
  counter++ and at >0x708 (0x709) -> phase=2; both fade paths run
  startAdxFadeOut(0/1) + push_fadeout(0x1e). Counter-before-timeout skips the
  fades (the one goto in the function -> shared draw tail).
- Phase 1/2: once var_isFading_8c226568 clears, unless init_8c03bd80 is set
  (reset pending -> do nothing), FUN_8c016182() + pushTitle_8c015fd6(1) for
  phase 1 (Start skip) / (0) for phase 2 (timeout). While still fading: counter++
  and fall to draw.
- Draw tail (all non-returning paths): FUN_8c02239c + execTasks(var_tasks_8c1ba5e8)
  + FUN_8c022910, drawSprite mark 0x7b @ z=-1.1 (MARK_Z_BASE), and mark 0x7c too
  when counter & 0x18 (LP_GEN_94405: H'0708 threshold, H'BF8CCCCD priority).
- Wrote the whole function on a Ghidra-trust guess and validated against the
  ORIGINAL asm first (10 tests, all green on src.obj) -- confirmed the FPSCR-noise
  calls really were startAdxFadeOut/drawSprite with (0,0,-1.1) float args.
- Test note: var_markTexlist_8c1bc418 is auto-allocated by addressOf(); do NOT
  also setSize() it (double-alloc error). Task struct built with alloc(0x20),
  phase/counter at +8/+0xc, passed as the single call arg.
- Refactored to switch(phase); phases 1/2 merged into one arm (only the
  pushTitle arg differs, ternary on phase), goto replaced by break-to-tail.
  New #defines: MARK_DEMO/MARK_DEMO_BLINK (0x7b/0x7c), DEMO_END_PLAYING/
  SKIPPED/TIMED_OUT, DEMO_END_TIMEOUT (0x708). Same side effects/order,
  all 10 tests green on both objects; full suite 290/290.
  Backup: scratchpad 0129cc.c.pre-d5a-refactor.bak.

**UNIT 0129cc COMPLETE -- all 4 functions decompiled. c.obj now covers 54-227.**

## serial logging
- Added `[PAUSE]` LOG_* instrumentation (serial_debug.h). Tag reused across all 4 fns.
- retirePhase transitions go through CHANGE_RETIRE_PHASE(x) macro +
  DEBUG_retirePhaseNames[] (IDLE/CONFIRM/FADING), same idiom as 019e98_main_menu.
- FUN_8c0129cc: DEBUG on open/close/resume/cursor/choice transitions, INFO on the
  two retire-complete leave paths (practice / course menu).
- task_8c012cbc/d06/d5a: TRACE entry (per-frame), DEBUG on reset-to-title,
  DEBUG/INFO on demo skip/timeout/end.
- Logs compiled out under UNIT_TESTING: all tests still 290/290; unit compiles
  0 warnings under SERIAL_DEBUG.

## notes
- FUN_8c0129cc takes NO param (asm overwrites r4 immediately); Ghidra's param_1 spurious
- FUN_8c022560() takes NO arg (pushes all regs, never reads r4); Ghidra's arg spurious
- peripheral: press=0x10 (bitmask), x1=0x1c, y1=0x1e (Sint16)
- priorities: arrows -1.09f (BF8B851F), base -1.1f (BF8CCCCD)
- init_8c03bf4c: njDrawPolygon vertex data, owned by 012f44
- test needs setSize on every external fn + init_8c03bf4c for reloc resolution
- press&4 deactivate branch writes var_pauseActive_8c1bb8cc=0 (NOT bb8b8; Ghidra puVar1 = bb8cc there)
- draws always fall through to base 0x74 draw; every branch-N test must also declare the 0x74 draw + njDrawPolygon (single ordered expectation queue: writes AND calls interleaved, strict order)
- to see the failing instruction: sh4objtest test <php> <obj.obj> -d  (suite/-c hides disasm)
- Ghidra PTR-name aliases (resolved via LP_GEN_43727 pool @asm:600): FUN_8c012de0=FUN_8c016182 (01614c.h); FUN_8c012e10=FUN_8c01f21c (01e27c.h); DAT_8c012e0c=var_menuState_8c1bc7a8.selected_0x38 (015ab8_title.h)
- ad04>=5 progress-commit block in Ghidra is DEAD (ad04 maxes at 2 in that branch); real reachable copy lives in ad08==2 path
- new externs added: var_8c22640c (int, 0fcd20_sectionB.h), FUN_8c01f21c (01e27c.h); FUN_8c016182/CourseMenuFUN_8c017ef2/menuState/isFading/playMode/progress already had headers
- ad08==2 isFading!=0 draws 0x76 at FR14=-1.09 (same arrow priority), then njDrawPolygon, returns WITHOUT base 0x74 draw
