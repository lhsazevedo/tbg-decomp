# Test Writing Guide

Tests are executable, frame-sized specs for an SH4 CPU simulator. Each test prepares symbols/memory, invokes one entrypoint, and asserts ordered side effects.

**Run a test:**
```bash
./docker-run.sh ./scripts/run_tests.sh -c /app/tests/path/to/test.php
./docker-run.sh ./scripts/run_tests.sh -c /app/tests/path/to/test.php -d   # + debug trace
```

For the full DSL reference and advanced patterns see [`dsl-reference.md`](dsl-reference.md).

---

## Role

You are the **test author** specifying the contract of a function/frame. Assert only observable behavior that matters — not implementation trivia.

## Before Writing: Interview Checklist

1. **Entry & params** — Which symbol is the entrypoint? Exact param types (int/float)? Struct pointer → `alloc()` + seed + pass explicitly. Any "don't care" args → `WildcardArgument`.
2. **State preconditions** — Which globals/struct fields must be initialized? Need strings? → `allocString()`. Specific addresses? → `rellocate()`.
3. **Observable effects** — Which subroutines are called, with what args and returns? Which addresses are written/read? Float writes? Complex side effects (memcpy) → `shouldCall()->do(callback)`.
4. **Memory randomization** — Default: uninitialized memory is random (catches uninit reads). Disable with `doNotRandomizeMemory()` only when needed for performance or clarity.
5. **Ordering & boundaries** — Are effects strictly ordered? Does the function return a value? Can epilogue be skipped? Use `forceStop()` to stop early.
6. **Helpers** — Does repetitive code warrant private methods (sprite draw, struct field init, state transition)?

## Writing a Test

```php
return new class extends TestCase {
    // 1. Resolve all symbols the code touches
    private function resolveSymbols(): void {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
    }

    public function test_1(): void {
        $this->resolveSymbols();

        // 2. Seed required pre-state
        $base = $this->addressOf('_menuState_8c1bc7a8');
        $this->initUint32($base + 0x18, 0);

        // 3. Invoke entrypoint
        $this->call('_MyFunc_8c017718')->with(/* args */);

        // 4. Assert effects in execution order
        $this->shouldCall('_helperFn_8c01432a')->andReturn(1);
        $this->shouldWriteLong($base + 0x18, 1);
        $this->shouldReturn(0);
    }
};
```

## Guardrails

- One `call()` per test; no legacy `singleCall`/`singleShouldReturn` API.
- No PHPUnit assertions.
- Preconditions use `initUint*`, never `shouldRead*`.
- Assert effects in exact execution order.
- Assert **all** meaningful writes — including increments/decrements of struct fields.
- Declare every symbol the code touches in `resolveSymbols()` with `setSize()`.
- If entrypoint takes non-void params, allocate and seed them.
- Always-executed epilogue code must be asserted in every test that reaches it.
- Use `forceStop()` when stopping before epilogue; mirror C conditions when epilogue is conditional.
- Use `WildcardArgument` for "don't care" parameters.
- Use `shouldWriteFloat()` for float assertions, not raw binary values.
- Extract helpers for patterns repeated 3+ times.
